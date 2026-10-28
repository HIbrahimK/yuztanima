#include "EmojiOverlayStrategy.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <QDebug>
#include <QFileInfo>

EmojiOverlayStrategy::EmojiOverlayStrategy(const QString& emojiPath)
{
    setEmojiPath(emojiPath);
}

void EmojiOverlayStrategy::apply(cv::Mat& image, const cv::Rect& faceRect)
{
    if (image.empty() || m_emoji.empty()) return;

    cv::Rect safeRect = faceRect & cv::Rect(0, 0, image.cols, image.rows);
    if (safeRect.empty()) return;

    // Emoji'yi yüz boyutuna ölçekle (%120 boyutunda — yüzü tamamen kaplasın)
    int emojiW = static_cast<int>(safeRect.width * 1.2);
    int emojiH = static_cast<int>(safeRect.height * 1.2);

    cv::Mat resizedEmoji;
    cv::resize(m_emoji, resizedEmoji, cv::Size(emojiW, emojiH),
               0, 0, cv::INTER_AREA);

    // Emoji'yi yüzün merkezine yerleştir
    int offsetX = safeRect.x + (safeRect.width - emojiW) / 2;
    int offsetY = safeRect.y + (safeRect.height - emojiH) / 2;

    overlayImage(image, resizedEmoji, cv::Point(offsetX, offsetY));
}

QString EmojiOverlayStrategy::name() const
{
    return QStringLiteral("Emoji / Smiley");
}

QString EmojiOverlayStrategy::description() const
{
    return QStringLiteral("Yüz bölgesinin üzerine emoji/smiley kaplar");
}

#include <QImage>
#include <QFile>

bool EmojiOverlayStrategy::setEmojiPath(const QString& path)
{
    m_emojiPath = path;

    QImage qimg;
    if (!path.isEmpty()) {
        qimg.load(path);
    }
    // Eğer dosya doğrudan yüklenemediyse (örn. ":/emojis/..." veya dosya adı ise):
    if (qimg.isNull() && !path.isEmpty()) {
        QString baseName = QFileInfo(path).fileName();
        qimg.load(QStringLiteral(":/emojis/") + baseName);
    }
    // Disk üzerindeki resources/emojis/ klasörünü dene:
    if (qimg.isNull() && !path.isEmpty()) {
        QString diskPath = QStringLiteral("resources/emojis/") + QFileInfo(path).fileName();
        qimg.load(diskPath);
    }
    // Son çare: varsayılan gömülü smile.png
    if (qimg.isNull()) {
        qimg.load(QStringLiteral(":/emojis/smile.png"));
    }

    if (qimg.isNull()) {
        qWarning() << "Emoji hiçbir kaynaktan yüklenemedi:" << path;
        m_emoji = cv::Mat();
        return false;
    }

    // QImage'i RGBA8888 formatına dönüştür
    QImage converted = qimg.convertToFormat(QImage::Format_RGBA8888);
    // RGBA -> BGRA cv::Mat
    cv::Mat rgbaMat(converted.height(), converted.width(), CV_8UC4,
                    const_cast<uchar*>(converted.bits()), static_cast<size_t>(converted.bytesPerLine()));
    cv::Mat bgraMat;
    cv::cvtColor(rgbaMat, bgraMat, cv::COLOR_RGBA2BGRA);
    m_emoji = bgraMat.clone();

    qDebug() << "Emoji başarıyla yüklendi:" << path
             << "boyut:" << m_emoji.cols << "x" << m_emoji.rows;
    return true;
}

void EmojiOverlayStrategy::overlayImage(cv::Mat& background,
                                         const cv::Mat& overlay,
                                         const cv::Point& position)
{
    // Kaplama bölgesini hesapla (görüntü sınırları içinde)
    int x = position.x;
    int y = position.y;

    // Overlay'in görüntü içinde kalan kısmını hesapla
    int startX = std::max(0, x);
    int startY = std::max(0, y);
    int endX = std::min(background.cols, x + overlay.cols);
    int endY = std::min(background.rows, y + overlay.rows);

    if (startX >= endX || startY >= endY) return;

    // Overlay'deki karşılık gelen bölge
    int ovStartX = startX - x;
    int ovStartY = startY - y;
    int roiW = endX - startX;
    int roiH = endY - startY;

    // Alfa blending piksel piksel
    for (int row = 0; row < roiH; ++row) {
        for (int col = 0; col < roiW; ++col) {
            const cv::Vec4b& overlayPixel = overlay.at<cv::Vec4b>(
                ovStartY + row, ovStartX + col);

            float alpha = overlayPixel[3] / 255.0f;

            if (alpha < 0.01f) continue; // Tamamen şeffaf, atla

            cv::Vec3b& bgPixel = background.at<cv::Vec3b>(
                startY + row, startX + col);

            // Alfa blending: result = overlay * alpha + background * (1 - alpha)
            bgPixel[0] = static_cast<uchar>(overlayPixel[0] * alpha + bgPixel[0] * (1.0f - alpha));
            bgPixel[1] = static_cast<uchar>(overlayPixel[1] * alpha + bgPixel[1] * (1.0f - alpha));
            bgPixel[2] = static_cast<uchar>(overlayPixel[2] * alpha + bgPixel[2] * (1.0f - alpha));
        }
    }
}
