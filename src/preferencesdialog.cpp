#include "preferencesdialog.h"
#include "settingsmanager.h"
#include "ocrmanager.h"
#include "hotkeymanager.h"

#include <KGlobalAccel>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <KKeySequenceWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

PreferencesDialog::PreferencesDialog(SettingsManager *settings, OCRManager *ocrManager,
                                     HotkeyManager *hotkeyManager, QWidget *parent)
    : QDialog(parent)
    , m_settings(settings)
    , m_ocrManager(ocrManager)
    , m_hotkeyManager(hotkeyManager)
{
    setWindowTitle(QStringLiteral("PenguinSnap Preferences"));
    setFixedSize(500, 600);
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

    // Shortcuts
    auto *shortcutsGroup = new QGroupBox(QStringLiteral("Shortcuts"));
    auto *shortcutsLayout = new QFormLayout(shortcutsGroup);

    m_areaShortcut = new KKeySequenceWidget();
    m_areaShortcut->setCheckForConflictsAgainst(KKeySequenceWidget::GlobalShortcuts);
    shortcutsLayout->addRow(QStringLiteral("Capture Area:"), m_areaShortcut);

    m_windowShortcut = new KKeySequenceWidget();
    m_windowShortcut->setCheckForConflictsAgainst(KKeySequenceWidget::GlobalShortcuts);
    shortcutsLayout->addRow(QStringLiteral("Capture Window:"), m_windowShortcut);

    m_fullscreenShortcut = new KKeySequenceWidget();
    m_fullscreenShortcut->setCheckForConflictsAgainst(KKeySequenceWidget::GlobalShortcuts);
    shortcutsLayout->addRow(QStringLiteral("Capture Fullscreen:"), m_fullscreenShortcut);

    m_ocrShortcut = new KKeySequenceWidget();
    m_ocrShortcut->setCheckForConflictsAgainst(KKeySequenceWidget::GlobalShortcuts);
    shortcutsLayout->addRow(QStringLiteral("Capture Text (OCR):"), m_ocrShortcut);

    mainLayout->addWidget(shortcutsGroup);

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

    // Load current shortcuts from KGlobalAccel
    auto areaKeys = KGlobalAccel::self()->shortcut(m_hotkeyManager->captureAreaAction());
    m_areaShortcut->setKeySequence(areaKeys.isEmpty() ? QKeySequence() : areaKeys.first());

    auto windowKeys = KGlobalAccel::self()->shortcut(m_hotkeyManager->captureWindowAction());
    m_windowShortcut->setKeySequence(windowKeys.isEmpty() ? QKeySequence() : windowKeys.first());

    auto fullscreenKeys = KGlobalAccel::self()->shortcut(m_hotkeyManager->captureFullscreenAction());
    m_fullscreenShortcut->setKeySequence(fullscreenKeys.isEmpty() ? QKeySequence() : fullscreenKeys.first());

    auto ocrKeys = KGlobalAccel::self()->shortcut(m_hotkeyManager->captureOCRAction());
    m_ocrShortcut->setKeySequence(ocrKeys.isEmpty() ? QKeySequence() : ocrKeys.first());
}

void PreferencesDialog::saveSettings()
{
    m_settings->setStartAtLogin(m_startAtLoginCheck->isChecked());
    m_settings->setSaveToClipboard(m_clipboardCheck->isChecked());
    m_settings->setFilenamePattern(m_patternEdit->text());
    m_settings->setOcrLanguage(m_languageCombo->currentText());

    // Save shortcuts via KGlobalAccel
    KGlobalAccel::self()->setShortcut(m_hotkeyManager->captureAreaAction(),
        {m_areaShortcut->keySequence()}, KGlobalAccel::NoAutoloading);
    KGlobalAccel::self()->setShortcut(m_hotkeyManager->captureWindowAction(),
        {m_windowShortcut->keySequence()}, KGlobalAccel::NoAutoloading);
    KGlobalAccel::self()->setShortcut(m_hotkeyManager->captureFullscreenAction(),
        {m_fullscreenShortcut->keySequence()}, KGlobalAccel::NoAutoloading);
    KGlobalAccel::self()->setShortcut(m_hotkeyManager->captureOCRAction(),
        {m_ocrShortcut->keySequence()}, KGlobalAccel::NoAutoloading);

    emit shortcutsChanged();
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
