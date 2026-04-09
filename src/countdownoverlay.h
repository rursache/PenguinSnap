#pragma once

#include <QWidget>
#include <QRect>

class QTimer;

class CountdownOverlay : public QWidget {
    Q_OBJECT
public:
    explicit CountdownOverlay(int seconds, const QRect &selectedRect = QRect(),
                              QWidget *parent = nullptr);

signals:
    void countdownFinished();
    void countdownCancelled();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void tick();
    void playTick();

    QTimer *m_timer;
    int m_remaining;
    QRect m_selectedRect;
};
