#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

struct DetectedCircle {
    cv::Point2f center;
    float radius;
    float score; // accumulator value or confidence
};

class CoinDetector {
public:
    struct Params {
        int gaussKernel = 9;
        double gaussSigma = 2.0;
        int cannyLow = 100;
        int cannyHigh = 200;
        int houghDp = 1.1;
        int houghMinDist = 47;
        int houghParam1 = 200; // Canny high threshold (internal)
        int houghParam2 = 32;  // accumulator threshold
        int minRadius = 10;
        int maxRadius = 200;
    };

    CoinDetector(const Params& p = Params());
    std::vector<DetectedCircle> detect(const cv::Mat& imgGray) const;

private:
    Params params_;
};
