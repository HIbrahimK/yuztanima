#include "HaarFaceDetector.h"
#include <QDebug>
#include <QFileInfo>
#include <opencv2/imgproc.hpp>

HaarFaceDetector::HaarFaceDetector(const QString& cascadePath)
{
    QFileInfo fi(cascadePath);
    if (!fi.exists()) {
        qWarning() << "Haar Cascade dosyası bulunamadı:" << cascadePath;
        return;
    }

    if (m_cascade.load(cascadePath.toStdString())) {
        m_loaded = true;
        qDebug() << "Haar Cascade başarıyla yüklendi:" << cascadePath;
    } else {
        qWarning() << "Haar Cascade yüklenemedi:" << cascadePath;
    }
}

std::vector<cv::Rect> HaarFaceDetector::detect(const cv::Mat& image)
{
    std::vector<cv::Rect> faces;

    if (!m_loaded || image.empty()) {
        return faces;
    }

    try {
        // Gri tonlamaya çevir (Haar Cascade gereksinimi)
        cv::Mat gray;
        if (image.channels() == 3) {
            cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        } else {
            gray = image.clone();
        }

        // Histogram eşitleme (aydınlatma normalleştirme)
        cv::equalizeHist(gray, gray);

        // Yüz algılama
        m_cascade.detectMultiScale(
            gray,
            faces,
            m_scaleFactor,                              // scaleFactor
            m_minNeighbors,                             // minNeighbors
            0,                                          // flags
            cv::Size(m_minFaceSize, m_minFaceSize)      // minSize
        );

    } catch (const cv::Exception& e) {
        qWarning() << "Haar Cascade algılama hatası:" << e.what();
    }

    return faces;
}

void HaarFaceDetector::setConfidenceThreshold(float threshold)
{
    // Haar Cascade'de doğrudan güven eşiği yoktur.
    // Bunun yerine scaleFactor ve minNeighbors kullanılır.
    // Yüksek threshold → düşük scaleFactor, yüksek minNeighbors
    m_scaleFactor = 1.05f + (1.0f - threshold) * 0.25f; // 1.05 - 1.30 arası
    m_minNeighbors = static_cast<int>(3 + threshold * 7); // 3 - 10 arası
}

void HaarFaceDetector::setMinFaceSize(int size)
{
    m_minFaceSize = std::max(10, size);
}

QString HaarFaceDetector::name() const
{
    return QStringLiteral("Haar Cascade");
}

bool HaarFaceDetector::isLoaded() const
{
    return m_loaded;
}
