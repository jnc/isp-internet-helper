
#ifndef __MAIN_H__
#define __MAIN_H__

#include <wbemidl.h>

#include <wx/treectrl.h>
#include <wx/aui/aui.h>
#include <wx/hashmap.h>
#include <wx/progdlg.h>
#include <wx/socket.h>
#include <wx/hyperlink.h>
#include <wx/aboutdlg.h>
#include <wx/config.h>

#include "interfaces.h"
#include "routes.h"
#include "diagnosis.h"
#include "traceroute.h"
#include "update.h"
#include "tray.h"

WX_DECLARE_HASH_MAP(int, wxTreeItemId, wxIntegerHash, wxIntegerEqual, wxTreeItemsMap);

class IHFrame;

class IHApp : public wxApp
{
	IHFrame				*m_MainFrame;

public:
    virtual bool OnInit();

	IHFrame *GetMainFrame() { return m_MainFrame; };
};

DECLARE_APP(IHApp);

class IHFrame : public wxFrame
{
	wxAuiManager		 m_Manager;
	wxTreeCtrl			*m_InfoTree;
	wxAuiNotebook		*m_Notebook;

	IWbemLocator		*m_WbemLocator;
	IWbemServices		*m_WbemServices;

	wxNetworkInterfaceArray
						 m_Interfaces;

	wxRoutingEntryArray	 m_RoutingTable;

	wxString			 m_DefaultGateway;

	wxTreeItemsMap		 m_TreeItems;

	wxDiagnosis			 m_Diagnosis;

	wxUpdateThread		*m_UpdateThread;
	wxDateTime			 m_LastPendingPaymentCheck;

	wxConfig			 m_Config;

	IHTaskBarIcon		 m_TaskBarIcon;

	wxLogNull			 m_LogNull;

	void CreateControls();
	void CreateInfoTree();

	void EnumerateInterfaces(wxProgressDialog &progress);
	void EnumerateRoutes(wxProgressDialog &progress);

	int GetDefaultInterface(wxString &nexthop) const;

	void QueryAddressing(wxProgressDialog &progress);

	void OnInterfacesSelected();
	void OnDiagnosisSelected();
	void OnRootSelected();
	void OnAccountSelected();

	DECLARE_DYNAMIC_CLASS(IHFrame)

public:
	IHFrame();
    IHFrame(wxWindow *parent,
			wxWindowID id = wxID_ANY,
			const wxString &title = L"Internet Helper",
			const wxPoint &pos = wxDefaultPosition,
			const wxSize &size = wxDefaultSize,
			long style = wxDEFAULT_FRAME_STYLE);
	~IHFrame();

	bool Create(wxWindow *parent,
				wxWindowID id = wxID_ANY,
				const wxString &title = L"Internet Helper",
				const wxPoint &pos = wxDefaultPosition,
				const wxSize &size = wxDefaultSize,
				long style = wxDEFAULT_FRAME_STYLE);

    void OnQuit(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);
	void OnTraceroute(wxCommandEvent &event);
	void OnTreeSelChanged(wxTreeEvent &event);
	void OnStartUpdateProcess(wxCommandEvent &event);
	void OnAccountInformationChanged(wxCommandEvent &event);
	void OnSettings(wxCommandEvent &event);
	void OnIdle(wxIdleEvent &event);
	void OnClose(wxCloseEvent &event);
	void OnIconize(wxIconizeEvent &event);

    DECLARE_EVENT_TABLE()
};

#endif