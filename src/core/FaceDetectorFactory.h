#ifndef FACE_DETECTOR_FACTORY_H
#define FACE_DETECTOR_FACTORY_H

#include "FaceDetector.h"
#include "ProcessSettings.h"
#include <memory>
#include <QString>

/**
 * @brief Yüz algılayıcı fabrikası (Factory Pattern)
 *
 * Seçilen algılama yöntemine göre uygun IFaceDetector
 * implementasyonunu oluşturur.
 */
class FaceDetectorFactory {
public:
    /**
     * @brief Algılayıcı oluşturur
     * @param type Algılayıcı tipi (YuNet veya HaarCascade)
     * @param modelsDir Model dosyalarının bulunduğu dizin
     * @return Oluşturulan algılayıcı (unique_ptr)
     */
    static std::unique_ptr<IFaceDetector> create(
        DetectorType type,
        const QString& modelsDir
    );

    /**
     * @brief Model dosyasının var olup olmadığını kontrol eder
     * @param type Algılayıcı tipi
     * @param modelsDir Model dizini
     * @return true ise model dosyası mevcut
     */
    static bool isModelAvailable(DetectorType type, const QString& modelsDir);

    /// YuNet ONNX model dosya adı
    static constexpr const char* YUNET_MODEL_FILE = "face_detection_yunet_2023mar.onnx";

    /// Haar Cascade XML dosya adı
    static constexpr const char* HAAR_CASCADE_FILE = "haarcascade_frontalface_default.xml";
};

#endif // FACE_DETECTOR_FACTORY_H
