#pragma once

#include <QDialog>

class SettingsManager;
class OCRManager;
class QCheckBox;
class QComboBox;
class QLineEdit;

class PreferencesDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreferencesDialog(SettingsManager *settings, OCRManager *ocrManager, QWidget *parent = nullptr);

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void chooseSaveDirectory();

    SettingsManager *m_settings;
    OCRManager *m_ocrManager;
    QCheckBox *m_clipboardCheck;
    QCheckBox *m_startAtLoginCheck;
    QLineEdit *m_directoryEdit;
    QLineEdit *m_patternEdit;
    QComboBox *m_languageCombo;
};
