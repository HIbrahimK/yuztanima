#include <QTest>
#include <QSignalSpy>
#include "GaussianBlurStrategy.h"
#include "PixelateStrategy.h"
#include "BlackBarStrategy.h"
#include "EmojiOverlayStrategy.h"
#include "FaceDetectorFactory.h"
#include "ProcessSettings.h"
#include "BatchResult.h"
#include "BatchProcessor.h"
#include "ImageUtils.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

class TestCore : public QObject {
    Q_OBJECT

private slots:
    void testGaussianBlurStrategy() {
        cv::Mat img = cv::Mat::ones(100, 100, CV_8UC3) * 128;
        cv::line(img, cv::Point(20, 20), cv::Point(80, 80), cv::Scalar(0, 0, 0), 5);
        GaussianBlurStrategy strategy(15);
        strategy.apply(img, cv::Rect(10, 10, 80, 80));
        QVERIFY(strategy.name().contains("Blur"));
    }

    void testPixelateStrategy() {
        cv::Mat img = cv::Mat::zeros(100, 100, CV_8UC3);
        PixelateStrategy strategy(10);
        strategy.apply(img, cv::Rect(10, 10, 80, 80));
        QCOMPARE(strategy.pixelSize(), 10);
    }

    void testBlackBarStrategy() {
        cv::Mat img = cv::Mat::ones(100, 100, CV_8UC3) * 255;
        BlackBarStrategy strategy;
        strategy.apply(img, cv::Rect(20, 20, 60, 60));
        cv::Vec3b pixel = img.at<cv::Vec3b>(35, 50);
        QCOMPARE(pixel[0], 0);
        QCOMPARE(pixel[1], 0);
        QCOMPARE(pixel[2], 0);
    }

    void testEmojiOverlayStrategy() {
        cv::Mat img = cv::Mat::zeros(100, 100, CV_8UC3);
        EmojiOverlayStrategy strategy("resources/emojis/smile.png");
        QVERIFY(strategy.isLoaded());
        strategy.apply(img, cv::Rect(20, 20, 60, 60));
        bool altered = false;
        for (int r = 30; r < 70; ++r) {
            for (int c = 30; c < 70; ++c) {
                if (img.at<cv::Vec3b>(r, c) != cv::Vec3b(0, 0, 0)) {
                    altered = true;
                    break;
                }
            }
            if (altered) break;
        }
        QVERIFY(altered);
    }

    void testBatchResultStats() {
        BatchResult res;
        res.totalFiles = 10;
        res.successCount = 8;
        res.errorCount = 2;
        QCOMPARE(res.successRate(), 80.0);
    }

    void testImageUtilsConversion() {
        cv::Mat mat(50, 50, CV_8UC3, cv::Scalar(255, 0, 0)); // BGR Blue
        QImage qimg = ImageUtils::matToQImage(mat);
        QCOMPARE(qimg.width(), 50);
        QCOMPARE(qimg.height(), 50);

        cv::Mat converted = ImageUtils::qImageToMat(qimg);
        QCOMPARE(converted.cols, 50);
        QCOMPARE(converted.rows, 50);
    }

    void testFaceDetectionYuNet() {
        QString modelDir = QStringLiteral("resources/models");
        auto detector = FaceDetectorFactory::create(DetectorType::YuNet, modelDir);
        QVERIFY(detector != nullptr);
        QVERIFY(detector->isLoaded());

        cv::Mat sample = cv::imread("tests/test_data/sample_face.jpg");
        QVERIFY(!sample.empty());

        auto faces = detector->detect(sample);
        QVERIFY(faces.size() >= 1);
        qDebug() << "YuNet algılanan yüz sayısı:" << faces.size();
    }

