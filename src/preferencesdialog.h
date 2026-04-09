#pragma once

#include <QDialog>

class SettingsManager;
class OCRManager;
class HotkeyManager;
class QCheckBox;
class QComboBox;
class QLabel;
class QSlider;
class KKeySequenceWidget;
class QLineEdit;

class PreferencesDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreferencesDialog(SettingsManager *settings, OCRManager *ocrManager,
                               HotkeyManager *hotkeyManager, QWidget *parent = nullptr);

signals:
    void shortcutsChanged();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void chooseSaveDirectory();

    SettingsManager *m_settings;
    OCRManager *m_ocrManager;
    HotkeyManager *m_hotkeyManager;
    QCheckBox *m_clipboardCheck;
    QCheckBox *m_startAtLoginCheck;
    QLineEdit *m_directoryEdit;
    QLineEdit *m_patternEdit;
    QComboBox *m_languageCombo;
    KKeySequenceWidget *m_areaShortcut;
    KKeySequenceWidget *m_windowShortcut;
    KKeySequenceWidget *m_fullscreenShortcut;
    KKeySequenceWidget *m_ocrShortcut;
    KKeySequenceWidget *m_timedAreaShortcut;
    KKeySequenceWidget *m_timedFullscreenShortcut;
    QSlider *m_timerSlider;
    QLabel *m_timerLabel;
};
