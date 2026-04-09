#include "countdownoverlay.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QProcess>
#include <QTimer>

#include <LayerShellQt/Window>

CountdownOverlay::CountdownOverlay(int seconds, const QRect &selectedRect, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint)
    , m_remaining(seconds)
    , m_selectedRect(selectedRect)
{
    setAttribute(Qt::WA_TranslucentBackground);

    winId();

    QWindow *win = windowHandle();
    if (win) {
        if (auto *lsWin = LayerShellQt::Window::get(win)) {
            lsWin->setLayer(LayerShellQt::Window::LayerOverlay);
            lsWin->setAnchors({LayerShellQt::Window::AnchorTop,
                               LayerShellQt::Window::AnchorBottom,
                               LayerShellQt::Window::AnchorLeft,
                               LayerShellQt::Window::AnchorRight});
            lsWin->setExclusiveZone(-1);
            lsWin->setKeyboardInteractivity(
                LayerShellQt::Window::KeyboardInteractivityExclusive);
            lsWin->setScope(QStringLiteral("penguinsnap-countdown"));
        }
    }

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &CountdownOverlay::tick);

    show();
    playTick();
    m_timer->start();
}

void CountdownOverlay::tick()
{
    m_remaining--;
    if (m_remaining <= 0) {
        m_timer->stop();
        emit countdownFinished();
        close();
        deleteLater();
    } else {
        playTick();
        update();
    }
}

void CountdownOverlay::playTick()
{
    QProcess::startDetached(QStringLiteral("canberra-gtk-play"),
                            {QStringLiteral("--id=message"), QStringLiteral("--description=countdown")});
}

void CountdownOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (m_selectedRect.isValid()) {
        // Timed area: dark overlay with selected area cut out
        QPainterPath overlayPath;
        overlayPath.addRect(rect());
        QPainterPath cutout;
        cutout.addRect(m_selectedRect);
        overlayPath = overlayPath.subtracted(cutout);

        p.fillPath(overlayPath, QColor(0, 0, 0, 64));

        // White border around selected area
        p.setPen(QPen(Qt::white, 2));
        p.drawRect(m_selectedRect);
    } else {
        // Timed fullscreen: uniform dark overlay
        p.fillRect(rect(), QColor(0, 0, 0, 77));
    }

    // Draw countdown number
    QFont f = font();
    f.setPixelSize(120);
    f.setBold(true);
    p.setFont(f);
    p.setPen(Qt::white);

    QString text = QString::number(m_remaining);
    QFontMetrics fm(f);
    QRect textRect = fm.boundingRect(text);
    int x = (width() - textRect.width()) / 2;
    int y = (height() + fm.ascent() - fm.descent()) / 2;
    p.drawText(x, y, text);
}

void CountdownOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        m_timer->stop();
        emit countdownCancelled();
        close();
        deleteLater();
    }
}
