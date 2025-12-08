#include "evaluator.hpp"
#include <cmath>

Evaluator::Evaluator(float match_tol, float radius_tol) : match_tol_(match_tol), radius_tol_(radius_tol) {}

EvalResult Evaluator::evaluate(const std::vector<DetectedCircle>& dets, const std::vector<GTCircle>& gts) const {
    EvalResult res;
    std::vector<bool> gt_used(gts.size(), false);
    // For each detection, find best matching GT
    for (const auto& d : dets) {
        int best_idx = -1;
        float best_dist = 1e9;
        for (size_t i = 0; i < gts.size(); ++i) {
            if (gt_used[i]) continue;
            float dx = d.center.x - gts[i].center.x;
            float dy = d.center.y - gts[i].center.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            float radius_diff = std::abs(d.radius - gts[i].radius) / gts[i].radius;
            if (dist <= match_tol_ && radius_diff <= radius_tol_) {
                if (dist < best_dist) { best_dist = dist; best_idx = int(i); }
            }
        }
        if (best_idx >= 0) {
            res.TP++;
            gt_used[best_idx] = true;
        }
        else {
            res.FP++;
        }
    }
    // remaining unmatched GT are FN
    for (size_t i = 0; i < gts.size(); ++i) if (!gt_used[i]) res.FN++;
    return res;
}
