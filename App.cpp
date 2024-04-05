#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>

wxIMPLEMENT_APP(App);

bool App:: OnInit()
{
	const auto frame = new MainFrame("mini Photoshop");

	//set Size
	frame->Maximize();
	frame->Show(true);
	return true;
}
