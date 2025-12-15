#include <wx/wx.h>
#include "MainFrame.hpp"

class App : public wxApp {
public:
    bool OnInit() override {
        wxInitAllImageHandlers();
        
        auto* frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(App);