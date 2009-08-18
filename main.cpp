#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "main.h"
#include "defines.h"
#include "account.h"
#include "version.h"

#include <wx/protocol/http.h>
#include <wx/filename.h>
#include <wx/file.h>

IMPLEMENT_DYNAMIC_CLASS(IHFrame, wxFrame)

BEGIN_EVENT_TABLE(IHFrame, wxFrame)
    EVT_MENU(wxID_EXIT, IHFrame::OnQuit)
	EVT_CLOSE(IHFrame::OnClose)
	EVT_MENU(wxID_ABOUT, IHFrame::OnAbout)
	EVT_MENU(ID_START_UPDATE_PROCESS, IHFrame::OnStartUpdateProcess)
	EVT_MENU(ID_ACCOUNT_INFORMATION_CHANGED, IHFrame::OnAccountInformationChanged)
	EVT_MENU(ID_SETTINGS, IHFrame::OnSettings)
	EVT_BUTTON(ID_TRACEROUTE, IHFrame::OnTraceroute)
	EVT_TREE_SEL_CHANGED(ID_INFO_TREE, IHFrame::OnTreeSelChanged)
	EVT_IDLE(IHFrame::OnIdle)
	EVT_ICONIZE(IHFrame::OnIconize)
END_EVENT_TABLE()

IMPLEMENT_APP(IHApp)

// ============================================================================
// implementation
// ============================================================================

bool IHApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

	HANDLE mtx = ::CreateMutex(NULL, FALSE, L"Global\\InternetHelper");

	if (mtx == NULL || ::GetLastError() == ERROR_ALREADY_EXISTS)
		return false;

	// proceed with wx initialization
	//

	wxInitAllImageHandlers();
	wxSocketBase::Initialize();

	m_MainFrame = new IHFrame(NULL, wxID_ANY, L"Интернет-Помощник", wxDefaultPosition, wxSize(600, 500));

	m_MainFrame->Centre();
	m_MainFrame->Show(true);

    return true;
}



// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

IHFrame::IHFrame()
{
}

IHFrame::IHFrame(wxWindow * parent,
				 wxWindowID id, 
				 const wxString & title, 
				 const wxPoint & pos,
				 const wxSize & size, 
				 long style)
				 :
	m_Interfaces(),
	m_TreeItems(),
	m_WbemLocator(NULL),
	m_WbemServices(NULL),
	m_Notebook(NULL),
	m_UpdateThread(NULL),
	m_Config(L"Internet Helper", L"REAL")
{
	HRESULT hres;

	// initialize OLE
	//

	hres = ::CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (hres != RPC_E_CHANGED_MODE)
	{
		if (FAILED(hres))
		{
			wxMessageDialog(NULL, 
				L"Не удалось инициализировать подсистему COM.  Пожалуйста, обратитесь в техническую поддержку.",
				L"Ошибка", wxOK).ShowModal();
		}
	}
	else
	{
		hres = S_OK;
	}

	if (SUCCEEDED(hres) && ::CoInitializeSecurity(NULL,
												  -1,
												  NULL,
												  NULL,
												  RPC_C_AUTHN_LEVEL_PKT,
												  RPC_C_IMP_LEVEL_IMPERSONATE,
												  NULL,
												  EOAC_NONE,
												  0) != S_OK)
	{
		wxMessageDialog(NULL, 
			L"Не удалось инициализировать подсистему COM.  Пожалуйста, обратитесь в техническую поддержку.",
			L"Ошибка", wxOK).ShowModal();

		::CoUninitialize();
	}
	else
	{
		BSTR bstrNamespace = L"root\\cimv2";
		HRESULT hres;

		hres = ::CoCreateInstance(CLSID_WbemAdministrativeLocator,
								  NULL,
								  CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
								  IID_IUnknown,
								  (void **)&m_WbemLocator);

		if (SUCCEEDED(hres) && m_WbemLocator != NULL)
		{
			hres = m_WbemLocator->ConnectServer(bstrNamespace,
												NULL,
												NULL,
												NULL,
												0,
												NULL,
												NULL,
												&m_WbemServices);

			if (SUCCEEDED(hres) && m_WbemServices != NULL)
			{
				// all OK
				//
			}
			else
			{
				wxMessageDialog(NULL,
					L"Не удалось инициализировать подсистему COM (WbemServices).  Пожалуйста, обратитесь в техническую поддержку.",
					L"Ошибка", wxOK).ShowModal();

				m_WbemLocator->Release();

				m_WbemServices = NULL;
				m_WbemLocator = NULL;

				::CoUninitialize();
			}
		}
		else
		{
			wxMessageDialog(NULL,
				L"Не удалось инициализировать подсистему COM (WbemLocator).  Пожалуйста, обратитесь в техническую поддержку.",
				L"Ошибка", wxOK).ShowModal();

			m_WbemLocator = NULL;

			::CoUninitialize();
		}
	}

	// start update thread
	//

	if (m_UpdateThread == NULL)
	{
		wxString login, password;

		m_LastPendingPaymentCheck = wxDateTime::Now() - wxTimeSpan::Minutes(1440);

		m_Config.Read(L"/Account/Login", &login, L"");
		m_Config.Read(L"/Account/PasswordClear", &password, L"");

		if (login.Length() == 0 || password.Length() == 0)
		{
			wxSettingsDialog dlg(this, &m_Config);

			if (dlg.ShowModal() == wxID_OK)
			{
				m_Config.Read(L"/Account/Login", &login, L"");
				m_Config.Read(L"/Account/PasswordClear", &password, L"");
			}
		}

		m_UpdateThread = new wxUpdateThread();

		m_UpdateThread->SetAccountLogin(login);
		m_UpdateThread->SetAccountPassword(password);

		m_UpdateThread->Create();
		m_UpdateThread->Run();
	}

	// create frame and controls
	//

	Create(parent, id, title, pos, size, style);
}

