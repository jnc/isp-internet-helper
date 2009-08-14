
#ifndef __TRACEROUTE_H__
#define __TRACEROUTE_H__

#include <wx/socket.h>

#include "diagnosis.h"

class IHTracerouteFrame : public wxFrame
{
	wxDiagnosis			 m_Diagnosis;

	wxString			 m_Host;

	wxTextCtrl			*m_HostEdit,
						*m_Results;

	wxButton			*m_StartButton;

	void CreateControls();

	void OnStart(wxCommandEvent &event);

	DECLARE_DYNAMIC_CLASS(IHTracerouteFrame)

public:
	IHTracerouteFrame();
    IHTracerouteFrame(wxWindow *parent,
					  wxWindowID id = wxID_ANY,
					  const wxString &host = L"ya.ru",
					  const wxString &title = L"Трассировка",
					  const wxPoint &pos = wxDefaultPosition,
					  const wxSize &size = wxDefaultSize,
					  long style = wxDEFAULT_FRAME_STYLE);
	~IHTracerouteFrame();

	bool Create(wxWindow *parent,
				wxWindowID id = wxID_ANY,
				const wxString &host = L"ya.ru",
				const wxString &title = L"Трассировка",
				const wxPoint &pos = wxDefaultPosition,
				const wxSize &size = wxDefaultSize,
				long style = wxDEFAULT_FRAME_STYLE);

    DECLARE_EVENT_TABLE()
};

#endif