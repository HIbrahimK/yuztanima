#ifndef HIDING_STRATEGY_H
#define HIDING_STRATEGY_H

#include <opencv2/core.hpp>
#include <QString>

/**
 * @brief Yüz gizleme stratejisi soyut arayüzü (Strategy Pattern)
 *
 * Farklı gizleme yöntemleri (blur, mozaik, siyah bant, emoji)
 * bu arayüzü uygular ve birbirinin yerine kullanılabilir.
 */
class IHidingStrategy {
public:
    virtual ~IHidingStrategy() = default;

    /**
     * @brief Yüz bölgesine gizleme uygular
     * @param image Tam görüntü (değiştirilecek)
     * @param faceRect Yüz bounding box'ı
     */
    virtual void apply(cv::Mat& image, const cv::Rect& faceRect) = 0;

    /**
     * @brief Strateji adını döndürür
     * @return İnsan okunabilir strateji adı
     */
    virtual QString name() const = 0;

    /**
     * @brief Strateji açıklamasını döndürür
     * @return Kısa açıklama
     */
    virtual QString description() const = 0;
};

#endif // HIDING_STRATEGY_H
