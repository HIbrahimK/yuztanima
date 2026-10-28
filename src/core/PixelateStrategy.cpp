#include "PixelateStrategy.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>

PixelateStrategy::PixelateStrategy(int pixelSize)
{
    setPixelSize(pixelSize);
}

void PixelateStrategy::apply(cv::Mat& image, const cv::Rect& faceRect)
{
    if (image.empty()) return;

    cv::Rect safeRect = faceRect & cv::Rect(0, 0, image.cols, image.rows);
    if (safeRect.empty()) return;

    cv::Mat faceROI = image(safeRect);

    // Mozaik efekti: küçült → büyüt (nearest-neighbor interpolation)
    int smallW = std::max(1, faceROI.cols / m_pixelSize);
    int smallH = std::max(1, faceROI.rows / m_pixelSize);

    cv::Mat small;
    cv::resize(faceROI, small, cv::Size(smallW, smallH), 0, 0, cv::INTER_LINEAR);
    cv::resize(small, faceROI, faceROI.size(), 0, 0, cv::INTER_NEAREST);
}

QString PixelateStrategy::name() const
{
    return QStringLiteral("Pikselleştirme");
}

QString PixelateStrategy::description() const
{
    return QStringLiteral("Yüz bölgesini mozaik efekti ile gizler");
}

void PixelateStrategy::setPixelSize(int size)
{
    m_pixelSize = std::clamp(size, 5, 30);
}
