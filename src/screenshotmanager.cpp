#include "screenshotmanager.h"
#include "selectionoverlay.h"
#include "windowoverlay.h"

#include <QDBusConnection>
#include <QRegularExpression>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTimer>
#include <QUuid>


static const QString kWindowScript = QStringLiteral(
    "var ws = workspace.stackingOrder;\n"
    "var result = [];\n"
    "for (var i = ws.length - 1; i >= 0; i--) {\n"
    "    var w = ws[i];\n"
    "    if (!w.minimized && w.normalWindow && !w.skipTaskbar) {\n"
    "        var g = w.frameGeometry;\n"
    "        var id = w.internalId.toString();\n"
    "        result.push(id + \":\" + Math.round(g.x) + \",\" + Math.round(g.y)\n"
    "                  + \",\" + Math.round(g.width) + \",\" + Math.round(g.height));\n"
    "    }\n"
    "}\n"
    "callDBus(\"ro.randusoft.PenguinSnap\", \"/ScreenshotManager\",\n"
    "         \"ro.randusoft.PenguinSnap\", \"receiveWindowList\",\n"
    "         result.join(\";\"));\n"
);

ScreenshotManager::ScreenshotManager(QObject *parent)
    : QObject(parent)
{
    auto bus = QDBusConnection::sessionBus();
    if (!bus.registerService(QStringLiteral("ro.randusoft.PenguinSnap")))
        qWarning("D-Bus: failed to register service: %s",
                 qPrintable(bus.lastError().message()));
    if (!bus.registerObject(QStringLiteral("/ScreenshotManager"), this,
                            QDBusConnection::ExportScriptableSlots))
        qWarning("D-Bus: failed to register object: %s",
                 qPrintable(bus.lastError().message()));
}

// ── Area ──────────────────────────────────────────────────────────

void ScreenshotManager::captureArea()
{
    QString out = tempFilePath();
    runSpectacleForSelection({QStringLiteral("-i"), QStringLiteral("-m"),
                              QStringLiteral("-b"), QStringLiteral("-n"),
                              QStringLiteral("-o"), out});
}

// ── Window ────────────────────────────────────────────────────────

void ScreenshotManager::captureWindow()
{
    enumerateWindows();
}

void ScreenshotManager::enumerateWindows()
{
    QString scriptPath = QDir::tempPath() + QStringLiteral("/penguinsnap_winlist.js");
    QFile scriptFile(scriptPath);
    if (!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit captureFailed(QStringLiteral("Failed to create KWin script"));
        return;
    }
    scriptFile.write(kWindowScript.toUtf8());
    scriptFile.close();

    QDBusMessage loadMsg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/Scripting"),
        QStringLiteral("org.kde.kwin.Scripting"),
        QStringLiteral("loadScript"));
    loadMsg << scriptPath << QStringLiteral("penguinsnap_winlist");

    auto *watcher = new QDBusPendingCallWatcher(
        QDBusConnection::sessionBus().asyncCall(loadMsg, 5000), this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, scriptPath](QDBusPendingCallWatcher *w) {
        w->deleteLater();
        QDBusPendingReply<int> reply = *w;
        if (reply.isError()) {
            QFile::remove(scriptPath);
            emit captureFailed(QStringLiteral("Failed to load KWin script: ")
                               + reply.error().message());
            return;
        }

        int scriptId = reply.argumentAt<0>();

        QDBusMessage runMsg = QDBusMessage::createMethodCall(
            QStringLiteral("org.kde.KWin"),
            QStringLiteral("/Scripting/Script%1").arg(scriptId),
            QStringLiteral("org.kde.kwin.Script"),
            QStringLiteral("run"));
        QDBusConnection::sessionBus().asyncCall(runMsg, 5000);

        QTimer::singleShot(2000, this, [scriptPath, scriptId]() {
            QDBusMessage stopMsg = QDBusMessage::createMethodCall(
                QStringLiteral("org.kde.KWin"),
                QStringLiteral("/Scripting/Script%1").arg(scriptId),
                QStringLiteral("org.kde.kwin.Script"),
                QStringLiteral("stop"));
            QDBusConnection::sessionBus().call(stopMsg, QDBus::NoBlock);
            QFile::remove(scriptPath);
        });
    });
}

