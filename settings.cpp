#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include <shlobj.h>

#include <wx/valgen.h>
#include <wx/filename.h>

#include "main.h"
#include "settings.h"
#include "defines.h"
#include "utils.h"

// wxSettingsDialog
//

BEGIN_EVENT_TABLE(wxSettingsDialog, wxDialog)
   EVT_BUTTON(wxID_OK, wxSettingsDialog::OnOK)
END_EVENT_TABLE()

IMPLEMENT_CLASS(wxSettingsDialog, wxDialog)

wxSettingsDialog::wxSettingsDialog(wxWindow *parent, wxConfig *cfg, int page_id)
				: wxDialog(parent, wxID_ANY, _("Настройки"), wxDefaultPosition, wxSize(550, 450),
						   wxDEFAULT_DIALOG_STYLE),
				  m_Config(cfg)
{
	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
	SetIcon(wxICON(IHIcon));

	m_Notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, this->GetClientSize(), 
		wxAUI_NB_TOP | wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);

	cfg->Read(L"/Account/Login", &m_AccountLogin, L"");
	cfg->Read(L"/Account/PendingPaymentCheckInterval", &m_PendingPaymentCheckInterval, 60);
	cfg->Read(L"/Account/PendingPaymentAdvanceInterval", &m_PendingPaymentAdvanceInterval, 3);
	AddAccountPage();

	cfg->Read(L"/Startup/AutoStart", &m_StartupAutoStart, false);
	AddStartupPage();

	m_Notebook->SetSelection(ID_SETTINGS_ACCOUNT - page_id);
}

void wxSettingsDialog::AddAccountPage()
{
	wxPanel *panel = new wxPanel(m_Notebook, wxID_ANY);
	wxPoint current(15, 15), shift(0, 20);
	wxSize psize;
	wxString status;

	psize = m_Notebook->GetClientSize();

	new wxStaticText(panel, wxID_ANY, L"Логин", current); 
	new wxTextCtrl(panel, wxID_ANY, m_AccountLogin, current + shift, wxSize(psize.GetX() - shift.x - 40, -1), 0, 
		wxTextValidator(wxFILTER_ASCII, &m_AccountLogin));

	current.y += 60;

	new wxStaticText(panel, wxID_ANY, L"Пароль", current); 
	new wxTextCtrl(panel, wxID_ANY, m_AccountPassword, current + shift, wxSize(psize.GetX() - shift.x - 40, -1), 
		wxTE_PASSWORD, wxTextValidator(wxFILTER_ASCII, &m_AccountPassword));

	current.y += 60;

	new wxStaticText(panel, wxID_ANY, L"Периодичность напоминания о необходимости оплатить услуги (в минутах)", current);
	m_PendingPaymentCheckIntervalCtrl = new wxSpinCtrl(panel, wxID_ANY, 
		wxString::Format(L"%d", m_PendingPaymentCheckInterval), current + shift, 
		wxSize(psize.GetX() - shift.x - 40, -1), wxSP_ARROW_KEYS, 60, 480, m_PendingPaymentCheckInterval);

	current.y += 60;

	new wxStaticText(panel, wxID_ANY, L"За какое количество дней напоминать о необходимости оплатить услуги", current);
	m_PendingPaymentAdvanceIntervalCtrl = new wxSpinCtrl(panel, wxID_ANY, 
		wxString::Format(L"%d", m_PendingPaymentAdvanceInterval), current + shift, 
		wxSize(psize.GetX() - shift.x - 40, -1), wxSP_ARROW_KEYS, 3, 30, m_PendingPaymentAdvanceInterval);

	current.y = psize.GetY() - 140;

	new wxStaticText(panel, wxID_ANY, 
		L"Укажите логин и пароль для доступа в Личный Кабинет. Используя эти логин и пароль,\n"
		L"Интернет-Помощник сможет проверять состояние Вашего лицевого счёта и оповещать\n"
		L"о необходимости произвести оплату услуг перед окончанием расчётного периода.", current);
	
	(new wxButton(panel, wxID_OK, _("OK"), wxPoint(psize.GetX() - 200, psize.GetY() - 65)))->SetDefault();
	new wxButton(panel, wxID_CANCEL, _("Отмена"), wxPoint(psize.GetX() - 100, psize.GetY() - 65));

	m_Notebook->AddPage(panel, L"Состояние Счёта");
}

