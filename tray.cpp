#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "defines.h"
#include "tray.h"

BEGIN_EVENT_TABLE(IHTaskBarIcon, wxTaskBarIcon)
	EVT_TASKBAR_LEFT_DCLICK(IHTaskBarIcon::OnDoubleClick)
	EVT_MENU(ID_TASKBAR_SHOW, IHTaskBarIcon::OnShow)
	EVT_MENU(ID_TASKBAR_QUIT, IHTaskBarIcon::OnQuit)
END_EVENT_TABLE()

// IHTaskBarIcon
//

IHTaskBarIcon::IHTaskBarIcon()
:	m_MainFrame(NULL)
{
}

void IHTaskBarIcon::OnDoubleClick(wxTaskBarIconEvent & WXUNUSED(event))
{
	m_MainFrame->Show();
	m_MainFrame->Iconize(false);
}

wxMenu *IHTaskBarIcon::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu();

	menu->Append(ID_TASKBAR_SHOW, L"Открыть Интернет-Помощник");
	menu->AppendSeparator();
	menu->Append(ID_TASKBAR_QUIT, L"Выход");

	return menu;
}

void IHTaskBarIcon::ShowBalloonNotification(wxString title, wxString info)
{
	wxASSERT_MSG( m_win != NULL, _T("taskbar icon not initialized") );

	NOTIFYICONDATA nid;
	size_t tries = 0;
	wxFrame *m_fwin = (wxFrame *)m_win;

	memset(&nid, 0, NOTIFYICONDATA_V2_SIZE);
	
	nid.cbSize = NOTIFYICONDATA_V2_SIZE;
	nid.hWnd = (HWND) m_fwin->GetHWND();
	nid.uFlags = NIF_INFO;
	nid.uID = 99;
	nid.dwInfoFlags = NIIF_WARNING;
	nid.uTimeout = 15000;

    if (m_icon.Ok())
    {
        nid.uFlags |= NIF_ICON;
        nid.hIcon = GetHiconOf(m_icon);
    }

	wxStrncpy(nid.szInfo, info.c_str(), WXSIZEOF(nid.szInfo));
	wxStrncpy(nid.szInfoTitle, title.c_str(), WXSIZEOF(nid.szInfoTitle));

	while (::Shell_NotifyIcon(NIM_MODIFY, &nid) == FALSE && tries++ < 10)
		::Sleep(200);
}

void IHTaskBarIcon::OnShow(wxCommandEvent & WXUNUSED(event))
{
	m_MainFrame->Show();
	m_MainFrame->Iconize(false);
}

void IHTaskBarIcon::OnQuit(wxCommandEvent & WXUNUSED(event))
{
	m_MainFrame->Close(true);
}
