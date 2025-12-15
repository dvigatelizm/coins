#include "MainFrame.hpp"
#include <wx/filedlg.h>

enum {
    ID_Open = wxID_HIGHEST + 1,
    ID_Save
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_Open, MainFrame::OnOpen)
EVT_MENU(ID_Save, MainFrame::OnSave)
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

    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
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