bool IHFrame::Create(wxWindow * parent,
					 wxWindowID id, 
					 const wxString & title, 
					 const wxPoint & pos,
					 const wxSize & size, 
					 long style)
{
	bool res = wxFrame::Create(parent, id, title, pos, size, style);
	
	if (res)
		CreateControls();

	return res;
}


void IHFrame::CreateControls()
{
    SetIcon(wxICON(IHIcon));
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

	wxMenuBar *menuBar = new wxMenuBar;
	SetMenuBar(menuBar);

	wxMenu *fileMenu = new wxMenu;

	fileMenu->Append(ID_SETTINGS, L"Настройки...");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_ABOUT, _("О Программе..."));
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, _("Выход\tAlt+F4"));

	menuBar->Append(fileMenu, _("Интернет-Помощник"));

	m_Manager.SetManagedWindow(this);

	CreateInfoTree();

	m_Notebook = new wxAuiNotebook(this, ID_NOTEBOOK, wxDefaultPosition, wxSize(600, 450),
		wxAUI_NB_TOP | wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);

	m_Manager.AddPane(m_Notebook, wxAuiPaneInfo().CenterPane());
	m_Manager.AddPane(m_InfoTree, wxAuiPaneInfo().Left().Layer(1).Caption(L"Information").CloseButton(false).
		Floatable(false));

	m_Manager.Update();

	OnRootSelected();

	m_TaskBarIcon.SetMainFrame(this);
	m_TaskBarIcon.SetIcon(wxICON(IHIcon));
}

void IHFrame::CreateInfoTree()
{
	m_InfoTree = new wxTreeCtrl(this, ID_INFO_TREE, wxDefaultPosition, wxSize(170, 250),
		wxTR_HAS_BUTTONS|wxTR_LINES_AT_ROOT|wxTR_SINGLE);
	
	wxTreeItemId root = m_InfoTree->AddRoot(L"Интернет-Помощник");
	wxTreeItemId local_resources;

	m_TreeItems[ID_TREEITEM_ROOT] = root;
	
	m_TreeItems[ID_TREEITEM_ACCOUNT] = m_InfoTree->AppendItem(root, L"Состояние Счёта");
	m_TreeItems[ID_TREEITEM_INTERFACES] = m_InfoTree->AppendItem(root, L"Сетевые Адаптеры");
	m_TreeItems[ID_TREEITEM_DIAGNOSIS] = m_InfoTree->AppendItem(root, L"Диагностика");
	
	//local_resources = m_InfoTree->AppendItem(root, L"Локальные Ресурсы");
	//m_InfoTree->AppendItem(local_resources, L"IPTV");
	//m_InfoTree->AppendItem(local_resources, L"DC++");

	m_InfoTree->Expand(root);
}


IHFrame::~IHFrame()
{
	m_Manager.UnInit();

	if (m_UpdateThread != NULL)
	{
		m_UpdateThread->Delete();
		m_UpdateThread = NULL;
	}
}

