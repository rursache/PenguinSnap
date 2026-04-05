#pragma once

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject {
    Q_OBJECT
public:
    explicit SettingsManager(QObject *parent = nullptr);

    bool saveToClipboard() const;
    void setSaveToClipboard(bool enabled);

    QString saveDirectory() const;
    void setSaveDirectory(const QString &dir);

    QString filenamePattern() const;
    void setFilenamePattern(const QString &pattern);

    bool startAtLogin() const;
    void setStartAtLogin(bool enabled);

    QString ocrLanguage() const;
    void setOcrLanguage(const QString &lang);

signals:
    void settingsChanged();

private:
    QSettings m_settings;
};
