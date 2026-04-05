#pragma once

#include <QObject>
#include <QImage>
#include <QRect>
class QProcess;

struct WindowInfo {
    QString id;   // KWin internalId (UUID)
    QRect rect;   // frame geometry in logical coords
};

class ScreenshotManager : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "ro.randusoft.PenguinSnap")
public:
    explicit ScreenshotManager(QObject *parent = nullptr);

public slots:
    void captureArea();
    void captureWindow();
    void captureFullscreen();

    Q_SCRIPTABLE void receiveWindowList(const QString &data);

signals:
    void screenshotCaptured(const QImage &image);
    void captureFailed(const QString &error);

private:
    void runSpectacle(const QStringList &args);
    void runSpectacleForSelection(const QStringList &args);
    void enumerateWindows();
    void captureWindowById(const QString &windowId);
    void raiseAndCapture(const QString &windowId);
    QString tempFilePath() const;

    QList<WindowInfo> m_pendingWindows;
};
