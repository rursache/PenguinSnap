#pragma once

#include <QObject>
#include <QMenu>

class KStatusNotifierItem;
class ScreenshotManager;
class OCRManager;
class OutputManager;
class SettingsManager;
class HotkeyManager;
class PreferencesDialog;
class QImage;

class Application : public QObject {
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override;

private slots:
    void captureArea();
    void captureWindow();
    void captureFullscreen();
    void captureOCR();
    void showPreferences();

private:
    void setupTrayIcon();
    void setupMenu();
    void updateMenuShortcuts();
    void onScreenshotCaptured(const QImage &image);
    void onOCRScreenshotCaptured(const QImage &image);
    void showNotification(const QString &title, const QString &body);
    void playSnapSound();

    KStatusNotifierItem *m_trayIcon = nullptr;
    QString m_snapSoundPath;
    QMenu *m_menu = nullptr;
    QAction *m_areaMenuAction = nullptr;
    QAction *m_windowMenuAction = nullptr;
    QAction *m_fullscreenMenuAction = nullptr;
    QAction *m_ocrMenuAction = nullptr;
    SettingsManager *m_settingsManager;
    ScreenshotManager *m_screenshotManager;
    OCRManager *m_ocrManager;
    OutputManager *m_outputManager;
    HotkeyManager *m_hotkeyManager;
    PreferencesDialog *m_prefsDialog = nullptr;
    bool m_ocrMode = false;
};
