#pragma once

#include <QObject>
#include <QImage>

class SettingsManager;

class OutputManager : public QObject {
    Q_OBJECT
public:
    explicit OutputManager(SettingsManager *settings, QObject *parent = nullptr);

    void saveScreenshot(const QImage &image);
    void copyImageToClipboard(const QImage &image);
    void copyTextToClipboard(const QString &text);

signals:
    void screenshotSaved(const QString &path);
    void saveFailed(const QString &error);

private:
    QString buildFilePath(bool png) const;
    SettingsManager *m_settings;
};
