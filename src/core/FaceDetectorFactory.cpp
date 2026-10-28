#include "FaceDetectorFactory.h"
#include "YuNetFaceDetector.h"
#include "HaarFaceDetector.h"
#include <QDir>
#include <QDebug>

std::unique_ptr<IFaceDetector> FaceDetectorFactory::create(
    DetectorType type,
    const QString& modelsDir)
{
    QString modelPath;

    switch (type) {
    case DetectorType::YuNet:
        modelPath = QDir(modelsDir).filePath(YUNET_MODEL_FILE);
        qDebug() << "YuNet algılayıcı oluşturuluyor:" << modelPath;
        return std::make_unique<YuNetFaceDetector>(modelPath);

    case DetectorType::HaarCascade:
        modelPath = QDir(modelsDir).filePath(HAAR_CASCADE_FILE);
        qDebug() << "Haar Cascade algılayıcı oluşturuluyor:" << modelPath;
        return std::make_unique<HaarFaceDetector>(modelPath);
    }

    // Varsayılan: YuNet
    qWarning() << "Bilinmeyen algılayıcı tipi, YuNet kullanılıyor.";
    modelPath = QDir(modelsDir).filePath(YUNET_MODEL_FILE);
    return std::make_unique<YuNetFaceDetector>(modelPath);
}

bool FaceDetectorFactory::isModelAvailable(DetectorType type, const QString& modelsDir)
{
    QString modelFile;

    switch (type) {
    case DetectorType::YuNet:
        modelFile = YUNET_MODEL_FILE;
        break;
    case DetectorType::HaarCascade:
        modelFile = HAAR_CASCADE_FILE;
        break;
    }

    QString fullPath = QDir(modelsDir).filePath(modelFile);
    return QFileInfo::exists(fullPath);
}
