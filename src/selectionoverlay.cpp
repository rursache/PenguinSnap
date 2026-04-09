#include "selectionoverlay.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

#include <LayerShellQt/Window>

SelectionOverlay::SelectionOverlay(const QImage &screenshot, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint)
    , m_screenshot(screenshot)
{
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);

    // Create native window handle without mapping the surface,
    // so no xdg_toplevel role is assigned before LayerShellQt configures it.
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
            lsWin->setScope(QStringLiteral("penguinsnap-overlay"));
        }
    }

    show();
}

void SelectionOverlay::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    qreal dpr = devicePixelRatio();
    QSize physicalSize(qRound(event->size().width() * dpr),
                       qRound(event->size().height() * dpr));

    m_darkPixmap = QPixmap::fromImage(m_screenshot).scaled(
        physicalSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_darkPixmap.setDevicePixelRatio(dpr);

    QPainter p(&m_darkPixmap);
    p.fillRect(m_darkPixmap.rect(), QColor(0, 0, 0, 100));
}

void SelectionOverlay::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.drawPixmap(0, 0, m_darkPixmap);

    if (m_selecting) {
        QRect sel = QRect(m_startPos, m_currentPos).normalized();
        if (sel.width() > 1 && sel.height() > 1) {
            QRect src = imageRect(sel);
            p.drawImage(sel, m_screenshot, src);

            p.setPen(QPen(Qt::white, 1));
            p.drawRect(sel);

            QString dims = QStringLiteral("%1 × %2").arg(src.width()).arg(src.height());
            QFont f = p.font();
            f.setPointSize(10);
            p.setFont(f);
            int tx = sel.x();
            int ty = sel.y() - 6;
            if (ty < 12)
                ty = sel.bottom() + 18;
            p.setPen(Qt::white);
            p.drawText(tx, ty, dims);
        }
    }
}

void SelectionOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_startPos = event->pos();
        m_currentPos = event->pos();
        m_selecting = true;
    }
}

void SelectionOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (m_selecting) {
        m_currentPos = event->pos();
        update();
    }
}

void SelectionOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_selecting) {
        m_selecting = false;
        QRect sel = QRect(m_startPos, event->pos()).normalized();
        if (sel.width() > 5 && sel.height() > 5) {
            QRect src = imageRect(sel);
            QImage cropped = m_screenshot.copy(src);
            emit regionSelected(cropped);
            emit rectSelected(sel);
        } else {
            emit selectionCancelled();
        }
        close();
        deleteLater();
    }
}

void SelectionOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit selectionCancelled();
        close();
        deleteLater();
    }
}

QRect SelectionOverlay::imageRect(const QRect &widgetRect) const
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
