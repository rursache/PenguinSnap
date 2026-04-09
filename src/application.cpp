#include "application.h"
#include "screenshotmanager.h"
#include "ocrmanager.h"
#include "outputmanager.h"
#include "settingsmanager.h"
#include "hotkeymanager.h"
#include "preferencesdialog.h"
#include "countdownoverlay.h"

#include <KGlobalAccel>
#include <KStatusNotifierItem>
#include <QAction>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QIcon>
#include <QPalette>
#include <QProcess>
#include <QScreen>

Application::Application(QObject *parent)
    : QObject(parent)
    , m_settingsManager(new SettingsManager(this))
    , m_screenshotManager(new ScreenshotManager(this))
    , m_ocrManager(new OCRManager(this))
    , m_outputManager(new OutputManager(m_settingsManager, this))
    , m_hotkeyManager(new HotkeyManager(this))
{
    // Extract bundled snap sound to a temp file for paplay
    m_snapSoundPath = QDir::tempPath() + QStringLiteral("/penguinsnap_snap.wav");
    QFile::remove(m_snapSoundPath);
    QFile::copy(QStringLiteral(":/sounds/snap"), m_snapSoundPath);

    setupTrayIcon();
    setupMenu();

    connect(m_screenshotManager, &ScreenshotManager::screenshotCaptured,
            this, [this](const QImage &image) {
        if (m_ocrMode) {
            m_ocrMode = false;
            onOCRScreenshotCaptured(image);
        } else {
            onScreenshotCaptured(image);
        }
    });

    connect(m_screenshotManager, &ScreenshotManager::captureFailed,
            this, [](const QString &error) {
        qWarning("Screenshot capture failed: %s", qPrintable(error));
    });

    // Timed area: user selected a rect → show countdown → capture
    connect(m_screenshotManager, &ScreenshotManager::areaRectSelected,
            this, [this](const QRect &widgetRect) {
        m_timedAreaRect = widgetRect;
        startCountdown(m_settingsManager->timerDuration(), widgetRect);
    });

    connect(m_ocrManager, &OCRManager::textRecognized,
            this, [this](const QString &text) {
        m_outputManager->copyTextToClipboard(text);
        QString preview = text.length() > 100 ? text.left(100) + QStringLiteral("...") : text;
        showNotification(QStringLiteral("OCR Complete"),
                         QStringLiteral("Copied to clipboard: \"%1\"").arg(preview));
    });

    connect(m_ocrManager, &OCRManager::ocrFailed,
            this, [this](const QString &error) {
        showNotification(QStringLiteral("OCR Failed"), error);
    });

    connect(m_hotkeyManager, &HotkeyManager::captureAreaRequested,
            this, &Application::captureArea);
    connect(m_hotkeyManager, &HotkeyManager::captureWindowRequested,
            this, &Application::captureWindow);
    connect(m_hotkeyManager, &HotkeyManager::captureFullscreenRequested,
            this, &Application::captureFullscreen);
    connect(m_hotkeyManager, &HotkeyManager::captureOCRRequested,
            this, &Application::captureOCR);
    connect(m_hotkeyManager, &HotkeyManager::timedCaptureAreaRequested,
            this, &Application::timedCaptureArea);
    connect(m_hotkeyManager, &HotkeyManager::timedCaptureFullscreenRequested,
            this, &Application::timedCaptureFullscreen);
}

Application::~Application() = default;

void Application::setupTrayIcon()
{
    m_trayIcon = new KStatusNotifierItem(this);
    m_trayIcon->setCategory(KStatusNotifierItem::ApplicationStatus);
    m_trayIcon->setStatus(KStatusNotifierItem::Active);
    m_trayIcon->setToolTipTitle(QStringLiteral("PenguinSnap"));
    m_trayIcon->setToolTipSubTitle(QStringLiteral("Screenshot & OCR Tool"));

    auto palette = QApplication::palette();
    bool isDark = palette.color(QPalette::Window).lightness() < 128;
    QString iconPath = isDark ? QStringLiteral(":/icons/camera-white")
                              : QStringLiteral(":/icons/camera-black");
    m_trayIcon->setIconByPixmap(QIcon(iconPath));
}

void Application::setupMenu()
{
    m_menu = new QMenu();

    auto *versionAction = m_menu->addAction(
        QStringLiteral("PenguinSnap · %1").arg(QApplication::applicationVersion()));
    versionAction->setEnabled(false);

    m_menu->addSeparator();

    m_areaMenuAction = m_menu->addAction(QStringLiteral("Capture Area"));
    connect(m_areaMenuAction, &QAction::triggered, this, &Application::captureArea);

    m_windowMenuAction = m_menu->addAction(QStringLiteral("Capture Window"));
    connect(m_windowMenuAction, &QAction::triggered, this, &Application::captureWindow);

    m_fullscreenMenuAction = m_menu->addAction(QStringLiteral("Capture Fullscreen"));
    connect(m_fullscreenMenuAction, &QAction::triggered, this, &Application::captureFullscreen);

    m_ocrMenuAction = m_menu->addAction(QStringLiteral("Capture Text via OCR"));
    connect(m_ocrMenuAction, &QAction::triggered, this, &Application::captureOCR);

    m_menu->addSeparator();

    m_timedAreaMenuAction = m_menu->addAction(QStringLiteral("Timed Capture Area"));
    connect(m_timedAreaMenuAction, &QAction::triggered, this, &Application::timedCaptureArea);

    m_timedFullscreenMenuAction = m_menu->addAction(QStringLiteral("Timed Capture Fullscreen"));
    connect(m_timedFullscreenMenuAction, &QAction::triggered, this, &Application::timedCaptureFullscreen);

    m_menu->addSeparator();

    auto *prefsAction = m_menu->addAction(QStringLiteral("Preferences..."));
    connect(prefsAction, &QAction::triggered, this, &Application::showPreferences);

    m_trayIcon->setContextMenu(m_menu);
    updateMenuShortcuts();
}

