
#ifndef __TRAY_H__
#define __TRAY_H__

#include <wx/taskbar.h>

class IHTaskBarIcon: public wxTaskBarIcon
{

	wxFrame					*m_MainFrame;

public:
	IHTaskBarIcon();

	void SetMainFrame(wxFrame *frame) { m_MainFrame = frame; };
	
	void OnDoubleClick(wxTaskBarIconEvent &event);
	void OnShow(wxCommandEvent &event);
	void OnQuit(wxCommandEvent &event);
	
	virtual wxMenu *CreatePopupMenu();

	virtual void ShowBalloonNotification(wxString title, wxString info);

	DECLARE_EVENT_TABLE()
};

#endif
