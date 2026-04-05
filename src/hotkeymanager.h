#pragma once

#include <QObject>

class HotkeyManager : public QObject {
    Q_OBJECT
public:
    explicit HotkeyManager(QObject *parent = nullptr);

signals:
    void captureAreaRequested();
    void captureWindowRequested();
    void captureFullscreenRequested();
    void captureOCRRequested();

private:
    void registerShortcuts();
};
