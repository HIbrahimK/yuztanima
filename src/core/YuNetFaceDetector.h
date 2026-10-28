#ifndef YUNET_FACE_DETECTOR_H
#define YUNET_FACE_DETECTOR_H

#include "FaceDetector.h"
#include <opencv2/objdetect.hpp>
#include <QString>
#include <memory>

/**
 * @brief YuNet tabanlı yüz algılayıcı (DNN/ONNX)
 *
 * OpenCV 5.0'ın cv::FaceDetectorYN arayüzünü kullanır.
 * Yüksek doğruluk, profil yüzleri de algılar.
 * ONNX model dosyası gerektirir.
 */
class YuNetFaceDetector : public IFaceDetector {
public:
    /**
     * @brief YuNet algılayıcısını oluşturur
     * @param modelPath ONNX model dosyası yolu
     */
    explicit YuNetFaceDetector(const QString& modelPath);
    ~YuNetFaceDetector() override = default;

    std::vector<cv::Rect> detect(const cv::Mat& image) override;
    void setConfidenceThreshold(float threshold) override;
    void setMinFaceSize(int size) override;
    QString name() const override;
    bool isLoaded() const override;

private:
    cv::Ptr<cv::FaceDetectorYN> m_detector;
    float m_confidenceThreshold = 0.7f;
    int m_minFaceSize = 30;
    bool m_loaded = false;
    QString m_modelPath;

    /// Algılayıcıyı yeniden oluşturur (parametre değişikliğinde)
    void recreateDetector(const cv::Size& inputSize);
};

#endif // YUNET_FACE_DETECTOR_H
