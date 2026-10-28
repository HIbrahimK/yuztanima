#include "GaussianBlurStrategy.h"
#include <opencv2/imgproc.hpp>
#include <algorithm>

GaussianBlurStrategy::GaussianBlurStrategy(int kernelSize)
{
    setKernelSize(kernelSize);
}

void GaussianBlurStrategy::apply(cv::Mat& image, const cv::Rect& faceRect)
{
    if (image.empty()) return;

    // Görüntü sınırları içinde kalan ROI
    cv::Rect safeRect = faceRect & cv::Rect(0, 0, image.cols, image.rows);
    if (safeRect.empty()) return;

    // Yüz bölgesine Gaussian blur uygula
    cv::Mat faceROI = image(safeRect);
    cv::GaussianBlur(faceROI, faceROI, cv::Size(m_kernelSize, m_kernelSize), 0);
}

QString GaussianBlurStrategy::name() const
{
    return QStringLiteral("Gaussian Blur");
}

QString GaussianBlurStrategy::description() const
{
    return QStringLiteral("Yüz bölgesini Gaussian bulanıklaştırma ile gizler");
}

void GaussianBlurStrategy::setKernelSize(int size)
{
    // Kernel boyutu tek sayı olmalı
    m_kernelSize = std::clamp(size, 15, 99);
    if (m_kernelSize % 2 == 0) {
        m_kernelSize++;
    }
}
