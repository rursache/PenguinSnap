#include "windowoverlay.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

#include <LayerShellQt/Window>

WindowOverlay::WindowOverlay(const QImage &screenshot, const QList<WindowInfo> &windows,
                             QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint)
    , m_screenshot(screenshot)
    , m_windows(windows)
{
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);

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
            lsWin->setScope(QStringLiteral("penguinsnap-window-overlay"));
        }
    }

    show();
}

void WindowOverlay::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    qreal dpr = devicePixelRatio();
    QSize physicalSize(qRound(event->size().width() * dpr),
                       qRound(event->size().height() * dpr));

    m_darkPixmap = QPixmap::fromImage(m_screenshot).scaled(
        physicalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_darkPixmap.setDevicePixelRatio(dpr);

    QPainter p(&m_darkPixmap);
    p.fillRect(m_darkPixmap.rect(), QColor(0, 0, 0, 128));
}

void WindowOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.drawPixmap(0, 0, m_darkPixmap);

    if (m_hoveredIndex >= 0 && m_hoveredIndex < m_windows.size()) {
        QRect winRect = m_windows[m_hoveredIndex].rect.intersected(rect());
        if (!winRect.isEmpty()) {
            QRect src = imageRect(winRect);
            p.drawImage(winRect, m_screenshot, src);

            p.setPen(QPen(QColor(61, 174, 233), 2));
            p.drawRect(winRect.adjusted(0, 0, -1, -1));
        }
    }
}

void WindowOverlay::mouseMoveEvent(QMouseEvent *event)
{
    int idx = windowIndexAt(event->pos());
    if (idx != m_hoveredIndex) {
        m_hoveredIndex = idx;
        update();
    }
}

void WindowOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_hoveredIndex >= 0) {
        QString windowId = m_windows[m_hoveredIndex].id;
        emit windowSelected(windowId);
        close();
        deleteLater();
    }
}

void WindowOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit selectionCancelled();
        close();
        deleteLater();
    }
}

QRect WindowOverlay::imageRect(const QRect &widgetRect) const
{
    if (width() <= 0 || height() <= 0)
        return {};
    qreal sx = m_screenshot.width()  / static_cast<qreal>(width());
    qreal sy = m_screenshot.height() / static_cast<qreal>(height());
    return QRect(
        qRound(widgetRect.x() * sx),
        qRound(widgetRect.y() * sy),
        qRound(widgetRect.width() * sx),
        qRound(widgetRect.height() * sy));
}

int WindowOverlay::windowIndexAt(const QPoint &pos) const
{
    for (int i = 0; i < m_windows.size(); ++i) {
        if (m_windows[i].rect.contains(pos))
            return i;
    }
    return -1;
}
