#pragma once

#include <QObject>
#include <QImage>
#include <QStringList>

class OCRManager : public QObject {
    Q_OBJECT
public:
    explicit OCRManager(QObject *parent = nullptr);

    QStringList availableLanguages() const;

public slots:
    void recognizeText(const QImage &image, const QString &language = QStringLiteral("eng"));

signals:
    void textRecognized(const QString &text);
    void ocrFailed(const QString &error);
};
