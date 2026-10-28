#include "YuNetFaceDetector.h"
#include <QDebug>
#include <QFileInfo>
#include <opencv2/imgproc.hpp>

YuNetFaceDetector::YuNetFaceDetector(const QString& modelPath)
    : m_modelPath(modelPath)
{
    QFileInfo fi(modelPath);
    if (!fi.exists()) {
        qWarning() << "YuNet model dosyası bulunamadı:" << modelPath;
        return;
    }

    try {
        // Varsayılan giriş boyutu (detect sırasında güncellenir)
        m_detector = cv::FaceDetectorYN::create(
            modelPath.toStdString(),
            "",                         // config (ONNX için boş)
            cv::Size(320, 320),         // input size (sonra güncellenir)
            m_confidenceThreshold,      // score threshold
            0.3f,                       // NMS threshold
            5000                        // top_k
        );
        m_loaded = true;
        qDebug() << "YuNet modeli başarıyla yüklendi:" << modelPath;
    } catch (const cv::Exception& e) {
        qWarning() << "YuNet modeli yüklenemedi:" << e.what();
        m_loaded = false;
    }
}

std::vector<cv::Rect> YuNetFaceDetector::detect(const cv::Mat& image)
{
    std::vector<cv::Rect> faces;

    if (!m_loaded || image.empty()) {
        return faces;
    }

    try {
        // Giriş boyutunu görüntüye göre ayarla
        m_detector->setInputSize(image.size());

        // Algılama çalıştır
        cv::Mat detections;
        m_detector->detect(image, detections);

        if (detections.empty()) {
            return faces;
        }

        // Sonuçları ayrıştır
        // YuNet çıktısı: [num_faces, 15]
        // Sütun 0-3: x, y, w, h (bounding box)
        // Sütun 14: güven skoru
        for (int i = 0; i < detections.rows; ++i) {
            float confidence = detections.at<float>(i, 14);

            if (confidence < m_confidenceThreshold) {
                continue;
            }

            int x = static_cast<int>(detections.at<float>(i, 0));
            int y = static_cast<int>(detections.at<float>(i, 1));
            int w = static_cast<int>(detections.at<float>(i, 2));
            int h = static_cast<int>(detections.at<float>(i, 3));

            // Minimum boyut kontrolü
            if (w < m_minFaceSize || h < m_minFaceSize) {
                continue;
            }

            // Görüntü sınırlarını aşmamasını sağla
            x = std::max(0, x);
            y = std::max(0, y);
            w = std::min(w, image.cols - x);
            h = std::min(h, image.rows - y);

            if (w > 0 && h > 0) {
                faces.emplace_back(x, y, w, h);
            }
        }

    } catch (const cv::Exception& e) {
        qWarning() << "YuNet algılama hatası:" << e.what();
    }

    return faces;
}

void YuNetFaceDetector::setConfidenceThreshold(float threshold)
{
    m_confidenceThreshold = std::clamp(threshold, 0.0f, 1.0f);
    if (m_detector) {
        m_detector->setScoreThreshold(m_confidenceThreshold);
    }
}

void YuNetFaceDetector::setMinFaceSize(int size)
{
    m_minFaceSize = std::max(10, size);
}

QString YuNetFaceDetector::name() const
{
    return QStringLiteral("YuNet (DNN)");
}

bool YuNetFaceDetector::isLoaded() const
{
    return m_loaded;
}

void YuNetFaceDetector::recreateDetector(const cv::Size& inputSize)
{
    if (!m_loaded) return;

    try {
        m_detector = cv::FaceDetectorYN::create(
            m_modelPath.toStdString(),
            "",
            inputSize,
            m_confidenceThreshold,
            0.3f,
            5000
        );
    } catch (const cv::Exception& e) {
        qWarning() << "YuNet detector yeniden oluşturulamadı:" << e.what();
    }
}
