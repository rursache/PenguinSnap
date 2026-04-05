#pragma once

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QRect>

#include "screenshotmanager.h"

class WindowOverlay : public QWidget {
    Q_OBJECT
public:
    explicit WindowOverlay(const QImage &screenshot, const QList<WindowInfo> &windows,
                           QWidget *parent = nullptr);

signals:
    void windowSelected(const QString &windowId);
    void selectionCancelled();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QRect imageRect(const QRect &widgetRect) const;
    int windowIndexAt(const QPoint &pos) const;

    QImage m_screenshot;
    QPixmap m_darkPixmap;
    QList<WindowInfo> m_windows;
    int m_hoveredIndex = -1;
};
