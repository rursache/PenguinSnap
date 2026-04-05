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
    auto *captureArea = new QAction(this);
    captureArea->setObjectName(QStringLiteral("capture_area"));
    captureArea->setText(QStringLiteral("Capture Area"));
    KGlobalAccel::setGlobalShortcut(captureArea, QKeySequence(QStringLiteral("Ctrl+Shift+Alt+A")));
    connect(captureArea, &QAction::triggered, this, &HotkeyManager::captureAreaRequested);

    auto *captureWindow = new QAction(this);
    captureWindow->setObjectName(QStringLiteral("capture_window"));
    captureWindow->setText(QStringLiteral("Capture Window"));
    KGlobalAccel::setGlobalShortcut(captureWindow, QKeySequence(QStringLiteral("Ctrl+Shift+Alt+W")));
    connect(captureWindow, &QAction::triggered, this, &HotkeyManager::captureWindowRequested);

    auto *captureFullscreen = new QAction(this);
    captureFullscreen->setObjectName(QStringLiteral("capture_fullscreen"));
    captureFullscreen->setText(QStringLiteral("Capture Fullscreen"));
    KGlobalAccel::setGlobalShortcut(captureFullscreen, QKeySequence(QStringLiteral("Ctrl+Shift+Alt+F")));
    connect(captureFullscreen, &QAction::triggered, this, &HotkeyManager::captureFullscreenRequested);

    auto *captureOCR = new QAction(this);
    captureOCR->setObjectName(QStringLiteral("capture_ocr"));
    captureOCR->setText(QStringLiteral("Capture Text via OCR"));
    KGlobalAccel::setGlobalShortcut(captureOCR, QKeySequence(QStringLiteral("Ctrl+Shift+Alt+T")));
    connect(captureOCR, &QAction::triggered, this, &HotkeyManager::captureOCRRequested);
}
