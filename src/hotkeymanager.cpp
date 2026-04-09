#include "hotkeymanager.h"

#include <KGlobalAccel>
#include <QAction>

HotkeyManager::HotkeyManager(QObject *parent)
    : QObject(parent)
{
    registerShortcuts();
}

void HotkeyManager::registerShortcuts()
{
    m_captureArea = new QAction(this);
    m_captureArea->setObjectName(QStringLiteral("capture_area"));
    m_captureArea->setText(QStringLiteral("Capture Area"));
    KGlobalAccel::setGlobalShortcut(m_captureArea, QList<QKeySequence>());
    connect(m_captureArea, &QAction::triggered, this, &HotkeyManager::captureAreaRequested);

    m_captureWindow = new QAction(this);
    m_captureWindow->setObjectName(QStringLiteral("capture_window"));
    m_captureWindow->setText(QStringLiteral("Capture Window"));
    KGlobalAccel::setGlobalShortcut(m_captureWindow, QList<QKeySequence>());
    connect(m_captureWindow, &QAction::triggered, this, &HotkeyManager::captureWindowRequested);

    m_captureFullscreen = new QAction(this);
    m_captureFullscreen->setObjectName(QStringLiteral("capture_fullscreen"));
    m_captureFullscreen->setText(QStringLiteral("Capture Fullscreen"));
    KGlobalAccel::setGlobalShortcut(m_captureFullscreen, QList<QKeySequence>());
    connect(m_captureFullscreen, &QAction::triggered, this, &HotkeyManager::captureFullscreenRequested);

    m_captureOCR = new QAction(this);
    m_captureOCR->setObjectName(QStringLiteral("capture_ocr"));
    m_captureOCR->setText(QStringLiteral("Capture Text via OCR"));
    KGlobalAccel::setGlobalShortcut(m_captureOCR, QList<QKeySequence>());
    connect(m_captureOCR, &QAction::triggered, this, &HotkeyManager::captureOCRRequested);
}
