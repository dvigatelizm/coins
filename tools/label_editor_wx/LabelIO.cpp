#include "LabelIO.hpp"
#include <fstream>

std::vector<Circle> LoadLabels(const std::string& path) {
    std::vector<Circle> out;
    std::ifstream in(path);
    if (!in.is_open()) return out;

    double cx, cy, r;
    while (in >> cx >> cy >> r) {
        out.push_back(Circle{ cx, cy, r });
    }
    return out;
}

bool SaveLabels(const std::string& path, const std::vector<Circle>& circles) {
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) return false;
    for (const auto& c : circles) {
        out << c.cx << " " << c.cy << " " << c.r << "\n";
    }
    return true;
}