void IHFrame::QueryAddressing(wxProgressDialog &progress)
{
	// get interfaces and routes
	//

	EnumerateInterfaces(progress);
	EnumerateRoutes(progress);

	// get addressing information on interfaces
	//

	for (size_t i = 0; i < m_Interfaces.GetCount(); i++)
	{
		progress.Pulse();
		m_Interfaces[i].GetAddressing(m_WbemServices);
	}

	// get default interface (to which default route belongs)
	//

	GetDefaultInterface(m_DefaultGateway);
}

void IHFrame::OnRootSelected()
{
	wxPanel *panel = new wxPanel(m_Notebook, wxID_ANY);
	wxPoint current(15, 15);
	wxPoint shift(150, 0);
	wxStaticText *text;

	// construct root page
	//

	text = new wxStaticText(panel, wxID_ANY, 
		wxString::Format(L"Добро пожаловать в Интернет-Помощник %d.%d.%d.%d!\n\n"

						 L"С помощью этой программы общение со службой технической поддержки\n"
						 L"станет быстрее и проще, а некоторые неисправности Вы сможете\n"
						 L"устранить самостоятельно.\n\n"

						 L"Также, Вы сможете определить, работает ли IPTV или DC++ в Вашем\n"
						 L"сегменте сети, оперативно следить за балансом Вашего лицевого счёта,\n"
						 L"получать новости компании и управлять своими услугами."

						 L"",
						 PRODUCTVER
						 ),
		current);

	current.y += text->GetSize().y + 20;

	new wxStaticText(panel, wxID_ANY, L"Наш сайт:", current);

	new wxHyperlinkCtrl(panel, wxID_ANY, L"http://real.astrakhan.ru/", L"http://real.astrakhan.ru/",
		current + shift);

	current.y += 20;

	new wxStaticText(panel, wxID_ANY, L"Служба тех. поддержки:", current);
	new wxStaticText(panel, wxID_ANY, L"+7 (8512) 480000", current + shift);

	current.y += 20;

	new wxStaticText(panel, wxID_ANY, L"Менеджеры продаж:", current);
	new wxStaticText(panel, wxID_ANY, L"+7 (8512) 481600", current + shift);

	current.y += 20;

	new wxStaticText(panel, wxID_ANY, L"Факс:", current);
	new wxStaticText(panel, wxID_ANY, L"+7 (8512) 481608", current + shift);

	// add page
	//

	m_Notebook->Freeze();

	while (m_Notebook->GetPageCount() > 0)
		m_Notebook->DeletePage(0);

	m_Notebook->AddPage(panel, L"Интернет-Помощник");

	m_Notebook->Thaw();
}

void IHFrame::OnTreeSelChanged(wxTreeEvent &event)
{
	if (m_TreeItems[ID_TREEITEM_INTERFACES] == event.GetItem())
		OnInterfacesSelected();
	else if (m_TreeItems[ID_TREEITEM_DIAGNOSIS] == event.GetItem())
		OnDiagnosisSelected();
	else if (m_TreeItems[ID_TREEITEM_ROOT] == event.GetItem())
		OnRootSelected();
	else if (m_TreeItems[ID_TREEITEM_ACCOUNT] == event.GetItem())
		OnAccountSelected();
}

void IHFrame::OnIdle(wxIdleEvent & WXUNUSED(event))
{
}

void IHFrame::OnAbout(wxCommandEvent & WXUNUSED(event))
{
	wxAboutDialogInfo about;

	about.SetName(L"Интернет-Помощник");
	about.SetVersion(wxString::Format(L"%d.%d.%d.%d", PRODUCTVER));
	about.SetCopyright(L"(c) 2009 ООО НТС \"РЕАЛ\"");
	about.AddDeveloper(L"Alexander Trunoff <jnc@real.astrakhan.ru>");
	about.SetWebSite(L"http://real.astrakhan.ru");

	wxAboutBox(about);
}

void IHFrame::OnTraceroute(wxCommandEvent & WXUNUSED(event))
{
	IHTracerouteFrame *tf = new IHTracerouteFrame(this);

	tf->Show();
}

