#pragma once
#include <wx/wx.h>
#include "Canvas.hpp"

class MainFrame : public wxFrame {
public:
    MainFrame();

private:
    void OnOpen(wxCommandEvent& evt);
    void OnSave(wxCommandEvent& evt);
    void OnExit(wxCommandEvent& evt);

private:
    Canvas* canvas_ = nullptr;
    wxDECLARE_EVENT_TABLE();
};