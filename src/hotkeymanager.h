#pragma once

#include <QObject>

class QAction;

class HotkeyManager : public QObject {
    Q_OBJECT
public:
    explicit HotkeyManager(QObject *parent = nullptr);

    QAction *captureAreaAction() const { return m_captureArea; }
    QAction *captureWindowAction() const { return m_captureWindow; }
    QAction *captureFullscreenAction() const { return m_captureFullscreen; }
    QAction *captureOCRAction() const { return m_captureOCR; }

signals:
    void captureAreaRequested();
    void captureWindowRequested();
    void captureFullscreenRequested();
    void captureOCRRequested();

private:
    void registerShortcuts();

    QAction *m_captureArea = nullptr;
    QAction *m_captureWindow = nullptr;
    QAction *m_captureFullscreen = nullptr;
    QAction *m_captureOCR = nullptr;
};