void ScreenshotManager::receiveWindowList(const QString &data)
{
    QList<WindowInfo> windows;
    if (!data.isEmpty()) {
        const auto entries = data.split(QLatin1Char(';'), Qt::SkipEmptyParts);
        for (const auto &entry : entries) {
            int colonIdx = entry.indexOf(QLatin1Char(':'));
            if (colonIdx < 0) continue;
            QString id = entry.left(colonIdx);
            const auto parts = entry.mid(colonIdx + 1).split(QLatin1Char(','));
            if (parts.size() == 4) {
                windows.append({id, QRect(parts[0].toInt(), parts[1].toInt(),
                                          parts[2].toInt(), parts[3].toInt())});
            }
        }
    }

    if (windows.isEmpty()) {
        emit captureFailed(QStringLiteral("No windows found"));
        return;
    }

    // Capture fullscreen for the overlay background, then show picker
    m_pendingWindows = windows;
    QString out = tempFilePath();
    auto *proc = new QProcess(this);
    QString outputFile = out;

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc, outputFile](int exitCode, QProcess::ExitStatus) {
        proc->deleteLater();
        if (exitCode != 0) { QFile::remove(outputFile); return; }
        if (!QFileInfo::exists(outputFile)) return;

        QImage fullscreen(outputFile);
        QFile::remove(outputFile);
        if (fullscreen.isNull()) return;

        auto *overlay = new WindowOverlay(fullscreen, m_pendingWindows);
        connect(overlay, &WindowOverlay::windowSelected,
                this, &ScreenshotManager::captureWindowById);
        connect(overlay, &WindowOverlay::selectionCancelled,
                overlay, &QObject::deleteLater);
    });

    proc->start(QStringLiteral("spectacle"),
                {QStringLiteral("-i"), QStringLiteral("-m"),
                 QStringLiteral("-b"), QStringLiteral("-n"),
                 QStringLiteral("-o"), out});
    if (!proc->waitForStarted(3000)) {
        proc->deleteLater();
        emit captureFailed(QStringLiteral("Failed to launch Spectacle"));
    }
}

// ── Capture single window by raising it and using spectacle -u ────

void ScreenshotManager::captureWindowById(const QString &windowId)
{
    raiseAndCapture(windowId);
}

void ScreenshotManager::raiseAndCapture(const QString &windowId)
{
    // Validate UUID format to prevent script injection
    static const QRegularExpression uuidRe(
        QStringLiteral("^\\{?[0-9a-fA-F\\-]+\\}?$"));
    if (!uuidRe.match(windowId).hasMatch()) {
        emit captureFailed(QStringLiteral("Invalid window ID"));
        return;
    }

    // Raise the window via a KWin script
    QString raiseScript = QStringLiteral(
        "var ws = workspace.windowList();\n"
        "for (var i = 0; i < ws.length; i++) {\n"
        "    if (ws[i].internalId.toString() === \"%1\") {\n"
        "        workspace.activeWindow = ws[i];\n"
        "        break;\n"
        "    }\n"
        "}\n"
    ).arg(windowId);

    QString scriptPath = QDir::tempPath() + QStringLiteral("/penguinsnap_raise.js");
    QFile scriptFile(scriptPath);
    if (!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit captureFailed(QStringLiteral("Failed to create raise script"));
        return;
    }
    scriptFile.write(raiseScript.toUtf8());
    scriptFile.close();

    QDBusMessage loadMsg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/Scripting"),
        QStringLiteral("org.kde.kwin.Scripting"),
        QStringLiteral("loadScript"));
    loadMsg << scriptPath << QStringLiteral("penguinsnap_raise");

    auto reply = QDBusConnection::sessionBus().call(loadMsg, QDBus::Block, 3000);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        QFile::remove(scriptPath);
        emit captureFailed(QStringLiteral("Failed to load raise script"));
        return;
    }

    int scriptId = reply.arguments().first().toInt();
    QDBusMessage runMsg = QDBusMessage::createMethodCall(
        QStringLiteral("org.kde.KWin"),
        QStringLiteral("/Scripting/Script%1").arg(scriptId),
        QStringLiteral("org.kde.kwin.Script"),
        QStringLiteral("run"));
    QDBusConnection::sessionBus().call(runMsg, QDBus::Block, 3000);

    // Give KWin time to raise and render the window, then capture
    QTimer::singleShot(300, this, [this, scriptPath, scriptId]() {
        // Cleanup the raise script
        QDBusMessage stopMsg = QDBusMessage::createMethodCall(
            QStringLiteral("org.kde.KWin"),
            QStringLiteral("/Scripting/Script%1").arg(scriptId),
            QStringLiteral("org.kde.kwin.Script"),
            QStringLiteral("stop"));
        QDBusConnection::sessionBus().call(stopMsg, QDBus::NoBlock);
        QFile::remove(scriptPath);

        // Capture the now-active window via spectacle
        QString out = tempFilePath();
        runSpectacle({QStringLiteral("-i"), QStringLiteral("-a"),
                      QStringLiteral("-b"), QStringLiteral("-n"),
                      QStringLiteral("-o"), out});
    });
}

