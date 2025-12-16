#include "Canvas.hpp"
#include <opencv2/imgproc.hpp>
#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;

wxBEGIN_EVENT_TABLE(Canvas, wxPanel)
EVT_PAINT(Canvas::OnPaint)
EVT_LEFT_DOWN(Canvas::OnLeftDown)
EVT_LEFT_UP(Canvas::OnLeftUp)
EVT_MOTION(Canvas::OnMotion)
EVT_RIGHT_DOWN(Canvas::OnRightDown)
EVT_KEY_DOWN(Canvas::OnKeyDown)
wxEND_EVENT_TABLE()

Canvas::Canvas(wxWindow* parent)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxFULL_REPAINT_ON_RESIZE | wxBORDER_SIMPLE) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetFocus(); // чтобы ловить клавиши (Del, S и т.п.)
}

cv::Mat Canvas::GetImageMat() const {
    if (!hasImage_)
        return cv::Mat();

    wxImage img = image_;

    cv::Mat mat(
        img.GetHeight(),
        img.GetWidth(),
        CV_8UC3,
        img.GetData()
    );

    cv::Mat matBGR;
    cv::cvtColor(mat, matBGR, cv::COLOR_RGB2BGR);

    return matBGR.clone();
}

const std::vector<Circle>& Canvas::GetGroundTruthCircles() const {
    return circles_;
}

bool Canvas::LoadImage(const std::string& imgPath) {
    imagePath_ = imgPath;
    if (!image_.LoadFile(imgPath)) {
        hasImage_ = false;
        return false;
    }
    
    // сброс детекций при смене изображения
    detected_.clear();
    showDetections_ = false;

    bitmap_ = wxBitmap(image_);
    hasImage_ = true;

    // Под размер картинки
    SetMinSize(wxSize(image_.GetWidth(), image_.GetHeight()));
    SetSize(wxSize(image_.GetWidth(), image_.GetHeight()));
    Refresh();
    return true;
}

bool Canvas::LoadOrCreateLabelsForImage() {
    if (!hasImage_) return false;
    fs::path p(imagePath_);
    fs::path lbl = p.parent_path() / (p.stem().string() + "_labels.txt");
    labelsPath_ = lbl.string();

    circles_ = LoadLabels(labelsPath_);
    active_ = circles_.empty() ? -1 : 0;
    Refresh();
    return true;
}

bool Canvas::SaveLabels() {
    if (labelsPath_.empty()) return false;
    return ::SaveLabels(labelsPath_, circles_);
}

double Canvas::Dist(double x1, double y1, double x2, double y2) const {
    const double dx = x1 - x2;
    const double dy = y1 - y2;
    return std::sqrt(dx * dx + dy * dy);
}

bool Canvas::HitCenter(const Circle& c, const wxPoint& p) const {
    return Dist(c.cx, c.cy, p.x, p.y) <= hitTolPx_;
}

int Canvas::HitTest(const wxPoint& p) const {
    // 1) сначала центр
    for (int i = (int)circles_.size() - 1; i >= 0; --i) {
        if (HitCenter(circles_[i], p)) return i;
    }
    // 2) потом граница окружности
    for (int i = (int)circles_.size() - 1; i >= 0; --i) {
        double d = Dist(circles_[i].cx, circles_[i].cy, p.x, p.y);
        if (std::abs(d - circles_[i].r) <= hitTolPx_) return i;
    }
    return -1;
}

void Canvas::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    if (!hasImage_) {
        dc.DrawText("Open an image to start", 10, 10);
        return;
    }

    dc.DrawBitmap(bitmap_, 0, 0, false);

    // Рисуем окружности
    wxPen penRed(*wxRED, 2);
    wxPen penGreen(*wxGREEN, 2);

    // ручная разметка
    for (size_t i = 0; i < circles_.size(); ++i) {
        const auto& c = circles_[i];
        dc.SetPen((int)i == active_ ? penGreen : penRed);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawCircle(wxPoint((int)std::round(c.cx), (int)std::round(c.cy)),
            (int)std::round(c.r));

        // центр
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush((int)i == active_ ? *wxGREEN_BRUSH : *wxBLUE_BRUSH);
        dc.DrawCircle(wxPoint((int)std::round(c.cx), (int)std::round(c.cy)), 3);
    }

    // ===== Детекция (синие пунктирные окружности) =====
    if (showDetections_) {
        wxPen detPen(
            wxColour(50, 100, 255), // насыщенный синий
            5,                      // тоньше ручной
            wxPENSTYLE_SHORT_DASH   // пунктир
        );

        dc.SetPen(detPen);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);

        for (const auto& d : detected_) {
            // рисуем окружность детекции
            dc.DrawCircle(
                wxPoint(
                    (int)std::round(d.cx),
                    (int)std::round(d.cy)
                ),
                (int)std::round(d.r)
            );

            // confidence (по желанию, но очень полезно)
            /*dc.DrawText(
                wxString::Format("score=%.2f", d.confidence),
                (int)std::round(d.cx + d.r + 2),
                (int)std::round(d.cy)
            );*/
        }
    }
}

void Canvas::OnLeftDown(wxMouseEvent& evt) {
    if (!hasImage_) return;
    SetFocus();

    wxPoint p = evt.GetPosition();
    int idx = HitTest(p);
    active_ = idx;

    if (active_ >= 0) {
        const auto& c = circles_[active_];
        if (HitCenter(c, p)) dragMode_ = DragMode::MoveCenter;
        else dragMode_ = DragMode::ResizeRadius;
        lastMouse_ = p;
        CaptureMouse();
    }

    Refresh();
}

void Canvas::OnLeftUp(wxMouseEvent&) {
    if (HasCapture()) ReleaseMouse();
    dragMode_ = DragMode::None;
}

void Canvas::OnMotion(wxMouseEvent& evt) {
    if (!hasImage_) return;
    if (active_ < 0) return;
    if (!evt.Dragging() || !evt.LeftIsDown()) return;

    wxPoint p = evt.GetPosition();
    auto& c = circles_[active_];

    if (dragMode_ == DragMode::MoveCenter) {
        c.cx = p.x;
        c.cy = p.y;
    }
    else if (dragMode_ == DragMode::ResizeRadius) {
        c.r = std::max(1.0, Dist(c.cx, c.cy, p.x, p.y));
    }
    Refresh();
}

void Canvas::OnRightDown(wxMouseEvent& evt) {
    if (!hasImage_) return;
    SetFocus();

    wxPoint p = evt.GetPosition();
    // Добавить новый круг (радиус по умолчанию)
    circles_.push_back(Circle{ (double)p.x, (double)p.y, 40.0 });
    active_ = (int)circles_.size() - 1;
    Refresh();
}

void Canvas::OnKeyDown(wxKeyEvent& evt) {
    if (evt.GetKeyCode() == WXK_DELETE) {
        if (active_ >= 0 && active_ < (int)circles_.size()) {
            circles_.erase(circles_.begin() + active_);
            if (circles_.empty()) active_ = -1;
            else active_ = std::min(active_, (int)circles_.size() - 1);
            Refresh();
        }
        return;
    }

    // Быстрое сохранение: Ctrl+S или просто S
    if (evt.GetKeyCode() == 'S' && (evt.ControlDown() || !evt.ControlDown())) {
        SaveLabels();
        return;
    }

    evt.Skip();
}

void Canvas::SetDetectedCircles(const std::vector<Circle>& circles) {
    detected_ = circles;
    showDetections_ = true;
    Refresh();
}
