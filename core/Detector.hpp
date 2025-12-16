#pragma once
#include <opencv2/core.hpp>
#include <vector>


struct Detection {
    cv::Rect bbox;
    int class_id;
    float confidence;
};

class Detector {
public:
    Detector();
    std::vector<Detection> run(const cv::Mat& image);
};
