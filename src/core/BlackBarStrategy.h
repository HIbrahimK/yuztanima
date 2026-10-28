#ifndef BLACK_BAR_STRATEGY_H
#define BLACK_BAR_STRATEGY_H

#include "HidingStrategy.h"

/**
 * @brief Siyah bant ile yüz gizleme
 *
 * Göz hizasında siyah bir dikdörtgen çizer.
 * Klasik gizlilik koruma yöntemi.
 */
class BlackBarStrategy : public IHidingStrategy {
public:
    BlackBarStrategy() = default;

    void apply(cv::Mat& image, const cv::Rect& faceRect) override;
    QString name() const override;
    QString description() const override;
};

#endif // BLACK_BAR_STRATEGY_H
