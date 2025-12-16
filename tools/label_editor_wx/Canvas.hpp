#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <vector>
#include <string>
#include "LabelIO.hpp"
#include <opencv2/core.hpp>
#include "Detector.hpp"

class Canvas : public wxPanel {
public:
    explicit Canvas(wxWindow* parent);

    bool LoadImage(const std::string& imgPath);
    bool LoadOrCreateLabelsForImage();
    bool SaveLabels();

    void SetLabelsPath(const std::string& path) { labelsPath_ = path; }
    const std::string& GetImagePath() const { return imagePath_; }
    const std::string& GetLabelsPath() const { return labelsPath_; }

    cv::Mat GetImageMat() const;
    const std::vector<Circle>& GetGroundTruthCircles() const;
    void SetDetectedCircles(const std::vector<Circle>& circles);

private:
    void OnPaint(wxPaintEvent& evt);
    void OnLeftDown(wxMouseEvent& evt);
    void OnLeftUp(wxMouseEvent& evt);
    void OnMotion(wxMouseEvent& evt);
    void OnRightDown(wxMouseEvent& evt);
    void OnKeyDown(wxKeyEvent& evt);

    int HitTest(const wxPoint& p) const;
    bool HitCenter(const Circle& c, const wxPoint& p) const;
    double Dist(double x1, double y1, double x2, double y2) const;

private:
    wxBitmap bitmap_;
    wxImage image_;
    bool hasImage_ = false;

    std::string imagePath_;
    std::string labelsPath_;
    std::vector<Circle> circles_;
    std::vector<Circle> detected_;
    bool showDetections_ = true;

    int active_ = -1;
    enum class DragMode { None, MoveCenter, ResizeRadius };
    DragMode dragMode_ = DragMode::None;

    wxPoint lastMouse_;
    double hitTolPx_ = 10.0;   // допуск клика по центру/границе
    wxDECLARE_EVENT_TABLE();
};