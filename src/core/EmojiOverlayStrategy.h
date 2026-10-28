#ifndef EMOJI_OVERLAY_STRATEGY_H
#define EMOJI_OVERLAY_STRATEGY_H

#include "HidingStrategy.h"
#include <QString>
#include <opencv2/core.hpp>

/**
 * @brief Emoji/Smiley kaplama ile yüz gizleme
 *
 * Yüz bölgesinin üzerine alfa kanallı bir emoji PNG dosyası kaplar.
 * Emoji boyutu yüz boyutuna göre otomatik ölçeklenir.
 */
class EmojiOverlayStrategy : public IHidingStrategy {
public:
    /**
     * @brief Oluşturucu
     * @param emojiPath Emoji PNG dosyası yolu (BGRA, alfa kanallı)
     */
    explicit EmojiOverlayStrategy(const QString& emojiPath);

    void apply(cv::Mat& image, const cv::Rect& faceRect) override;
    QString name() const override;
    QString description() const override;

    /// Emoji dosyasını değiştirir
    bool setEmojiPath(const QString& path);

    /// Emoji dosyasının yüklenip yüklenmediğini kontrol eder
    bool isLoaded() const { return !m_emoji.empty(); }

private:
    cv::Mat m_emoji;          ///< Emoji görüntüsü (BGRA)
    QString m_emojiPath;

    /**
     * @brief Alfa blending ile overlay uygular
     * @param background Hedef görüntü (BGR)
     * @param overlay Emoji görüntüsü (BGRA, boyutlandırılmış)
     * @param position Sol üst köşe konumu
     */
    void overlayImage(cv::Mat& background, const cv::Mat& overlay,
                      const cv::Point& position);
};

#endif // EMOJI_OVERLAY_STRATEGY_H
