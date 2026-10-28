#ifndef FACE_DETECTOR_H
#define FACE_DETECTOR_H

#include <vector>
#include <QString>
#include <opencv2/core.hpp>

/**
 * @brief Yüz algılayıcı soyut arayüzü
 *
 * Tüm yüz algılama stratejileri bu arayüzü uygular.
 * Strategy Pattern kullanılarak farklı algılayıcılar (YuNet, Haar)
 * kolayca değiştirilebilir.
 */
class IFaceDetector {
public:
    virtual ~IFaceDetector() = default;

    /**
     * @brief Görüntüdeki yüzleri algılar
     * @param image Giriş görüntüsü (BGR formatında)
     * @return Algılanan yüzlerin bounding box'ları
     */
    virtual std::vector<cv::Rect> detect(const cv::Mat& image) = 0;

    /**
     * @brief Güven eşiğini ayarlar
     * @param threshold 0.0 - 1.0 arası değer
     */
    virtual void setConfidenceThreshold(float threshold) = 0;

    /**
     * @brief Minimum yüz boyutunu ayarlar
     * @param size Piksel cinsinden minimum yüz genişliği/yüksekliği
     */
    virtual void setMinFaceSize(int size) = 0;

    /**
     * @brief Algılayıcının adını döndürür
     * @return Algılayıcı adı (örn. "YuNet", "Haar Cascade")
     */
    virtual QString name() const = 0;

    /**
     * @brief Algılayıcının başarıyla yüklenip yüklenmediğini kontrol eder
     * @return true ise model yüklü ve kullanıma hazır
     */
    virtual bool isLoaded() const = 0;
};

#endif // FACE_DETECTOR_H
