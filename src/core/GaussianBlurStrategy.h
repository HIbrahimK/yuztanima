#ifndef GAUSSIAN_BLUR_STRATEGY_H
#define GAUSSIAN_BLUR_STRATEGY_H

#include "HidingStrategy.h"

/**
 * @brief Gaussian Blur ile yüz gizleme
 *
 * Yüz bölgesine Gaussian bulanıklaştırma uygular.
 * Yoğunluk (kernel boyutu) ayarlanabilir.
 */
class GaussianBlurStrategy : public IHidingStrategy {
public:
    /**
     * @brief Oluşturucu
     * @param kernelSize Blur kernel boyutu (tek sayı, 15-99)
     */
    explicit GaussianBlurStrategy(int kernelSize = 51);

    void apply(cv::Mat& image, const cv::Rect& faceRect) override;
    QString name() const override;
    QString description() const override;

    void setKernelSize(int size);
    int kernelSize() const { return m_kernelSize; }

private:
    int m_kernelSize;
};

#endif // GAUSSIAN_BLUR_STRATEGY_H
