#include "BatchProcessor.h"
#include "FaceDetectorFactory.h"
#include "GaussianBlurStrategy.h"
#include "PixelateStrategy.h"
#include "BlackBarStrategy.h"
#include "EmojiOverlayStrategy.h"
#include "ImageUtils.h"

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QElapsedTimer>
#include <opencv2/imgcodecs.hpp>

BatchProcessor::BatchProcessor(QObject* parent)
    : QObject(parent)
{
}

void BatchProcessor::setManualOverrides(const QMap<QString, QList<FaceItem>>& overrides)
{
    m_manualOverrides = overrides;
}

void BatchProcessor::processBatch(const QStringList& files,
                                  const ProcessSettings& settings,
                                  const QString& modelsDir)
{
    if (files.isEmpty()) return;

    m_cancelled = false;
    m_running = true;

    QElapsedTimer timer;
    timer.start();

    BatchResult result;
    result.totalFiles = files.size();

    // Algılayıcıyı oluştur
    auto detector = FaceDetectorFactory::create(settings.detectorType, modelsDir);
    if (!detector || !detector->isLoaded()) {
        emit errorOccurred("", tr("Yüz algılayıcı modeli yüklenemedi!"));
        m_running = false;
        return;
    }
    detector->setConfidenceThreshold(settings.confidenceThreshold);
    detector->setMinFaceSize(settings.minFaceSize);

    // Gizleme stratejisini oluştur
    std::unique_ptr<IHidingStrategy> strategy;
    switch (settings.hidingMethod) {
    case HidingMethod::GaussianBlur:
        strategy = std::make_unique<GaussianBlurStrategy>(settings.blurIntensity);
        break;
    case HidingMethod::Pixelate:
        strategy = std::make_unique<PixelateStrategy>(settings.pixelSize);
        break;
    case HidingMethod::BlackBar:
        strategy = std::make_unique<BlackBarStrategy>();
        break;
    case HidingMethod::Emoji:
        strategy = std::make_unique<EmojiOverlayStrategy>(settings.emojiPath);
        break;
    }

    IHidingStrategy* rawStrategy = strategy.get();

    // FaceHider'ı yapılandır
    FaceHider hider;
    hider.setDetector(std::move(detector));
    hider.setStrategy(std::move(strategy));

    // Çıktı dizinini oluştur
    QString outputDir = settings.outputDirectory;
    if (outputDir.isEmpty()) {
        QFileInfo fi(files.first());
        outputDir = fi.absolutePath() + "/output";
    }
    QDir().mkpath(outputDir);
    result.outputDirectory = outputDir;

    // Dosyaları sırasıyla işle
    for (int i = 0; i < files.size(); ++i) {
        if (m_cancelled) {
            qDebug() << "İşlem kullanıcı tarafından iptal edildi.";
            break;
        }

        const QString& filePath = files[i];
        emit progressUpdated(i + 1, files.size(), QFileInfo(filePath).fileName());

        FileResult fileResult = processFile(filePath, settings, hider, rawStrategy);
        result.fileResults.append(fileResult);

        if (fileResult.success) {
            result.successCount++;
            result.totalFacesHidden += fileResult.facesFound;
        } else if (fileResult.facesFound == 0 && fileResult.success) {
            result.noFaceCount++;
        } else {
            result.errorCount++;
        }

        emit fileProcessed(fileResult);
    }

    result.elapsedSeconds = timer.elapsed() / 1000.0;
    m_running = false;

    emit batchCompleted(result);
}

void BatchProcessor::cancel()
{
    m_cancelled = true;
    qDebug() << "İşlem iptal isteği gönderildi.";
}

FileResult BatchProcessor::processFile(const QString& filePath,
                                       const ProcessSettings& settings,
                                       FaceHider& hider,
                                       IHidingStrategy* rawStrategy)
{
    FileResult result;
    result.filePath = filePath;

    // Dosyayı oku
    cv::Mat image = cv::imread(filePath.toStdString(), cv::IMREAD_COLOR);
    if (image.empty()) {
        result.success = false;
        result.errorMessage = tr("Dosya okunamadı: %1").arg(filePath);
        emit errorOccurred(filePath, result.errorMessage);
        return result;
    }

    int facesFound = 0;

    // Eğer bu dosya için kullanıcının elle yaptığı yüz seçimleri/muafiyetleri varsa onu kullan
    if (m_manualOverrides.contains(filePath) && rawStrategy) {
        const auto& overrideList = m_manualOverrides[filePath];
        for (const auto& item : overrideList) {
            if (item.enabled) {
                rawStrategy->apply(image, item.toCvRect());
                facesFound++;
            }
        }
    } else {
        // Otomatik algıla ve gizle
        facesFound = hider.processImage(image);
    }

    result.facesFound = facesFound;

    // Yüz bulunamazsa ve ayar "atla" ise
    if (facesFound == 0 && settings.skipNoFace) {
        result.success = true;
        result.outputPath = "";
        return result;
    }

    // Çıktı dosya yolunu oluştur
    result.outputPath = buildOutputPath(filePath, settings);

    // MEB boyutlandırma ve format standartlarına göre kaydet
    bool saved = ImageUtils::saveImageWithSettings(result.outputPath, image, settings);

    if (!saved) {
        result.success = false;
        result.errorMessage = tr("Dosya kaydedilemedi: %1").arg(result.outputPath);
        emit errorOccurred(filePath, result.errorMessage);
    } else {
        result.success = true;
    }

    return result;
}

QString BatchProcessor::buildOutputPath(const QString& inputPath,
                                        const ProcessSettings& settings)
{
    QFileInfo fi(inputPath);
    QString outputDir = settings.outputDirectory;
    if (outputDir.isEmpty()) {
        outputDir = fi.absolutePath() + "/output";
    }

    QString extension;
    if (settings.resizeMode == ResizeMode::EOkul) {
        extension = ".jpg"; // e-Okul mutlaka JPEG olmalıdır
    } else {
        switch (settings.outputFormat) {
        case OutputFormat::SameAsSource:
            extension = "." + fi.suffix().toLower();
            break;
        case OutputFormat::JPEG:
            extension = ".jpg";
            break;
        case OutputFormat::PNG:
            extension = ".png";
            break;
        case OutputFormat::WEBP:
            extension = ".webp";
            break;
        case OutputFormat::BMP:
            extension = ".bmp";
            break;
        }
    }

    QString outputFileName = fi.completeBaseName() + settings.filenameSuffix + extension;
    return QDir(outputDir).filePath(outputFileName);
}