    void testBatchProcessorRun() {
        BatchProcessor processor;
        QSignalSpy spyCompleted(&processor, &BatchProcessor::batchCompleted);

        ProcessSettings settings;
        settings.detectorType = DetectorType::YuNet;
        settings.hidingMethod = HidingMethod::GaussianBlur;
        settings.outputDirectory = QStringLiteral("build/test_output");

        QStringList files = {QStringLiteral("tests/test_data/sample_face.jpg")};
        processor.processBatch(files, settings, QStringLiteral("resources/models"));

        QCOMPARE(spyCompleted.count(), 1);
        auto result = qvariant_cast<BatchResult>(spyCompleted.at(0).at(0));
        QCOMPARE(result.totalFiles, 1);
        QCOMPARE(result.successCount, 1);
        QVERIFY(result.totalFacesHidden >= 1);
    }

    void testBatchProcessorRunWithEmoji() {
        BatchProcessor processor;
        QSignalSpy spyCompleted(&processor, &BatchProcessor::batchCompleted);

        ProcessSettings settings;
        settings.detectorType = DetectorType::YuNet;
        settings.hidingMethod = HidingMethod::Emoji;
        settings.emojiPath = QStringLiteral(":/emojis/smile.png");
        settings.outputDirectory = QStringLiteral("build/test_emoji_output");

        QStringList files = {QStringLiteral("tests/test_data/sample_face.jpg")};
        processor.processBatch(files, settings, QStringLiteral("resources/models"));

        QCOMPARE(spyCompleted.count(), 1);
        auto result = qvariant_cast<BatchResult>(spyCompleted.at(0).at(0));
        QCOMPARE(result.successCount, 1);
        QVERIFY(result.totalFacesHidden >= 1);
    }

    void testMebMansetPreset() {
        cv::Mat sample = cv::imread("tests/test_data/sample_face.jpg");
        QVERIFY(!sample.empty());

        cv::Mat manset = ImageUtils::applyResizePreset(sample, ResizeMode::MebManset);
        QCOMPARE(manset.cols, 1200);
        QCOMPARE(manset.rows, 568);
    }

    void testEOkulPresetAndSize() {
        cv::Mat sample = cv::imread("tests/test_data/sample_face.jpg");
        QVERIFY(!sample.empty());

        ProcessSettings settings;
        settings.resizeMode = ResizeMode::EOkul;
        QString outputPath = QStringLiteral("build/test_eokul_sample.jpg");

        bool saved = ImageUtils::saveImageWithSettings(outputPath, sample, settings);
        QVERIFY(saved);

        QFileInfo fi(outputPath);
        QVERIFY(fi.exists());
        qint64 sizeKb = fi.size() / 1024;
        qDebug() << "e-Okul çıktı dosya boyutu:" << sizeKb << "KB";
        QVERIFY(sizeKb <= 20); // MEB kuralı: 20 KB'dan küçük
    }

    void testMebbisPreset() {
        cv::Mat sample = cv::imread("tests/test_data/sample_face.jpg");
        QVERIFY(!sample.empty());

        cv::Mat mebbis = ImageUtils::applyResizePreset(sample, ResizeMode::Mebbis);
        QCOMPARE(mebbis.cols, 394);
        QCOMPARE(mebbis.rows, 512);
    }

    void testInstagramPresets() {
        cv::Mat sample = cv::imread("tests/test_data/sample_face.jpg");
        QVERIFY(!sample.empty());

        cv::Mat sq = ImageUtils::applyResizePreset(sample, ResizeMode::InstagramSquare);
        QCOMPARE(sq.cols, 1080);
        QCOMPARE(sq.rows, 1080);

        cv::Mat port = ImageUtils::applyResizePreset(sample, ResizeMode::InstagramPortrait);
        QCOMPARE(port.cols, 1080);
        QCOMPARE(port.rows, 1350);

        cv::Mat story = ImageUtils::applyResizePreset(sample, ResizeMode::InstagramStory);
        QCOMPARE(story.cols, 1080);
        QCOMPARE(story.rows, 1920);
    }

