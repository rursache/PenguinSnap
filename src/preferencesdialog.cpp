#include "preferencesdialog.h"
#include "settingsmanager.h"
#include "ocrmanager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

PreferencesDialog::PreferencesDialog(SettingsManager *settings, OCRManager *ocrManager, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_ocrManager(ocrManager)
{
    setWindowTitle(QStringLiteral("PenguinSnap Preferences"));
    setFixedSize(500, 350);
    setupUI();
    loadSettings();
}

void PreferencesDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    // General
    auto *generalGroup = new QGroupBox(QStringLiteral("General"));
    auto *generalLayout = new QFormLayout(generalGroup);

    m_startAtLoginCheck = new QCheckBox(QStringLiteral("Start at login"));
    generalLayout->addRow(m_startAtLoginCheck);

    m_clipboardCheck = new QCheckBox(QStringLiteral("Copy screenshots to clipboard"));
    generalLayout->addRow(m_clipboardCheck);

    mainLayout->addWidget(generalGroup);

    // Output
    auto *outputGroup = new QGroupBox(QStringLiteral("Output"));
    auto *outputLayout = new QFormLayout(outputGroup);

    auto *dirLayout = new QHBoxLayout();
    m_directoryEdit = new QLineEdit();
    m_directoryEdit->setReadOnly(true);
    auto *browseBtn = new QPushButton(QStringLiteral("Browse..."));
    dirLayout->addWidget(m_directoryEdit);
    dirLayout->addWidget(browseBtn);
    outputLayout->addRow(QStringLiteral("Save to:"), dirLayout);
    connect(browseBtn, &QPushButton::clicked, this, &PreferencesDialog::chooseSaveDirectory);

    m_patternEdit = new QLineEdit();
    m_patternEdit->setToolTip(QStringLiteral("Available tokens: YYYY, MM, DD, HH, mm, ss"));
    outputLayout->addRow(QStringLiteral("Filename pattern:"), m_patternEdit);

    mainLayout->addWidget(outputGroup);

    // OCR
    auto *ocrGroup = new QGroupBox(QStringLiteral("OCR"));
    auto *ocrLayout = new QFormLayout(ocrGroup);

    m_languageCombo = new QComboBox();
    const QStringList langs = m_ocrManager->availableLanguages();
    for (const auto &lang : langs)
        m_languageCombo->addItem(lang);
    ocrLayout->addRow(QStringLiteral("Language:"), m_languageCombo);

    mainLayout->addWidget(ocrGroup);

    // Buttons
    mainLayout->addStretch();
    auto *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        saveSettings();
        accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void PreferencesDialog::loadSettings()
{
    m_startAtLoginCheck->setChecked(m_settings->startAtLogin());
    m_clipboardCheck->setChecked(m_settings->saveToClipboard());
    m_directoryEdit->setText(m_settings->saveDirectory());
    m_patternEdit->setText(m_settings->filenamePattern());

    int langIdx = m_languageCombo->findText(m_settings->ocrLanguage());
    if (langIdx >= 0)
        m_languageCombo->setCurrentIndex(langIdx);
}

void PreferencesDialog::saveSettings()
{
    m_settings->setStartAtLogin(m_startAtLoginCheck->isChecked());
    m_settings->setSaveToClipboard(m_clipboardCheck->isChecked());
    m_settings->setFilenamePattern(m_patternEdit->text());
    m_settings->setOcrLanguage(m_languageCombo->currentText());
}

void PreferencesDialog::chooseSaveDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, QStringLiteral("Select Save Directory"), m_settings->saveDirectory());
    if (!dir.isEmpty()) {
        m_directoryEdit->setText(dir);
        m_settings->setSaveDirectory(dir);
    }
}
