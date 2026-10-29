#include "ImageUtils.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QRectF>
#include <QColor>
#include <algorithm>

QImage ImageUtils::matToQImage(const cv::Mat& mat)
{
    if (mat.empty()) {
        return QImage();
    }

    switch (mat.type()) {
    case CV_8UC3: {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        QImage image(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
        return image.copy();
    }
    case CV_8UC4: {
        cv::Mat rgba;
        cv::cvtColor(mat, rgba, cv::COLOR_BGRA2RGBA);
        QImage image(rgba.data, rgba.cols, rgba.rows, static_cast<int>(rgba.step), QImage::Format_RGBA8888);
        return image.copy();
    }
    case CV_8UC1: {
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8);
        return image.copy();
    }
    default:
        qWarning() << "ImageUtils: Desteklenmeyen cv::Mat tipi:" << mat.type();
        return QImage();
    }
}

cv::Mat ImageUtils::qImageToMat(const QImage& image)
{
    if (image.isNull()) {
        return cv::Mat();
    }

    QImage converted = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(converted.height(), converted.width(), CV_8UC3,
                const_cast<uchar*>(converted.bits()), static_cast<size_t>(converted.bytesPerLine()));

    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);
    return bgr.clone();
}

QPixmap ImageUtils::matToQPixmap(const cv::Mat& mat)
{
    return QPixmap::fromImage(matToQImage(mat));
}

QStringList ImageUtils::supportedExtensions()
{
    return {"jpg", "jpeg", "png", "bmp", "webp", "tif", "tiff"};
}

QString ImageUtils::fileFilter()
{
    return QStringLiteral("Fotoğraflar (*.jpg *.jpeg *.png *.bmp *.webp *.tif *.tiff);;Tüm Dosyalar (*.*)");
}

cv::Mat ImageUtils::cropToAspectRatio(const cv::Mat& src, double targetAspect)
{
    if (src.empty() || targetAspect <= 0.0) return src;

    double currentAspect = static_cast<double>(src.cols) / static_cast<double>(src.rows);
    int cropW = src.cols;
    int cropH = src.rows;

    if (currentAspect > targetAspect) {
        // Genişlik fazla: sağdan ve soldan kırp
        cropW = std::clamp(static_cast<int>(src.rows * targetAspect), 1, src.cols);
    } else if (currentAspect < targetAspect) {
        // Yükseklik fazla: üstten ve alttan kırp
        cropH = std::clamp(static_cast<int>(src.cols / targetAspect), 1, src.rows);
    }

    int offsetX = (src.cols - cropW) / 2;
    int offsetY = (src.rows - cropH) / 2;

    cv::Rect roi(offsetX, offsetY, cropW, cropH);
    return src(roi).clone();
}

cv::Mat ImageUtils::resizeImage(const cv::Mat& src, int targetWidth, int targetHeight, bool cropToFit)
{
    if (src.empty() || targetWidth <= 0 || targetHeight <= 0) return src;

    cv::Mat input = src;
    if (cropToFit) {
        double targetAspect = static_cast<double>(targetWidth) / static_cast<double>(targetHeight);
        input = cropToAspectRatio(src, targetAspect);
    }

    cv::Mat output;
    int interp = (targetWidth < input.cols || targetHeight < input.rows) ? cv::INTER_AREA : cv::INTER_CUBIC;
    cv::resize(input, output, cv::Size(targetWidth, targetHeight), 0, 0, interp);
    return output;
}

cv::Mat ImageUtils::applyResizePreset(const cv::Mat& src, ResizeMode mode, int customW, int customH, bool maintainAspect)
{
    if (src.empty()) return src;

    switch (mode) {
    case ResizeMode::Original:
        return src.clone();

    case ResizeMode::MebManset:
        // MEB Okul Web Sitesi Haber Manşeti: En az 1200px genişlik ve 19/9 en-boy oranı (1200 × 568 px)
        return resizeImage(src, 1200, 568, true);

    case ResizeMode::EOkul:
        // MEB e-Okul Standartları: 35x45 mm (133x171 piksel), < 20 KB JPEG
        return resizeImage(src, 133, 171, true);

    case ResizeMode::Mebbis:
        // MEBBİS Vesikalık Standardı: 394x512 piksel
        return resizeImage(src, 394, 512, true);

    case ResizeMode::InstagramSquare:
        // Instagram Kare: 1080x1080 piksel (1:1)
        return resizeImage(src, 1080, 1080, true);

    case ResizeMode::InstagramPortrait:
        // Instagram Dikey / Portre: 1080x1350 piksel (4:5)
        return resizeImage(src, 1080, 1350, true);

    case ResizeMode::InstagramStory:
        // Instagram Hikaye / Reels: 1080x1920 piksel (9:16)
        return resizeImage(src, 1080, 1920, true);

    case ResizeMode::CustomSize:
        return resizeImage(src, customW, customH, maintainAspect);
    }

    return src.clone();
}

