#include "Detector.hpp"
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

static void usage() {
    std::cout
        << "Usage:\n"
        << "  coin_detect_cli --image <path> [--out <labels.txt>]\n"
        << "\n"
        << "Output format (stdout and --out): cx cy r\n";
}

int main(int argc, char** argv) {
    std::string imagePath;
    std::string outPath;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--image" && i + 1 < argc) {
            imagePath = argv[++i];
        }
        else if (a == "--out" && i + 1 < argc) {
            outPath = argv[++i];
        }
        else if (a == "--help" || a == "-h") {
            usage();
            return 0;
        }
        else {
            std::cerr << "Unknown arg: " << a << "\n";
            usage();
            return 2;
        }
    }

    if (imagePath.empty()) {
        std::cerr << "Error: --image is required\n";
        usage();
        return 2;
    }

    cv::Mat img = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Error: failed to read image: " << imagePath << "\n";
        return 3;
    }

    Detector det;
    auto detections = det.run(img);

    // Detection(bbox) -> Circle(cx,cy,r)
    struct CircleOut { double cx, cy, r; };
    std::vector<CircleOut> circles;
    circles.reserve(detections.size());

    for (const auto& d : detections) {
        double cx = d.bbox.x + d.bbox.width * 0.5;
        double cy = d.bbox.y + d.bbox.height * 0.5;
        double r = 0.5 * std::min(d.bbox.width, d.bbox.height);
        circles.push_back({ cx, cy, r });
    }

    auto dump = [&](std::ostream& os) {
        for (const auto& c : circles) {
            os << c.cx << " " << c.cy << " " << c.r << "\n";
        }
        };

    // stdout
    dump(std::cout);

    // file
    if (!outPath.empty()) {
        std::ofstream f(outPath);
        if (!f) {
            std::cerr << "Error: can't open out file: " << outPath << "\n";
            return 4;
        }
        dump(f);
    }

    return 0;
}
