#include "FaceHider.h"
#include <QDebug>

FaceHider::FaceHider()
    : m_detector(nullptr)
    , m_strategy(nullptr)
{
}

void FaceHider::setDetector(std::unique_ptr<IFaceDetector> detector)
{
    m_detector = std::move(detector);
}

void FaceHider::setStrategy(std::unique_ptr<IHidingStrategy> strategy)
{
    m_strategy = std::move(strategy);
}

int FaceHider::processImage(cv::Mat& image)
{
    if (!isReady() || image.empty()) {
        return 0;
    }

    // Yüzleri algıla
    auto faces = m_detector->detect(image);

    if (faces.empty()) {
        qDebug() << "Görüntüde yüz bulunamadı.";
        return 0;
    }

    qDebug() << faces.size() << "yüz algılandı, gizleniyor...";

    // Her yüze gizleme stratejisini uygula
    for (const auto& faceRect : faces) {
        m_strategy->apply(image, faceRect);
    }

    return static_cast<int>(faces.size());
}

std::vector<cv::Rect> FaceHider::detectFaces(const cv::Mat& image)
{
    if (!m_detector || image.empty()) {
        return {};
    }
    return m_detector->detect(image);
}

bool FaceHider::isReady() const
{
    return m_detector && m_detector->isLoaded() && m_strategy;
}

QString FaceHider::detectorName() const
{
    return m_detector ? m_detector->name() : QStringLiteral("(yok)");
}

QString FaceHider::strategyName() const
{
    return m_strategy ? m_strategy->name() : QStringLiteral("(yok)");
}
