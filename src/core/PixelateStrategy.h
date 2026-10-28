#ifndef PIXELATE_STRATEGY_H
#define PIXELATE_STRATEGY_H

#include "HidingStrategy.h"

/**
 * @brief Pikselleştirme (mozaik) ile yüz gizleme
 *
 * Yüz bölgesini küçültüp tekrar büyüterek mozaik efekti oluşturur.
 * TV sansürü tarzı bir görünüm sağlar.
 */
class PixelateStrategy : public IHidingStrategy {
public:
    /**
     * @brief Oluşturucu
     * @param pixelSize Mozaik piksel boyutu (5-30)
     */
    explicit PixelateStrategy(int pixelSize = 10);

    void apply(cv::Mat& image, const cv::Rect& faceRect) override;
    QString name() const override;
    QString description() const override;

    void setPixelSize(int size);
    int pixelSize() const { return m_pixelSize; }

private:
    int m_pixelSize;
};

#endif // PIXELATE_STRATEGY_H
