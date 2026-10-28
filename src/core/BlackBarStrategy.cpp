#include "BlackBarStrategy.h"
#include <opencv2/imgproc.hpp>

void BlackBarStrategy::apply(cv::Mat& image, const cv::Rect& faceRect)
{
    if (image.empty()) return;

    cv::Rect safeRect = faceRect & cv::Rect(0, 0, image.cols, image.rows);
    if (safeRect.empty()) return;

    // Göz bölgesi: yüzün üst %25'inden başla, yüksekliğin %25'i kadar
    int bandY = safeRect.y + static_cast<int>(safeRect.height * 0.22);
    int bandH = static_cast<int>(safeRect.height * 0.28);

    // Siyah bandı biraz genişlet (yüz genişliğinin %10'u kadar her yöne)
    int padding = static_cast<int>(safeRect.width * 0.1);
    int bandX = std::max(0, safeRect.x - padding);
    int bandW = std::min(image.cols - bandX, safeRect.width + 2 * padding);

    cv::Rect bandRect(bandX, bandY, bandW, bandH);
    bandRect = bandRect & cv::Rect(0, 0, image.cols, image.rows);

    if (!bandRect.empty()) {
        cv::rectangle(image, bandRect, cv::Scalar(0, 0, 0), cv::FILLED);
    }
}

QString BlackBarStrategy::name() const
{
    return QStringLiteral("Siyah Bant");
}

QString BlackBarStrategy::description() const
{
    return QStringLiteral("Göz hizasında siyah bant ile yüzü gizler");
}
