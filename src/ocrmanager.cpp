#include "ocrmanager.h"

#include <tesseract/baseapi.h>
#include <QDir>
#include <QThread>

OCRManager::OCRManager(QObject *parent)
    : QObject(parent)
{
}

QStringList OCRManager::availableLanguages() const
{
    QStringList languages;

    QStringList searchPaths = {
        QStringLiteral("/usr/share/tessdata"),
        QStringLiteral("/usr/share/tesseract-ocr/5/tessdata"),
        QStringLiteral("/usr/local/share/tessdata"),
    };

    QString envPath = qEnvironmentVariable("TESSDATA_PREFIX");
    if (!envPath.isEmpty())
        searchPaths.prepend(envPath);

    for (const auto &path : searchPaths) {
        QDir dir(path);
        if (!dir.exists())
            continue;
        const auto entries = dir.entryList({QStringLiteral("*.traineddata")}, QDir::Files);
        for (const auto &entry : entries) {
            QString lang = entry.left(entry.indexOf(QLatin1Char('.')));
            if (!languages.contains(lang))
                languages << lang;
        }
    }

    languages.sort();

    // Ensure English is listed first when available
    if (languages.removeAll(QStringLiteral("eng")))
        languages.prepend(QStringLiteral("eng"));

    return languages;
}

void OCRManager::recognizeText(const QImage &image, const QString &language)
{
    QThread *thread = QThread::create([this, image, language]() {
        auto api = std::make_unique<tesseract::TessBaseAPI>();

        if (api->Init(nullptr, language.toUtf8().constData()) != 0) {
            QMetaObject::invokeMethod(this, [this]() {
                emit ocrFailed(QStringLiteral("Failed to initialize Tesseract OCR engine"));
            }, Qt::QueuedConnection);
            return;
        }

        QImage gray = image.convertToFormat(QImage::Format_Grayscale8);
        api->SetImage(gray.constBits(), gray.width(), gray.height(),
                      1, static_cast<int>(gray.bytesPerLine()));

        char *rawText = api->GetUTF8Text();
        QString text = QString::fromUtf8(rawText).trimmed();
        delete[] rawText;

        api->End();

        if (text.isEmpty()) {
            QMetaObject::invokeMethod(this, [this]() {
                emit ocrFailed(QStringLiteral("No text detected in the selected area"));
            }, Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(this, [this, text]() {
                emit textRecognized(text);
            }, Qt::QueuedConnection);
        }
    });

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}
