#ifndef HAAR_FACE_DETECTOR_H
#define HAAR_FACE_DETECTOR_H

#include "FaceDetector.h"
#include <opencv2/core/version.hpp>
#include <opencv2/objdetect.hpp>
#if __has_include(<opencv2/xobjdetect.hpp>)
#include <opencv2/xobjdetect.hpp>
#endif
#include <QString>

/**
 * @brief Haar Cascade tabanlı yüz algılayıcı
 *
 * Klasik Haar Cascade yöntemi ile yüz algılama.
 * Hızlı ama sadece ön cephe yüzleri algılar.
 * OpenCV 5.0'da contrib modülüne taşınmış olabilir;
 * CascadeClassifier hâlâ core'da mevcuttur.
 */
class HaarFaceDetector : public IFaceDetector {
public:
    /**
     * @brief Haar Cascade algılayıcısını oluşturur
     * @param cascadePath XML cascade dosyası yolu
     */
    explicit HaarFaceDetector(const QString& cascadePath);
    ~HaarFaceDetector() override = default;

    std::vector<cv::Rect> detect(const cv::Mat& image) override;
    void setConfidenceThreshold(float threshold) override;
    void setMinFaceSize(int size) override;
    QString name() const override;
    bool isLoaded() const override;

private:
    cv::CascadeClassifier m_cascade;
    float m_scaleFactor = 1.1f;     ///< scaleFactor (güven eşiği yerine)
    int m_minNeighbors = 5;         ///< Min komşu sayısı
    int m_minFaceSize = 30;
    bool m_loaded = false;
};

#endif // HAAR_FACE_DETECTOR_H