void Application::updateMenuShortcuts()
{
    auto shortcutFor = [](QAction *globalAction) -> QKeySequence {
        auto keys = KGlobalAccel::self()->shortcut(globalAction);
        return keys.isEmpty() ? QKeySequence() : keys.first();
    };

    m_areaMenuAction->setShortcut(shortcutFor(m_hotkeyManager->captureAreaAction()));
    m_windowMenuAction->setShortcut(shortcutFor(m_hotkeyManager->captureWindowAction()));
    m_fullscreenMenuAction->setShortcut(shortcutFor(m_hotkeyManager->captureFullscreenAction()));
    m_ocrMenuAction->setShortcut(shortcutFor(m_hotkeyManager->captureOCRAction()));
    m_timedAreaMenuAction->setShortcut(shortcutFor(m_hotkeyManager->timedCaptureAreaAction()));
    m_timedFullscreenMenuAction->setShortcut(shortcutFor(m_hotkeyManager->timedCaptureFullscreenAction()));
}

void Application::captureArea()
{
    m_ocrMode = false;
    m_screenshotManager->captureArea();
}

void Application::captureWindow()
{
    m_ocrMode = false;
    m_screenshotManager->captureWindow();
}

void Application::captureFullscreen()
{
    m_ocrMode = false;
    m_screenshotManager->captureFullscreen();
}

void Application::captureOCR()
{
    m_ocrMode = true;
    m_screenshotManager->captureArea();
}

void Application::timedCaptureArea()
{
    m_ocrMode = false;
    m_screenshotManager->selectAreaRect();
}

void Application::timedCaptureFullscreen()
{
    m_ocrMode = false;
    m_timedFullscreenMode = true;
    startCountdown(m_settingsManager->timerDuration());
}

void Application::startCountdown(int seconds, const QRect &selectedRect)
{
    m_countdown = new CountdownOverlay(seconds, selectedRect);

    connect(m_countdown, &CountdownOverlay::countdownFinished, this, [this]() {
        m_countdown = nullptr;
        if (m_timedFullscreenMode) {
            m_timedFullscreenMode = false;
            m_screenshotManager->captureFullscreen();
        } else {
            // Timed area: take fresh fullscreen, crop to saved rect
            // We need to get screen geometry to map widget rect to image rect
            m_timedAreaMode = true;
            m_screenshotManager->captureFullscreen();
        }
    });

    connect(m_countdown, &CountdownOverlay::countdownCancelled, this, [this]() {
        m_countdown = nullptr;
        m_timedFullscreenMode = false;
        m_timedAreaMode = false;
    });
}

void Application::showPreferences()
{
    if (!m_prefsDialog) {
        m_prefsDialog = new PreferencesDialog(m_settingsManager, m_ocrManager, m_hotkeyManager);
        connect(m_prefsDialog, &PreferencesDialog::shortcutsChanged,
                this, &Application::updateMenuShortcuts);
    }
    m_prefsDialog->show();
    m_prefsDialog->raise();
    m_prefsDialog->activateWindow();
}

void Application::onScreenshotCaptured(const QImage &image)
{
    if (m_timedAreaMode) {
        m_timedAreaMode = false;
        // Crop the fullscreen image to the saved widget rect.
        // The widget rect is in overlay (screen) coordinates.
        // The image from Spectacle fullscreen matches the screen resolution.
        QScreen *screen = QApplication::primaryScreen();
        if (screen && !m_timedAreaRect.isNull()) {
            qreal dpr = screen->devicePixelRatio();
            QRect imgRect(
                qRound(m_timedAreaRect.x() * dpr),
                qRound(m_timedAreaRect.y() * dpr),
                qRound(m_timedAreaRect.width() * dpr),
                qRound(m_timedAreaRect.height() * dpr));
            QImage cropped = image.copy(imgRect);
            m_outputManager->saveScreenshot(cropped);
            if (m_settingsManager->saveToClipboard())
                m_outputManager->copyImageToClipboard(cropped);
            playSnapSound();
        }
        return;
    }

    m_outputManager->saveScreenshot(image);
    if (m_settingsManager->saveToClipboard()) {
        m_outputManager->copyImageToClipboard(image);
    }
    playSnapSound();
}

void Application::onOCRScreenshotCaptured(const QImage &image)
{
    m_ocrManager->recognizeText(image, m_settingsManager->ocrLanguage());
}

void Application::showNotification(const QString &title, const QString &body)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("Notify"));

    msg << QStringLiteral("PenguinSnap")   // app_name
        << static_cast<uint>(0)            // replaces_id
        << QStringLiteral("penguinsnap")   // app_icon (installed via CMake)
        << title                           // summary
        << body                            // body
        << QStringList()                   // actions
        << QVariantMap()                   // hints
        << static_cast<int>(5000);         // timeout ms

    QDBusConnection::sessionBus().asyncCall(msg);
}

void Application::playSnapSound()
{
    if (QFile::exists(m_snapSoundPath))
        QProcess::startDetached(QStringLiteral("paplay"), {m_snapSoundPath});
}
