#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#include "ProcessSettings.h"
#include <opencv2/core.hpp>
#include <QImage>
#include <QPixmap>
#include <QStringList>

/**
 * @brief Görüntü dönüşüm ve MEB boyutlandırma yardımcıları
 */
class ImageUtils {
public:
    /// cv::Mat (BGR) → QImage
    static QImage matToQImage(const cv::Mat& mat);

    /// QImage → cv::Mat (BGR)
    static cv::Mat qImageToMat(const QImage& image);

    /// cv::Mat (BGR) → QPixmap (doğrudan)
    static QPixmap matToQPixmap(const cv::Mat& mat);

    /// Desteklenen görüntü uzantılarını döndürür
    static QStringList supportedExtensions();

    /// Dosya filtresi stringi (dosya seçici için)
    static QString fileFilter();

    /// Belirtilen en-boy oranına göre ortadan kırpar
    static cv::Mat cropToAspectRatio(const cv::Mat& src, double targetAspect);

    /// Görüntüyü boyutlandırır (isteğe bağlı en-boy oranını koruyarak kırpar)
    static cv::Mat resizeImage(const cv::Mat& src, int targetWidth, int targetHeight, bool cropToFit = true);

    /// MEB şablonuna göre boyutlandırır (e-Okul, MEBBİS, Manşet 19:9, Instagram vb.)
    static cv::Mat applyResizePreset(const cv::Mat& src, ResizeMode mode, int customW = 1200, int customH = 568, bool maintainAspect = true);

    /// Görüntüye okul logosu ve/veya metin filigran uygular
    static cv::Mat applyWatermark(const cv::Mat& src, const ProcessSettings& settings);

    /// Görüntüyü ProcessSettings ayarlarına göre kaydeder (e-Okul <20KB boyut hedeflemesi ve filigran dahil)
    static bool saveImageWithSettings(const QString& outputPath, const cv::Mat& image, const ProcessSettings& settings);

    /// Dosya boyutunu byte cinsinden döndürür
    static qint64 getFileSize(const QString& filePath);
};

#endif // IMAGE_UTILS_H
