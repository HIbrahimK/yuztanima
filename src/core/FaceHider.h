#ifndef FACE_HIDER_H
#define FACE_HIDER_H

#include "FaceDetector.h"
#include "HidingStrategy.h"
#include "ProcessSettings.h"
#include <opencv2/core.hpp>
#include <memory>
#include <vector>

/**
 * @brief Yüz algılama ve gizleme orkestrasyonu
 *
 * FaceDetector ve HidingStrategy bileşenlerini koordine eder.
 * Tek bir görüntüdeki tüm yüzleri algılayıp gizler.
 */
class FaceHider {
public:
    FaceHider();
    ~FaceHider() = default;

    /**
     * @brief Algılayıcıyı ayarlar
     * @param detector Yüz algılayıcı (ownership transfer)
     */
    void setDetector(std::unique_ptr<IFaceDetector> detector);

    /**
     * @brief Gizleme stratejisini ayarlar
     * @param strategy Gizleme stratejisi (ownership transfer)
     */
    void setStrategy(std::unique_ptr<IHidingStrategy> strategy);

    /**
     * @brief Görüntüdeki yüzleri algılar ve gizler
     * @param image Giriş görüntüsü (yerinde değiştirilir)
     * @return Algılanan ve gizlenen yüz sayısı
     */
    int processImage(cv::Mat& image);

    /**
     * @brief Sadece yüzleri algılar (gizleme yapmadan)
     * @param image Giriş görüntüsü
     * @return Algılanan yüzlerin bounding box'ları
     */
    std::vector<cv::Rect> detectFaces(const cv::Mat& image);

    /**
     * @brief Algılayıcının yüklenip yüklenmediğini kontrol eder
     */
    bool isReady() const;

    /**
     * @brief Mevcut algılayıcının adını döndürür
     */
    QString detectorName() const;

    /**
     * @brief Mevcut stratejinin adını döndürür
     */
    QString strategyName() const;

private:
    std::unique_ptr<IFaceDetector> m_detector;
    std::unique_ptr<IHidingStrategy> m_strategy;
};

#endif // FACE_HIDER_H