    void testWatermarkTextAndLogo() {
        cv::Mat sample = cv::imread("tests/test_data/sample_face.jpg");
        QVERIFY(!sample.empty());

        ProcessSettings settings;
        settings.watermarkType = WatermarkType::Text;
        settings.watermarkText = QStringLiteral("T.C. Millî Eğitim Bakanlığı");
        settings.watermarkPosition = WatermarkPosition::BottomRight;
        settings.watermarkOpacity = 80;

        cv::Mat withText = ImageUtils::applyWatermark(sample, settings);
        QCOMPARE(withText.cols, sample.cols);
        QCOMPARE(withText.rows, sample.rows);

        // Logo + Metin testi
        settings.watermarkType = WatermarkType::Both;
        settings.watermarkImagePath = QStringLiteral("resources/icons/app_icon.png");
        cv::Mat withBoth = ImageUtils::applyWatermark(sample, settings);
        QCOMPARE(withBoth.cols, sample.cols);
        QCOMPARE(withBoth.rows, sample.rows);
    }

    void testBatchProcessorWithManualOverrides() {
        BatchProcessor processor;
        QSignalSpy spyCompleted(&processor, &BatchProcessor::batchCompleted);

        ProcessSettings settings;
        settings.detectorType = DetectorType::YuNet;
        settings.hidingMethod = HidingMethod::GaussianBlur;
        settings.outputDirectory = QStringLiteral("build/test_override_output");

        QString filePath = QStringLiteral("tests/test_data/sample_face.jpg");

        // Biri öğrenci (enabled), biri öğretmen (muaf / enabled=false)
        QList<FaceItem> overrides;
        overrides.append(FaceItem::fromCvRect(cv::Rect(100, 100, 50, 50), true, true, "Öğrenci"));
        overrides.append(FaceItem::fromCvRect(cv::Rect(200, 200, 50, 50), false, false, "Öğretmen"));

        QMap<QString, QList<FaceItem>> mapOverrides;
        mapOverrides[filePath] = overrides;

        processor.setManualOverrides(mapOverrides);
        processor.processBatch({filePath}, settings, QStringLiteral("resources/models"));

        QCOMPARE(spyCompleted.count(), 1);
        auto result = qvariant_cast<BatchResult>(spyCompleted.at(0).at(0));
        QCOMPARE(result.successCount, 1);
        QCOMPARE(result.totalFacesHidden, 1); // Yalnızca 1 yüz gizlenmeli
    }

    void testMaxFileSizeConstraintQualityOnly() {
        cv::Mat sample = cv::imread("tests/test_data/sample_face.jpg");
        QVERIFY(!sample.empty());

        ProcessSettings settings;
        settings.enableMaxFileSize = true;
        settings.maxFileSizeKb = 100;
        settings.maxFileSizeAction = MaxFileSizeAction::CompressQualityOnly;
        QString outPath = QStringLiteral("build/test_max_100kb.jpg");

        bool saved = ImageUtils::saveImageWithSettings(outPath, sample, settings);
        QVERIFY(saved);

        qint64 sizeKb = ImageUtils::getFileSize(outPath) / 1024;
        qDebug() << "100 KB hedefli test çıktısı:" << sizeKb << "KB";
        QVERIFY(sizeKb <= 100);
    }

    void testMaxFileSizeConstraintCompressAndResize() {
        cv::Mat sample = cv::imread("tests/test_data/sample_face.jpg");
        QVERIFY(!sample.empty());

        ProcessSettings settings;
        settings.enableMaxFileSize = true;
        settings.maxFileSizeKb = 50;
        settings.maxFileSizeAction = MaxFileSizeAction::CompressAndResize;
        QString outPath = QStringLiteral("build/test_max_50kb.jpg");

        bool saved = ImageUtils::saveImageWithSettings(outPath, sample, settings);
        QVERIFY(saved);

        qint64 sizeKb = ImageUtils::getFileSize(outPath) / 1024;
        qDebug() << "50 KB hedefli yeniden boyutlandırmalı test çıktısı:" << sizeKb << "KB";
        QVERIFY(sizeKb <= 50);
    }
};

QTEST_MAIN(TestCore)
#include "test_core.moc"