cv::Mat ImageUtils::applyWatermark(const cv::Mat& src, const ProcessSettings& settings)
{
    if (src.empty() || settings.watermarkType == WatermarkType::None) {
        return src.clone();
    }

    bool hasImage = (settings.watermarkType == WatermarkType::Image || settings.watermarkType == WatermarkType::Both)
                    && !settings.watermarkImagePath.isEmpty()
                    && QFile::exists(settings.watermarkImagePath);
    bool hasText = (settings.watermarkType == WatermarkType::Text || settings.watermarkType == WatermarkType::Both)
                   && !settings.watermarkText.trimmed().isEmpty();

    if (!hasImage && !hasText) {
        return src.clone();
    }

    QImage qimg = matToQImage(src);
    if (qimg.isNull()) {
        return src.clone();
    }

    QPainter painter(&qimg);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    double opacity = std::clamp(settings.watermarkOpacity, 10, 100) / 100.0;
    painter.setOpacity(opacity);

    int imgW = qimg.width();
    int imgH = qimg.height();
    int margin = std::max(12, imgW / 50);

    QPixmap logoPm;
    int logoW = 0;
    int logoH = 0;
    if (hasImage) {
        logoPm.load(settings.watermarkImagePath);
        if (!logoPm.isNull()) {
            int targetLogoW = std::clamp(imgW * settings.watermarkScalePercent / 100, 24, imgW / 2);
            logoPm = logoPm.scaledToWidth(targetLogoW, Qt::SmoothTransformation);
            logoW = logoPm.width();
            logoH = logoPm.height();
        } else {
            hasImage = false;
        }
    }

    QString text = settings.watermarkText.trimmed();
    QFont font(QStringLiteral("Segoe UI, Arial, sans-serif"));
    int fontSize = std::clamp(imgW / 55, 11, 48);
    font.setPixelSize(fontSize);
    font.setBold(true);
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text);
    int textW = textRect.width();
    int textH = textRect.height();

    int blockW = 0;
    int blockH = 0;
    int spacing = 10;
    int padX = 10;
    int padY = 6;

    if (hasImage && hasText) {
        blockW = padX * 2 + logoW + spacing + textW;
        blockH = padY * 2 + std::max(logoH, textH);
    } else if (hasImage) {
        blockW = logoW;
        blockH = logoH;
    } else if (hasText) {
        blockW = padX * 2 + textW;
        blockH = padY * 2 + textH;
    }

    int x = margin;
    int y = margin;
    switch (settings.watermarkPosition) {
    case WatermarkPosition::BottomRight:
        x = imgW - blockW - margin;
        y = imgH - blockH - margin;
        break;
    case WatermarkPosition::BottomLeft:
        x = margin;
        y = imgH - blockH - margin;
        break;
    case WatermarkPosition::TopRight:
        x = imgW - blockW - margin;
        y = margin;
        break;
    case WatermarkPosition::TopLeft:
        x = margin;
        y = margin;
        break;
    case WatermarkPosition::Center:
        x = (imgW - blockW) / 2;
        y = (imgH - blockH) / 2;
        break;
    case WatermarkPosition::Custom:
        x = static_cast<int>((imgW - blockW) * std::clamp(settings.watermarkCustomXRatio, 0.0, 1.0));
        y = static_cast<int>((imgH - blockH) * std::clamp(settings.watermarkCustomYRatio, 0.0, 1.0));
        break;
    }

    x = std::max(0, x);
    y = std::max(0, y);

    if (settings.watermarkTextBackground && (hasText || hasImage)) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 160));
        QRectF bgRect(x - (hasText ? 0 : 4), y - (hasText ? 0 : 4),
                      blockW + (hasText ? 0 : 8), blockH + (hasText ? 0 : 8));
        painter.drawRoundedRect(bgRect, 8.0, 8.0);
        painter.restore();
    }

    if (hasImage && hasText) {
        int logoX = x + padX;
        int logoY = y + (blockH - logoH) / 2;
        painter.drawPixmap(logoX, logoY, logoPm);

        int textX = logoX + logoW + spacing;
        int textY = y + (blockH - textH) / 2 + fm.ascent();
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(textX, textY, text);
    } else if (hasImage) {
        painter.drawPixmap(x, y, logoPm);
    } else if (hasText) {
        int textX = x + padX;
        int textY = y + padY + fm.ascent();
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(textX, textY, text);
    }

    painter.end();
    return qImageToMat(qimg);
}

