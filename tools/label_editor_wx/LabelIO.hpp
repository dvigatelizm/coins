#pragma once
#include <vector>
#include <string>

struct Circle {
    double cx{};
    double cy{};
    double r{};
    double confidence = -1.0;
};

std::vector<Circle> LoadLabels(const std::string& path);
bool SaveLabels(const std::string& path, const std::vector<Circle>& circles);