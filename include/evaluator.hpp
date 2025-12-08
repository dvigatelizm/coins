#pragma once
#include "coin_detector.hpp"
#include <vector>
#include <string>

struct GTCircle {
    cv::Point2f center;
    float radius;
};

struct EvalResult {
    int TP = 0;
    int FP = 0;
    int FN = 0;
    double precision() const { return TP + FP ? double(TP) / double(TP + FP) : 0.0; }
    double recall() const { return TP + FN ? double(TP) / double(TP + FN) : 0.0; }
    double f1() const {
        double p = precision(), r = recall();
        return (p + r) ? 2.0 * p * r / (p + r) : 0.0;
    }
};

class Evaluator {
public:
    // match_tol: maximum center distance (in pixels) to be considered a match
    // radius_tol: relative tolerance (fraction) for radius difference (e.g. 0.3 => 30%)
    Evaluator(float match_tol = 20.0f, float radius_tol = 0.4f);
    EvalResult evaluate(const std::vector<DetectedCircle>& dets, const std::vector<GTCircle>& gts) const;

private:
    float match_tol_;
    float radius_tol_;
};