bool ImageUtils::saveImageWithSettings(const QString& outputPath, const cv::Mat& image, const ProcessSettings& settings)
{
    if (image.empty()) return false;

    // 1. MEB / Boyutlandırma şablonunu uygula
    cv::Mat finalImage = applyResizePreset(image, settings.resizeMode, settings.customWidth, settings.customHeight, settings.maintainAspectRatio);

    // 2. Okul logosu veya metin filigran uygula
    finalImage = applyWatermark(finalImage, settings);

    // 3. Format ve e-Okul boyutu kontrolü
    if (settings.resizeMode == ResizeMode::EOkul) {
        // MEB e-Okul standartları: JPEG formatında, 133x171 px ve dosya boyutu 20 KB'dan küçük (<20 KB)
        std::vector<uchar> buf;
        int quality = 85;
        cv::imencode(".jpg", finalImage, buf, {cv::IMWRITE_JPEG_QUALITY, quality});

        // 20 KB sınırını garantiye al
        while (buf.size() > 20 * 1024 && quality > 20) {
            quality -= 5;
            cv::imencode(".jpg", finalImage, buf, {cv::IMWRITE_JPEG_QUALITY, quality});
        }

        QFile f(outputPath);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(reinterpret_cast<const char*>(buf.data()), static_cast<qint64>(buf.size()));
            f.close();
            qDebug() << "e-Okul fotoğrafı kaydedildi:" << outputPath << "Boyut:" << (buf.size() / 1024) << "KB";
            return true;
        }
        return false;
    }

    // 3. Maksimum Dosya Boyutu Sınırlaması (KB / MB)
    if (settings.enableMaxFileSize && settings.maxFileSizeKb > 0) {
        size_t maxBytes = static_cast<size_t>(settings.maxFileSizeKb) * 1024;
        std::vector<uchar> buf;
        int quality = std::clamp(settings.jpegQuality, 20, 95);
        cv::Mat workImage = finalImage.clone();

        // Kaliteyi kademeli düşürerek hedef KB'a sığdır
        cv::imencode(".jpg", workImage, buf, {cv::IMWRITE_JPEG_QUALITY, quality});
        while (buf.size() > maxBytes && quality > 15) {
            quality = std::max(15, quality - 5);
            cv::imencode(".jpg", workImage, buf, {cv::IMWRITE_JPEG_QUALITY, quality});
        }

        // Çözünürlüğü de kademeli küçültmeye izin verildiyse ve hala hedef boyuttan büyükse:
        if (buf.size() > maxBytes && settings.maxFileSizeAction == MaxFileSizeAction::CompressAndResize) {
            double scale = 0.9;
            while (buf.size() > maxBytes && scale >= 0.15) {
                cv::Mat scaled;
                cv::resize(finalImage, scaled, cv::Size(), scale, scale, cv::INTER_AREA);
                cv::imencode(".jpg", scaled, buf, {cv::IMWRITE_JPEG_QUALITY, quality});
                if (buf.size() <= maxBytes) {
                    break;
                }
                scale -= 0.1;
            }
        }

        QFile f(outputPath);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(reinterpret_cast<const char*>(buf.data()), static_cast<qint64>(buf.size()));
            f.close();
            qDebug() << "Fotoğraf hedef dosya boyutuyla kaydedildi:" << outputPath
                     << "Boyut:" << (buf.size() / 1024) << "KB"
                     << "(Hedef maks:" << settings.maxFileSizeKb << "KB)";
            return true;
        }
        return false;
    }

    // Normal kayıt işlemi
    std::vector<int> params;
    QString ext = QFileInfo(outputPath).suffix().toLower();

    if (ext == "jpg" || ext == "jpeg") {
        params = {cv::IMWRITE_JPEG_QUALITY, settings.jpegQuality};
    } else if (ext == "png") {
        params = {cv::IMWRITE_PNG_COMPRESSION, 6};
    } else if (ext == "webp") {
        params = {cv::IMWRITE_WEBP_QUALITY, settings.jpegQuality};
    }

    return cv::imwrite(outputPath.toStdString(), finalImage, params);
}

qint64 ImageUtils::getFileSize(const QString& filePath)
{
    return QFileInfo(filePath).size();
}