// ── Fullscreen ───────────────────────────────────────────────────

void ScreenshotManager::captureFullscreen()
{
    QString out = tempFilePath();
    runSpectacle({QStringLiteral("-i"), QStringLiteral("-f"),
                  QStringLiteral("-b"), QStringLiteral("-n"),
                  QStringLiteral("-o"), out});
}

// ── Spectacle helpers ────────────────────────────────────────────

void ScreenshotManager::runSpectacle(const QStringList &args)
{
    auto *proc = new QProcess(this);
    QString outputFile = args.last();

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc, outputFile](int exitCode, QProcess::ExitStatus) {
        proc->deleteLater();
        if (exitCode != 0) {
            QFile::remove(outputFile);
            return;
        }
        if (!QFileInfo::exists(outputFile)) {
            emit captureFailed(QStringLiteral("Spectacle did not produce an output file"));
            return;
        }
        QImage image(outputFile);
        QFile::remove(outputFile);
        if (image.isNull()) {
            emit captureFailed(QStringLiteral("Failed to load captured screenshot"));
            return;
        }
        emit screenshotCaptured(image);
    });

    proc->start(QStringLiteral("spectacle"), args);
    if (!proc->waitForStarted(3000)) {
        proc->deleteLater();
        emit captureFailed(QStringLiteral("Failed to launch Spectacle. Is it installed?"));
    }
}

void ScreenshotManager::runSpectacleForSelection(const QStringList &args)
{
    auto *proc = new QProcess(this);
    QString outputFile = args.last();

    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc, outputFile](int exitCode, QProcess::ExitStatus) {
        proc->deleteLater();
        if (exitCode != 0) { QFile::remove(outputFile); return; }
        if (!QFileInfo::exists(outputFile)) {
            emit captureFailed(QStringLiteral("Spectacle did not produce an output file"));
            return;
        }
        QImage fullscreen(outputFile);
        QFile::remove(outputFile);
        if (fullscreen.isNull()) {
            emit captureFailed(QStringLiteral("Failed to load captured screenshot"));
            return;
        }
        auto *overlay = new SelectionOverlay(fullscreen);
        connect(overlay, &SelectionOverlay::regionSelected,
                this, &ScreenshotManager::screenshotCaptured);
        connect(overlay, &SelectionOverlay::selectionCancelled,
                overlay, &QObject::deleteLater);
    });

    proc->start(QStringLiteral("spectacle"), args);
    if (!proc->waitForStarted(3000)) {
        proc->deleteLater();
        emit captureFailed(QStringLiteral("Failed to launch Spectacle. Is it installed?"));
    }
}

QString ScreenshotManager::tempFilePath() const
{
    return QDir::tempPath() + QStringLiteral("/penguinsnap_")
           + QUuid::createUuid().toString(QUuid::Id128) + QStringLiteral(".png");
}
