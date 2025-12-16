#include "Detector.hpp"
#include "../include/coin_detector.hpp"

Detector::Detector() = default;

std::vector<Detection> Detector::run(const cv::Mat& image) {
    std::vector<Detection> out;
    if (image.empty())
        return out;

    cv::Mat gray;
    if (image.channels() == 3)
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    else
        gray = image;

    CoinDetector detector;
    auto circles = detector.detect(gray);

    for (const auto& c : circles) {
        Detection d;
        d.bbox = cv::Rect(
            int(c.center.x - c.radius),
            int(c.center.y - c.radius),
            int(2 * c.radius),
            int(2 * c.radius)
        );
        d.class_id = 0;
        d.confidence = c.score;
        out.push_back(d);
    }

    return out;
}
