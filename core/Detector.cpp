#include "Detector.hpp"
#include "../include/coin_detector.hpp"
#include <opencv2/imgproc.hpp>

std::vector<Detection> Detector::run(const cv::Mat& image) {
    std::vector<Detection> out;
    if (image.empty())
        return out;

    cv::Mat gray;
    if (image.channels() == 3)
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    else
        gray = image;

    CoinDetector detector;              // использует Params из coin_detector.hpp
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
        d.confidence = c.score;         // score из CoinDetector
        out.push_back(d);
    }
    return out;
}
