#pragma once

#include <QWidget>
#include <QImage>
#include <QPixmap>
#include <QPoint>

class SelectionOverlay : public QWidget {
    Q_OBJECT
public:
    explicit SelectionOverlay(const QImage &screenshot, QWidget *parent = nullptr);

signals:
    void regionSelected(const QImage &cropped);
    void rectSelected(const QRect &screenRect);
    void selectionCancelled();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QRect imageRect(const QRect &widgetRect) const;

    QImage m_screenshot;
    QPixmap m_darkPixmap;
    QPoint m_startPos;
    QPoint m_currentPos;
    bool m_selecting = false;
};
