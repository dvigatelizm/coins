#pragma once
#include <wx/wx.h>
#include "Canvas.hpp"

enum
{
    ID_Open = wxID_HIGHEST + 1,
    ID_Save,
    ID_Detect
};

class MainFrame : public wxFrame
{
public:
    MainFrame();

private:
    void OnOpen(wxCommandEvent& evt);
    void OnSave(wxCommandEvent& evt);
    void OnExit(wxCommandEvent& evt);
    void OnDetect(wxCommandEvent& evt);

private:
    Canvas* canvas_ = nullptr;
    wxDECLARE_EVENT_TABLE();
};
