#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include <filesystem>

#include <opencv2/opencv.hpp>
#include "coin_detector.hpp"
#include "evaluator.hpp"

namespace fs = std::filesystem;

std::vector<GTCircle> read_gt_file(const std::string& path) {
    // Simple ground truth format: each line -> cx cy radius
    std::vector<GTCircle> out;
    std::ifstream in(path);
    if (!in.is_open()) return out;
    float cx, cy, r;
    while (in >> cx >> cy >> r) {
        GTCircle g; g.center = cv::Point2f(cx, cy); g.radius = r;
        out.push_back(g);
    }
    return out;
}

void draw_and_save(const cv::Mat& img, const std::vector<DetectedCircle>& dets, const std::string& outpath) {
    cv::Mat vis;
    if (img.channels() == 1) cv::cvtColor(img, vis, cv::COLOR_GRAY2BGR);
    else vis = img.clone();

    for (const auto& d : dets) {
        cv::circle(vis, d.center, (int)std::round(d.radius), cv::Scalar(0, 0, 255), 2);
        cv::circle(vis, d.center, 2, cv::Scalar(0, 255, 0), -1);
    }
    cv::imwrite(outpath, vis);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: coin_detector <image> [gt_file(optional)]\n";
        std::cout << "Or: coin_detector <input_folder> --batch\n";
        return 0;
    }

    std::string arg = argv[1];
    bool batch = false;
    if (argc >= 3 && std::string(argv[2]) == std::string("--batch")) batch = true;

    CoinDetector::Params params;
    // default params; optionally parse CLI args to override

    CoinDetector detector(params);
    Evaluator eval(25.0f, 0.5f);

    if (!batch) {
        cv::Mat img = cv::imread(arg, cv::IMREAD_COLOR);
        if (img.empty()) { std::cerr << "Cannot open " << arg << "\n"; return -1; }
        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

        auto t0 = std::chrono::high_resolution_clock::now();
        auto dets = detector.detect(gray);
        auto t1 = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(t1 - t0).count();

        std::cout << "Detected circles: " << dets.size() << "\n";
        for (size_t i = 0; i < dets.size(); ++i) {
            std::cout << i << ": cx=" << dets[i].center.x << " cy=" << dets[i].center.y << " r=" << dets[i].radius << "\n";
        }
        std::cout << "Detection time (ms): " << elapsed << "\n";

        if (argc >= 3) {
            std::string gtfile = argv[2];
            auto gts = read_gt_file(gtfile);
            auto res = eval.evaluate(dets, gts);
            std::cout << "TP=" << res.TP << " FP=" << res.FP << " FN=" << res.FN << "\n";
            std::cout << "Precision=" << res.precision() << " Recall=" << res.recall() << " F1=" << res.f1() << "\n";
        }

        // save visualization
        std::string outpath = fs::path(arg).stem().string() + "_detected.png";
        draw_and_save(img, dets, outpath);
        std::cout << "Saved visualization to " << outpath << "\n";
    }
    else {
        // batch mode: iterate files in folder
        fs::path folder = arg;
        if (!fs::is_directory(folder)) { std::cerr << "Not a directory: " << arg << "\n"; return -1; }
        std::vector<std::string> images;
        for (auto& p : fs::directory_iterator(folder)) {
            if (!p.is_regular_file()) continue;
            std::string ext = p.path().extension().string();
            if (ext == ".jpg" || ext == ".png" || ext == ".jpeg") images.push_back(p.path().string());
        }
        int totalTP = 0, totalFP = 0, totalFN = 0;
        int count = 0;
        for (auto& imgpath : images) {
            cv::Mat img = cv::imread(imgpath, cv::IMREAD_COLOR);
            if (img.empty()) continue;
            cv::Mat gray; cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            auto t0 = std::chrono::high_resolution_clock::now();
            auto dets = detector.detect(gray);
            auto t1 = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(t1 - t0).count();

            std::cout << "Image: " << imgpath << " dets=" << dets.size() << " time(ms)=" << elapsed << "\n";
            // look for a gt file with same stem and .txt
            fs::path gtpath = fs::path(imgpath).replace_extension(".txt");
            if (fs::exists(gtpath)) {
                auto gts = read_gt_file(gtpath.string());
                auto res = eval.evaluate(dets, gts);
                totalTP += res.TP; totalFP += res.FP; totalFN += res.FN; count++;
                std::cout << "  TP=" << res.TP << " FP=" << res.FP << " FN=" << res.FN << "\n";
            }
            std::string outpath = fs::path(imgpath).stem().string() + "_detected.png";
            draw_and_save(img, dets, outpath);
        }
        if (count > 0) {
            double precision = totalTP + totalFP ? double(totalTP) / double(totalTP + totalFP) : 0.0;
            double recall = totalTP + totalFN ? double(totalTP) / double(totalTP + totalFN) : 0.0;
            double f1 = (precision + recall) ? 2.0 * precision * recall / (precision + recall) : 0.0;
            std::cout << "Batch eval across " << count << " images: Precision=" << precision << " Recall=" << recall << " F1=" << f1 << "\n";
        }
    }

    return 0;
}
