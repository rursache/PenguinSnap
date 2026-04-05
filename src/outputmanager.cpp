#include "outputmanager.h"
#include "settingsmanager.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QImageWriter>
#include <QProcess>

OutputManager::OutputManager(SettingsManager *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
}

static bool imageUsesAlpha(const QImage &image)
{
    if (!image.hasAlphaChannel())
        return false;
    const QList<QPoint> corners = {
        {0, 0},
        {image.width() - 1, 0},
        {0, image.height() - 1},
        {image.width() - 1, image.height() - 1},
    };
    for (const auto &p : corners) {
        if (qAlpha(image.pixel(p)) < 255)
            return true;
    }
    return false;
}

void OutputManager::saveScreenshot(const QImage &image)
{
    bool usePng = imageUsesAlpha(image);
    QString filePath = buildFilePath(usePng);

    QDir dir = QFileInfo(filePath).absoluteDir();
    if (!dir.exists())
        dir.mkpath(QStringLiteral("."));

    if (usePng) {
        QImageWriter writer(filePath, "png");
        if (writer.write(image)) {
            emit screenshotSaved(filePath);
        } else {
            emit saveFailed(writer.errorString());
        }
    } else {
        QImageWriter writer(filePath, "jpeg");
        writer.setQuality(90);
        if (writer.write(image)) {
            emit screenshotSaved(filePath);
        } else {
            emit saveFailed(writer.errorString());
        }
    }
}

void OutputManager::copyImageToClipboard(const QImage &image)
{
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, proc, &QProcess::deleteLater);
    proc->start(QStringLiteral("wl-copy"), {QStringLiteral("-t"), QStringLiteral("image/png")});
    if (proc->waitForStarted(2000)) {
        QByteArray pngData;
        QBuffer buffer(&pngData);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        proc->write(pngData);
        proc->closeWriteChannel();
    } else {
        proc->deleteLater();
        QApplication::clipboard()->setImage(image);
    }
}

void OutputManager::copyTextToClipboard(const QString &text)
{
    auto *proc = new QProcess(this);
    connect(proc, &QProcess::finished, proc, &QProcess::deleteLater);
    proc->start(QStringLiteral("wl-copy"));
    if (proc->waitForStarted(2000)) {
        proc->write(text.toUtf8());
        proc->closeWriteChannel();
    } else {
        proc->deleteLater();
        QApplication::clipboard()->setText(text);
    }
}

QString OutputManager::buildFilePath(bool png) const
{
    QString pattern = m_settings->filenamePattern();
    QDateTime now = QDateTime::currentDateTime();

    QString filename = pattern;
    filename.replace(QStringLiteral("YYYY"), now.toString(QStringLiteral("yyyy")));
    filename.replace(QStringLiteral("DD"),   now.toString(QStringLiteral("dd")));
    filename.replace(QStringLiteral("HH"),   now.toString(QStringLiteral("HH")));
    filename.replace(QStringLiteral("MM"),   now.toString(QStringLiteral("MM")));
    filename.replace(QStringLiteral("mm"),   now.toString(QStringLiteral("mm")));
    filename.replace(QStringLiteral("ss"),   now.toString(QStringLiteral("ss")));

    QString ext = png ? QStringLiteral(".png") : QStringLiteral(".jpg");
    return QDir(m_settings->saveDirectory()).filePath(filename + ext);
}
