#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "defines.h"
#include "traceroute.h"

IMPLEMENT_DYNAMIC_CLASS(IHTracerouteFrame, wxFrame)

BEGIN_EVENT_TABLE(IHTracerouteFrame, wxFrame)
	EVT_BUTTON(ID_TRACEROUTE_START, IHTracerouteFrame::OnStart)
END_EVENT_TABLE()

// IHTracerouteFrame
//

IHTracerouteFrame::IHTracerouteFrame()
{
}

IHTracerouteFrame::~IHTracerouteFrame()
{
}

IHTracerouteFrame::IHTracerouteFrame(wxWindow * parent,
									 wxWindowID id, 
									 const wxString & host,
									 const wxString & title, 
									 const wxPoint & pos,
									 const wxSize & size, 
									 long style)
{
	Create(parent, id, host, title, pos, size, style);
}

bool IHTracerouteFrame::Create(wxWindow * parent,
							   wxWindowID id, 
							   const wxString & host,
							   const wxString & title, 
							   const wxPoint & pos,
							   const wxSize & size, 
							   long style)
{
	bool res = wxFrame::Create(parent, id, title, pos, size, style);

	m_Host = host;
	
	if (res)
		CreateControls();

	return res;
}


void IHTracerouteFrame::CreateControls()
{
    SetIcon(wxICON(IHIcon));
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

	wxPoint current(15, 15);
	wxPanel *panel;
	wxFont font;

	panel = new wxPanel(this);

	m_HostEdit = new wxTextCtrl(panel, wxID_ANY, m_Host, current, wxSize(GetSize().x - 130, -1));
	m_StartButton = new wxButton(panel, ID_TRACEROUTE_START, L"Запустить", current + wxPoint(GetSize().x - 120, -1));

	current.y += 35;

	m_Results = new wxTextCtrl(panel, wxID_ANY, L"", current, wxSize(GetSize().x - 45, GetSize().y - 100),
		wxTE_MULTILINE | wxTE_READONLY);

	font = m_Results->GetFont();
	font.SetFaceName(L"Courier New");

	m_Results->SetFont(font);

	OnStart(wxCommandEvent());
}

void IHTracerouteFrame::OnStart(wxCommandEvent & WXUNUSED(event))
{
	double loss;
	unsigned int rtt, consecutive_no_reply = 0;
	wxString reply_address;
	wxIPV4address ipv4;
	int ttl = 1;

	m_StartButton->Enable(false);
	m_Results->SetValue(L"");

	UpdateWindowUI(wxUPDATE_UI_RECURSE);

	if (m_HostEdit->GetValue().Length() != 0 && ipv4.Hostname(m_HostEdit->GetValue()))
	{
		while (ttl < 30 && reply_address != ipv4.IPAddress() && consecutive_no_reply < 5)
		{
			if (m_Diagnosis.Ping(ipv4.IPAddress(), 1000, 1, &loss, &rtt, ttl, &reply_address) && loss == 0.0)
			{
				*m_Results << wxString::Format(L"%2d: (%4d мс) - %s\r\n", ttl, rtt, reply_address);
				consecutive_no_reply = 0;
			}
			else
			{
				*m_Results << wxString::Format(L"%2d: (нет ответа)\r\n", ttl);
				consecutive_no_reply++;
			}

			ttl++;

			UpdateWindowUI(wxUPDATE_UI_RECURSE);
		}
	}
	else
	{
		*m_Results << L"Не удалось определить IP-адрес узла.\r\n";
	}

	m_StartButton->Enable();
}
