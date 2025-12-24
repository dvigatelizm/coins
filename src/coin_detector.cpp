#include "coin_detector.hpp"
#include <opencv2/imgproc.hpp>

CoinDetector::CoinDetector(const Params& p) : params_(p) {}

CoinDetector::CoinDetector()
    : CoinDetector(Params{}) {}

std::vector<DetectedCircle> CoinDetector::detect(const cv::Mat& imgGray) const {
    CV_Assert(imgGray.channels() == 1);

    cv::Mat blurred;
    int k = params_.gaussKernel | 1; // ensure odd
    if (k < 3) k = 3;
    cv::GaussianBlur(imgGray, blurred, cv::Size(k, k), params_.gaussSigma);

    // Canny - for internal Hough param1, also helps visualize
    cv::Mat edges;
    cv::Canny(blurred, edges, params_.cannyLow, params_.cannyHigh);

    // HoughCircles requires 8-bit image; use blurred or edges (prefer blurred)
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(blurred, circles, cv::HOUGH_GRADIENT,
        params_.houghDp,
        params_.houghMinDist,
        params_.houghParam1,
        params_.houghParam2,
        params_.minRadius,
        params_.maxRadius);

    std::vector<DetectedCircle> out;
    out.reserve(circles.size());
    for (auto& c : circles) {
        DetectedCircle d;
        d.center = cv::Point2f(c[0], c[1]);
        d.radius = c[2];
        d.score = 1.0f; // OpenCV Hough doesn't return score; placeholder
        out.push_back(d);
    }

    // Optional: filter overlapping duplicates (simple NMS by center distance)
    std::vector<bool> keep(out.size(), true);
    for (size_t i = 0; i < out.size(); ++i) {
        if (!keep[i]) continue;
        for (size_t j = i + 1; j < out.size(); ++j) {
            if (!keep[j]) continue;
            float dx = out[i].center.x - out[j].center.x;
            float dy = out[i].center.y - out[j].center.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < std::min(out[i].radius, out[j].radius) * 0.5f) {
                // keep larger radius
                if (out[i].radius >= out[j].radius) keep[j] = false;
                else keep[i] = false;
            }
        }
    }
    std::vector<DetectedCircle> filtered;
    for (size_t i = 0; i < out.size(); ++i) if (keep[i]) filtered.push_back(out[i]);
    return filtered;
}
