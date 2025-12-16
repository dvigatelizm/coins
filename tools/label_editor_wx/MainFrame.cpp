#include "MainFrame.hpp"
#include <wx/filedlg.h>
#include "Detector.hpp"
#include "evaluator.hpp"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_Open, MainFrame::OnOpen)
EVT_MENU(ID_Save, MainFrame::OnSave)
EVT_MENU(ID_Detect, MainFrame::OnDetect)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "Label Editor (wxWidgets)",
        wxDefaultPosition, wxSize(900, 700)) {

    wxMenu* fileMenu = new wxMenu();
    fileMenu->Append(ID_Open, "&Open...\tCtrl+O");
    fileMenu->Append(ID_Save, "&Save labels\tCtrl+S");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit");

    wxMenu* toolsMenu = new wxMenu();
    toolsMenu->Append(ID_Detect, "&Detect\tCtrl+D");

    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(toolsMenu, "&Tools");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Open an image. Right click adds a circle. Drag to move/resize. Del deletes. S saves.");

    canvas_ = new Canvas(this);
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(canvas_, 1, wxEXPAND);
    SetSizer(sizer);
}

void MainFrame::OnOpen(wxCommandEvent&) {
    wxFileDialog openFileDialog(
        this,
        "Open image",
        "",
        "",
        "Images (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (openFileDialog.ShowModal() == wxID_CANCEL) return;

    std::string imgPath = openFileDialog.GetPath().ToStdString();
    if (!canvas_->LoadImage(imgPath)) {
        wxMessageBox("Failed to load image", "Error", wxICON_ERROR);
        return;
    }

    canvas_->LoadOrCreateLabelsForImage();
    SetStatusText(("Opened: " + imgPath).c_str());
    Layout();
    Fit(); // под размер картинки (можно убрать, если неудобно)
}

void MainFrame::OnSave(wxCommandEvent&) {
    if (!canvas_->SaveLabels()) {
        wxMessageBox("Failed to save labels", "Error", wxICON_ERROR);
        return;
    }
    SetStatusText(("Saved: " + canvas_->GetLabelsPath()).c_str());
}

void MainFrame::OnExit(wxCommandEvent&) {
    Close(true);
}

void MainFrame::OnDetect(wxCommandEvent&) {
    if (!canvas_) return;

    // 1. Берём изображение из Canvas
    cv::Mat img = canvas_->GetImageMat();
    if (img.empty()) {
        wxMessageBox("No image loaded", "Error", wxICON_ERROR);
        return;
    }

    // 2. Запускаем детектор
    Detector detector;
    auto detections = detector.run(img);

    // 3. Преобразуем Detection -> Circle
    std::vector<Circle> circles;
    for (const auto& d : detections) {
        Circle c;
        c.cx = d.bbox.x + d.bbox.width * 0.5;
        c.cy = d.bbox.y + d.bbox.height * 0.5;
        c.r = 0.5 * std::min(d.bbox.width, d.bbox.height);
        circles.push_back(c);
    }

    // ===== 3.5 ОЦЕНКА КАЧЕСТВА =====
    Evaluator eval;

    // детекции (уже есть)
    std::vector<DetectedCircle> detectedEval;
    detectedEval.reserve(circles.size());

    for (const auto& c : circles) {
        DetectedCircle dc;
        dc.center = cv::Point2f(c.cx, c.cy);
        dc.radius = c.r;
        dc.score = 1.0f; // или d.confidence, если хочешь
        detectedEval.push_back(dc);
    }

    // ground truth
    const auto& gtCircles = canvas_->GetGroundTruthCircles();

    std::vector<GTCircle> gtEval;
    gtEval.reserve(gtCircles.size());

    for (const auto& c : gtCircles) {
        GTCircle gt;
        gt.center = cv::Point2f(c.cx, c.cy);
        gt.radius = static_cast<float>(c.r);
        gtEval.push_back(gt);
    }

    auto result = eval.evaluate(detectedEval, gtEval);

    float precision = result.TP / float(result.TP + result.FP + 1e-6f);
    float recall = result.TP / float(result.TP + result.FN + 1e-6f);
    float f1 = 2.0f * precision * recall / (precision + recall + 1e-6f);

    // 4. Передаём в Canvas
    canvas_->SetDetectedCircles(circles);

    // 5. Сообщение
    SetStatusText(
        wxString::Format(
            "Detections=%zu | TP=%d FP=%d FN=%d | P=%.2f R=%.2f F1=%.2f",
            detections.size(),
            result.TP,
            result.FP,
            result.FN,
            precision,
            recall,
            f1
        )
    );
}