void IHFrame::OnAccountInformationChanged(wxCommandEvent & WXUNUSED(event))
{
	if (m_InfoTree->GetSelection() == m_TreeItems[ID_TREEITEM_ACCOUNT])
		OnAccountSelected();

	// update taskbar icon hint with current balance
	// 

	wxString status;

	if (m_UpdateThread->GetAccountError().Length() == 0)
	{
		status = wxString::Format(L"Остаток на счету: %.2f руб.",
			m_UpdateThread->GetAccountBalance() + m_UpdateThread->GetAccountCredit());
	}
	else
	{
		status = L"Остаток на счету: ошибка при запросе остатка.";
	}

	m_TaskBarIcon.SetIcon(wxICON(IHIcon), status);

	// display pending payment balloon tip
	//

	int pending_payment_check_interval, pending_payment_advance_interval;

	m_Config.Read(L"/Account/PendingPaymentCheckInterval", &pending_payment_check_interval, 60);
	m_Config.Read(L"/Account/PendingPaymentAdvanceInterval", &pending_payment_advance_interval, 3);

	if (m_UpdateThread->GetPendingPaymentServices().Length() != 0 &&
		m_LastPendingPaymentCheck + wxTimeSpan::Minutes(pending_payment_check_interval) <= wxDateTime::Now() &&
		m_UpdateThread->GetPendingPaymentDateTime() - wxDateSpan::Days(pending_payment_advance_interval) <= wxDateTime::Now() &&
		m_UpdateThread->GetPendingPaymentSum() > m_UpdateThread->GetAccountBalance() + m_UpdateThread->GetAccountCredit())
	{
		m_TaskBarIcon.ShowBalloonNotification(
			L"Оплата тарифа", 
			wxString::Format(L"Необходимо до %s %s произвести оплату услуг.\n"
				L"\n"
				L"Услуги: %s.\n"
				L"Сумма: %.2f руб.",
				m_UpdateThread->GetPendingPaymentDateTime().FormatDate(),
				m_UpdateThread->GetPendingPaymentDateTime().FormatTime(),
				m_UpdateThread->GetPendingPaymentServices(),
				m_UpdateThread->GetPendingPaymentSum()));

		m_LastPendingPaymentCheck = wxDateTime::Now();
	}
}

void IHFrame::OnSettings(wxCommandEvent & WXUNUSED(event))
{
	wxSettingsDialog dlg(this, &m_Config);

	if (dlg.ShowModal() == wxID_OK)
	{
		wxString login, password;

		m_Config.Read(L"/Account/Login", &login, L"");
		m_Config.Read(L"/Account/PasswordClear", &password, L"");

		m_UpdateThread->SetAccountLogin(login);
		m_UpdateThread->SetAccountPassword(password);
		m_UpdateThread->ResetAccountTimer();
	}
}

void IHFrame::OnStartUpdateProcess(wxCommandEvent & WXUNUSED(event))
{
	unsigned long new_version_size = m_UpdateThread->GetNewVersionSize(), current_ptr = 0;
	wxProgressDialog progress(L"Загрузка обновления", 
							  L"Выполняется загрузка обновления для Интернет-Помощника...",
							  new_version_size, 
							  this, 
							  wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
	wxHTTP get;
	wxFile f;
	wxString update_path = wxFileName::CreateTempFileName(L"ih", &f);
	char buf[8192];
	size_t buf_read;
	
	get.SetTimeout(5);

	while (!get.Connect(L"account.astrakhan.ru"))
		::Sleep(1);

	wxInputStream *httpStream = get.GetInputStream(wxString::Format(L"/helper/InternetHelperSetup-%s.exe", 
																	m_UpdateThread->GetNewVersion()));

	if (get.GetError() == wxPROTO_NOERR)
	{
		while (current_ptr < new_version_size && progress.Update(current_ptr))
		{
			::Sleep(1);

			if (!httpStream->CanRead())
				continue;

			buf_read = httpStream->Read(buf, sizeof(buf)).LastRead();
			f.Write(buf, buf_read);

			current_ptr += buf_read;
		}
	}

	wxDELETE(httpStream);
	f.Close();

	// we got the file successfully
	//

	if (current_ptr == new_version_size)
	{
		int majorV, minorV;

		::wxRenameFile(update_path, update_path + L".exe");
		update_path += L".exe";

		::wxGetOsVersion(&majorV, &minorV);

		if (majorV >= 6)
			::ShellExecute(NULL, L"runas", update_path.c_str(), L"/S", NULL, SW_SHOWNORMAL);
		else
			::wxExecute(wxString::Format(L"%s /S", update_path));

		Close(true);
	}
}

void IHFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void IHFrame::OnClose(wxCloseEvent &event)
{
	if (event.CanVeto())
	{
		Show(false);
	}
	else
	{
		m_TaskBarIcon.RemoveIcon();
		wxFrame::OnCloseWindow(event);
	}
}

void IHFrame::OnIconize(wxIconizeEvent &event)
{
	Show(!event.Iconized());
}
