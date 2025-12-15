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

// ============================================================
// Ground truth reader
// ============================================================
std::vector<GTCircle> read_gt_file(const std::string& path) {
    std::vector<GTCircle> out;
    std::ifstream in(path);
    if (!in.is_open()) return out;

    float cx, cy, r;
    while (in >> cx >> cy >> r) {
        GTCircle g;
        g.center = cv::Point2f(cx, cy);
        g.radius = r;
        out.push_back(g);
    }
    return out;
}

// ============================================================
// Visualization
// ============================================================
void draw_and_save(const cv::Mat& img,
    const std::vector<DetectedCircle>& dets,
    const std::string& outpath) {
    cv::Mat vis;
    if (img.channels() == 1)
        cv::cvtColor(img, vis, cv::COLOR_GRAY2BGR);
    else
        vis = img.clone();

    for (const auto& d : dets) {
        cv::circle(vis, d.center,
            static_cast<int>(std::round(d.radius)),
            cv::Scalar(0, 0, 255), 2);
        cv::circle(vis, d.center, 2,
            cv::Scalar(0, 255, 0), -1);
    }
    cv::imwrite(outpath, vis);
}

// ============================================================
// Save detections + evaluation (if available)
// ============================================================
void save_detections_txt(const std::vector<DetectedCircle>& dets,
    const std::string& imgpath,
    const EvalResult* res) {
    fs::path p(imgpath);
    fs::path txtPath =
        p.parent_path() / (p.stem().string() + "_detected.txt");

    std::ofstream out(txtPath.string());
    if (!out.is_open()) {
        std::cerr << "Cannot open file for writing: "
            << txtPath << "\n";
        return;
    }

    out << "Detected circles: " << dets.size() << "\n";
    for (size_t i = 0; i < dets.size(); ++i) {
        out << i << ": cx=" << dets[i].center.x
            << " cy=" << dets[i].center.y
            << " r=" << dets[i].radius << "\n";
    }

    if (res) {
        out << "\n=== Evaluation ===\n";
        out << "TP=" << res->TP << "\n";
        out << "FP=" << res->FP << "\n";
        out << "FN=" << res->FN << "\n";
        out << "Precision=" << res->precision() << "\n";
        out << "Recall=" << res->recall() << "\n";
        out << "F1=" << res->f1() << "\n";
    }

    out.close();
    std::cout << "Saved detections to " << txtPath << "\n";
}

// ============================================================
// Main
// ============================================================
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage:\n"
            << "  coin_detector <image> [gt_file]\n"
            << "  coin_detector <folder> --batch\n";
        return 0;
    }

    CoinDetector::Params params;
    CoinDetector detector(params);
    Evaluator eval(25.0f, 0.5f);

    std::string input = argv[1];
    bool batch = (argc >= 3 && std::string(argv[2]) == "--batch");

    // --------------------------------------------------------
    // Image processing function
    // --------------------------------------------------------
    auto process_image =
        [&](const fs::path& imgPath, const fs::path* gtPath) {

        cv::Mat img = cv::imread(imgPath.string(), cv::IMREAD_COLOR);
        if (img.empty()) {
            std::cerr << "Cannot open image: " << imgPath << "\n";
            return;
        }

        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

        auto t0 = std::chrono::high_resolution_clock::now();
        auto dets = detector.detect(gray);
        auto t1 = std::chrono::high_resolution_clock::now();

        double elapsed =
            std::chrono::duration<double, std::milli>(t1 - t0).count();

        std::cout << "\nImage: " << imgPath << "\n";
        std::cout << "Detected circles: " << dets.size() << "\n";
        for (size_t i = 0; i < dets.size(); ++i) {
            std::cout << i << ": cx=" << dets[i].center.x
                << " cy=" << dets[i].center.y
                << " r=" << dets[i].radius << "\n";
        }
        std::cout << "Detection time (ms): " << elapsed << "\n";

        EvalResult evalRes;
        bool hasEval = false;

        if (gtPath && fs::exists(*gtPath)) {
            auto gts = read_gt_file(gtPath->string());
            evalRes = eval.evaluate(dets, gts);
            hasEval = true;

            std::cout << "TP=" << evalRes.TP
                << " FP=" << evalRes.FP
                << " FN=" << evalRes.FN << "\n";
            std::cout << "Precision=" << evalRes.precision()
                << " Recall=" << evalRes.recall()
                << " F1=" << evalRes.f1() << "\n";
        }

        fs::path outImg =
            imgPath.parent_path() /
            (imgPath.stem().string() + "_detected.png");

        draw_and_save(img, dets, outImg.string());
        save_detections_txt(
            dets,
            imgPath.string(),
            hasEval ? &evalRes : nullptr
        );

        std::cout << "Saved visualization to "
            << outImg << "\n";
        };

    // --------------------------------------------------------
    // Single image mode
    // --------------------------------------------------------
    if (!batch) {
        fs::path imgPath = input;
        fs::path gtPath;
        fs::path* gtPtr = nullptr;

        if (argc >= 3) {
            gtPath = argv[2];
            gtPtr = &gtPath;
        }

        process_image(imgPath, gtPtr);
    }
    // --------------------------------------------------------
    // Batch mode
    // --------------------------------------------------------
    else {
        fs::path folder = input;
        if (!fs::is_directory(folder)) {
            std::cerr << "Not a directory: " << folder << "\n";
            return -1;
        }

        int totalTP = 0, totalFP = 0, totalFN = 0, count = 0;

        for (const auto& p : fs::directory_iterator(folder)) {
            if (!p.is_regular_file())
                continue;

            std::string ext = p.path().extension().string();
            if (ext != ".jpg" && ext != ".png" && ext != ".jpeg")
                continue;

            // Skip already generated result images (_detected)
            std::string stem = p.path().stem().string();
            if (stem.size() >= 9 &&
                stem.compare(stem.size() - 9, 9, "_detected") == 0)
                continue;

            fs::path gtPath = p.path();
            gtPath.replace_extension(".txt");

            process_image(
                p.path(),
                fs::exists(gtPath) ? &gtPath : nullptr
            );

            if (fs::exists(gtPath)) {
                cv::Mat img = cv::imread(p.path().string(),
                    cv::IMREAD_GRAYSCALE);
                auto dets = detector.detect(img);
                auto gts = read_gt_file(gtPath.string());
                auto res = eval.evaluate(dets, gts);

                totalTP += res.TP;
                totalFP += res.FP;
                totalFN += res.FN;
                count++;
            }
        }

        if (count > 0) {
            double precision =
                (totalTP + totalFP)
                ? double(totalTP) / (totalTP + totalFP)
                : 0.0;
            double recall =
                (totalTP + totalFN)
                ? double(totalTP) / (totalTP + totalFN)
                : 0.0;
            double f1 =
                (precision + recall)
                ? 2.0 * precision * recall / (precision + recall)
                : 0.0;

            std::cout << "\nBatch evaluation ("
                << count << " images):\n";
            std::cout << "Precision=" << precision
                << " Recall=" << recall
                << " F1=" << f1 << "\n";
        }
    }

    return 0;
}