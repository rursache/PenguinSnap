#include "settingsmanager.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("randusoft"), QStringLiteral("penguinsnap"))
{
}

bool SettingsManager::saveToClipboard() const
{
    return m_settings.value(QStringLiteral("saveToClipboard"), true).toBool();
}

void SettingsManager::setSaveToClipboard(bool enabled)
{
    m_settings.setValue(QStringLiteral("saveToClipboard"), enabled);
    emit settingsChanged();
}

QString SettingsManager::saveDirectory() const
{
    return m_settings.value(QStringLiteral("saveDirectory"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
}

void SettingsManager::setSaveDirectory(const QString &dir)
{
    m_settings.setValue(QStringLiteral("saveDirectory"), dir);
    emit settingsChanged();
}

QString SettingsManager::filenamePattern() const
{
    return m_settings.value(QStringLiteral("filenamePattern"),
        QStringLiteral("PenguinSnap_YYYY-MM-DD_HH.mm.ss")).toString();
}

void SettingsManager::setFilenamePattern(const QString &pattern)
{
    m_settings.setValue(QStringLiteral("filenamePattern"), pattern);
    emit settingsChanged();
}

bool SettingsManager::startAtLogin() const
{
    return m_settings.value(QStringLiteral("startAtLogin"), false).toBool();
}

void SettingsManager::setStartAtLogin(bool enabled)
{
    m_settings.setValue(QStringLiteral("startAtLogin"), enabled);

    QString autostartDir = QStandardPaths::writableLocation(
        QStandardPaths::ConfigLocation) + QStringLiteral("/autostart");
    QString desktopFile = autostartDir + QStringLiteral("/penguinsnap.desktop");

    if (enabled) {
        QDir().mkpath(autostartDir);
        QFile file(desktopFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "[Desktop Entry]\n"
                << "Type=Application\n"
                << "Name=PenguinSnap\n"
                << "Exec=penguinsnap\n"
                << "Icon=penguinsnap\n"
                << "Comment=Screenshot and OCR tool\n"
                << "StartupNotify=false\n"
                << "Terminal=false\n";
        }
    } else {
        QFile::remove(desktopFile);
    }

    emit settingsChanged();
}

int SettingsManager::timerDuration() const
{
    return m_settings.value(QStringLiteral("timerDuration"), 3).toInt();
}

void SettingsManager::setTimerDuration(int seconds)
{
    m_settings.setValue(QStringLiteral("timerDuration"), seconds);
    emit settingsChanged();
}

QString SettingsManager::ocrLanguage() const
{
    return m_settings.value(QStringLiteral("ocrLanguage"),
        QStringLiteral("eng")).toString();
}

void SettingsManager::setOcrLanguage(const QString &lang)
{
    m_settings.setValue(QStringLiteral("ocrLanguage"), lang);
    emit settingsChanged();
}