void wxSettingsDialog::AddStartupPage()
{
	wxPanel *panel = new wxPanel(m_Notebook, wxID_ANY);
	wxPoint current(15, 15), shift(0, 20);
	wxSize psize;
	wxString status;

	psize = m_Notebook->GetClientSize();

	new wxCheckBox(panel, wxID_ANY, L"Запускать Интернет-Помощник при входе в Windows", current, wxDefaultSize, 0, 
		wxGenericValidator(&m_StartupAutoStart));

	current.y = psize.GetY() - 140;

	new wxStaticText(panel, wxID_ANY, 
		L"Следует иметь ввиду, что Интернет-Помощник запустится автоматически только\n"
		L"если войти в Windows под текущим пользователем. Для запуска Интернет-Помощника\n"
		L"под другим пользователем, выполните настройку Интернет-Помощника под ним.", current);
	
	(new wxButton(panel, wxID_OK, _("OK"), wxPoint(psize.GetX() - 200, psize.GetY() - 65)))->SetDefault();
	new wxButton(panel, wxID_CANCEL, _("Отмена"), wxPoint(psize.GetX() - 100, psize.GetY() - 65));

	m_Notebook->AddPage(panel, L"Автозапуск");
}


void wxSettingsDialog::OnOK(wxCommandEvent & WXUNUSED(event))
{
	if (Validate() && TransferDataFromWindow())
	{
		m_PendingPaymentCheckInterval = m_PendingPaymentCheckIntervalCtrl->GetValue();
		m_PendingPaymentAdvanceInterval = m_PendingPaymentAdvanceIntervalCtrl->GetValue();

		m_Config->Write(L"/Account/Login", m_AccountLogin);
		m_Config->Write(L"/Account/PendingPaymentCheckInterval", m_PendingPaymentCheckInterval);
		m_Config->Write(L"/Account/PendingPaymentAdvanceInterval", m_PendingPaymentAdvanceInterval);

		if (m_AccountPassword.Length() != 0)
		{
			m_Config->Write(L"/Account/Password", 
				IHUtils::GetMD5(wxString::Format(L"InternetHelper%scan'tcrackthis", m_AccountPassword)));
		}

		m_Config->Write(L"/Startup/AutoStart", m_StartupAutoStart);

		if (m_StartupAutoStart)
			CreateLink();
		else
			RemoveLink();

		EndModal(wxID_OK);
	}
}

void wxSettingsDialog::CreateLink()
{
	wxString app_path, link;
	wxChar *pszAppPath;
	bool rc = true;

	// get ours placement
	//

	pszAppPath = app_path.GetWriteBuf(MAX_PATH);
	pszAppPath[0] = 0;

	if ((link = GetLinkPath()).Length() == 0 ||
		::GetModuleFileName(NULL, pszAppPath, MAX_PATH) == 0)
	{
		app_path.UngetWriteBuf();

		wxMessageDialog(this, 
						L"Ошибка создания ярлыка", 
						L"Не удалось создать ярлык Интернет-Помощника в папке Автозапуска Windows.").ShowModal();

		return;
	}

	app_path.UngetWriteBuf();

	// create link, if not exists
	//

	if (::wxFileExists(link))
		return;

	HRESULT hres;
	IShellLink *isl = NULL;

	hres = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&isl);

	if (SUCCEEDED(hres) && isl != NULL)
	{
		IPersistFile *ipf = NULL;

		if (SUCCEEDED(isl->SetPath(app_path.c_str())) &&
			SUCCEEDED(isl->SetDescription(IHNAMEL)) &&
			SUCCEEDED(isl->SetArguments(L"--minimize")) &&
			SUCCEEDED(isl->SetWorkingDirectory(wxFileName(app_path).GetPath().c_str())))
		{
			hres = isl->QueryInterface(IID_IPersistFile, (LPVOID *)&ipf);

			if (SUCCEEDED(hres) && ipf != NULL)
			{
				ipf->Save(link.c_str(), true);
				ipf->Release();
			}
			else
			{
				rc = false;
			}
		}
		else
		{
			rc = false;
		}

		isl->Release();
	}
	else
	{
		rc = false;
	}

	if (!rc)
	{
		wxMessageDialog(this, 
						L"Ошибка создания ярлыка", 
						L"Не удалось создать ярлык Интернет-Помощника в папке Автозапуска Windows.").ShowModal();
	}
}

void wxSettingsDialog::RemoveLink()
{
	wxString link = GetLinkPath();

	if (::wxFileExists(link))
		::wxRemoveFile(link);
}


wxString wxSettingsDialog::GetLinkPath() const
{
	wxString startup_folder, link;
	wxChar *pszStartupFolder;

	pszStartupFolder = startup_folder.GetWriteBuf(MAX_PATH);
	pszStartupFolder[0] = 0;

	if (::SHGetSpecialFolderPath((HWND)GetHWND(), pszStartupFolder, CSIDL_STARTUP, FALSE) == FALSE)
	{
		startup_folder.UngetWriteBuf();
		return L"";
	}

	startup_folder.UngetWriteBuf();

	link  = startup_folder + L"\\";
	link += IHNAMEL;
	link += L".lnk";

	return link;
}