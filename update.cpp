
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include <wx/sstream.h>
#include <wx/protocol/http.h>
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>

#include "defines.h"
#include "main.h"
#include "update.h"
#include "version.h"

// wxUpdateThread
//

void *wxUpdateThread::Entry()
{
	unsigned int iteration = 0;
	bool check_update, check_account;

	// load TaskDialogIndirect() function, if available
	//

	m_Library = ::LoadLibrary(L"comctl32.dll");

	m_TaskDialogIndirect = NULL;

	if (m_Library != NULL)
		m_TaskDialogIndirect = (tpTaskDialogIndirect)::GetProcAddress(m_Library, "TaskDialogIndirect");

	// check for updates periodically
	//

	ResetUpdateTimer();
	ResetAccountTimer();
	
	m_NewVersion = L"";
	m_NewVersionSize = 0;

	m_AccountBalance = 0.0;
	m_AccountCredit = 0.0;
	m_AccountBlockStatus = 0;
	m_AccountNumber = 0;

	while (!TestDestroy())
	{
		iteration++;

		if ((iteration % 5) == 0)
		{
			iteration = 0;
			check_update = false;
			check_account = false;

			m_Mutex.Lock();

			if (m_LastUpdateCheck + wxTimeSpan::Minutes(30) <= wxDateTime::Now())
			{
				m_LastUpdateCheck = wxDateTime::Now();
				check_update = true;
			}

			if (m_LastAccountCheck + wxTimeSpan::Minutes(5) <= wxDateTime::Now())
			{
				m_LastAccountCheck = wxDateTime::Now();
				check_account = true;
			}

			m_Mutex.Unlock();

			if (check_update)
				CheckForUpdates();

			if (check_account)
				CheckAccountState();
		}

		::Sleep(200);
	}

	// free library, if needed
	//

	if (m_Library != NULL)
		::FreeLibrary(m_Library);

	return NULL;
}

void wxUpdateThread::CheckAccountState()
{
	// we don't want to abuse server with empty logins
	//

	if (m_AccountLogin.Length() == 0 ||
		m_AccountPassword.Length() == 0)
	{
		m_AccountError = L"Не указаны логин и пароль в настройках.";

		wxPostEvent(wxGetApp().GetMainFrame(), 
			wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_ACCOUNT_INFORMATION_CHANGED));

		return;
	}

	// query server for account data
	//

	wxHTTP get;

	get.SetHeader(L"Content-Type", L"text/html; charset=utf-8");
	get.SetTimeout(2);

	while (!get.Connect(L"account.astrakhan.ru") && !TestDestroy())
		::Sleep(200);

	wxInputStream *httpStream;
	wxString request;

	m_Mutex.Lock();
	request = wxString::Format(L"/helper/check-account.php?login=%s&password=%s",
		m_AccountLogin,
		m_AccountPassword);
	m_Mutex.Unlock();

	httpStream = get.GetInputStream(request);

	if (get.GetError() == wxPROTO_NOERR)
	{
		wxXmlDocument doc(*httpStream);

		m_AccountError = L"";
		m_PendingPaymentDateTime = wxDateTime::Now();
		m_PendingPaymentServices = L"";
		m_PendingPaymentSum = 0.0;

		if (doc.GetRoot() != NULL && doc.GetRoot()->GetName() == L"response")
		{
			wxXmlNode *child = doc.GetRoot()->GetChildren();

			while (child != NULL)
			{
				wxMutexLocker lck(m_Mutex);

				if (child->GetName() == L"error")
				{
					m_AccountError = child->GetNodeContent();
					break;
				}

				if (child->GetName() == L"balance")
					child->GetNodeContent().ToDouble(&m_AccountBalance);
				else if (child->GetName() == L"credit")
					child->GetNodeContent().ToDouble(&m_AccountCredit);
				else if (child->GetName() == L"account")
					child->GetNodeContent().ToULong(&m_AccountNumber);
				else if (child->GetName() == L"is-blocked")
					child->GetNodeContent().ToULong(&m_AccountBlockStatus);
				else if (child->GetName() == L"disc-date")
				{
					unsigned long dttm;
					time_t dttm_t;

					child->GetNodeContent().ToULong(&dttm); dttm_t = dttm - 1;
					m_PendingPaymentDateTime = wxDateTime(dttm_t);
				}
				else if (child->GetName() == L"disc-cost")
					child->GetNodeContent().ToDouble(&m_PendingPaymentSum);
				else if (child->GetName() == L"disc-service")
					m_PendingPaymentServices = child->GetNodeContent();

				child = child->GetNext();
			}
		}
		else
		{
			m_AccountError = L"Ошибочный XML со стороны сервера.";
		}
	}

	wxDELETE(httpStream);
	get.Close();

	wxPostEvent(wxGetApp().GetMainFrame(), wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_ACCOUNT_INFORMATION_CHANGED));
}

void wxUpdateThread::CheckForUpdates()
{
	unsigned long current_version[4] = { PRODUCTVER };
	unsigned long new_version[4] = { 0, 0, 0, 0 };
	wxHTTP get;

	get.SetHeader(L"Content-Type", L"text/html; charset=utf-8");
	get.SetTimeout(2);

	while (!get.Connect(L"account.astrakhan.ru") && !TestDestroy())
		::Sleep(200);

	wxInputStream *httpStream;

	httpStream = get.GetInputStream(wxString::Format(L"/helper/check-version.php?ver=%d.%d.%d.%d",
													 current_version[0],
													 current_version[1],
													 current_version[2],
													 current_version[3]));

	if (get.GetError() == wxPROTO_NOERR)
	{
		wxXmlDocument doc(*httpStream);
		wxString ver, size;
		unsigned long size_l;

		if (doc.GetRoot() != NULL && doc.GetRoot()->GetName() == L"response")
		{
			wxXmlNode *child = doc.GetRoot()->GetChildren();

			while (child != NULL)
			{
				if (child->GetName() == L"version")
					ver = child->GetNodeContent();
				else if (child->GetName() == L"size")
					size = child->GetNodeContent();

				child = child->GetNext();
			}
		}

		// extract new version into machine-readable format
		//

		wxStringTokenizer tkz(ver, L".");

		if (tkz.HasMoreTokens())
			tkz.GetNextToken().ToULong(&(new_version[0]));
		if (tkz.HasMoreTokens())
			tkz.GetNextToken().ToULong(&(new_version[1]));
		if (tkz.HasMoreTokens())
			tkz.GetNextToken().ToULong(&(new_version[2]));
		if (tkz.HasMoreTokens())
			tkz.GetNextToken().ToULong(&(new_version[3]));

		// check if it's new version and start update process if needed
		//

		if (new_version[0] > current_version[0] ||
			new_version[1] > current_version[1] ||
			new_version[2] > current_version[2] ||
			new_version[3] > current_version[3])
		{
			if (size.ToULong(&size_l) && AllowUpdate(ver))
				StartUpdateProcess(ver, size_l);
		}
	}

	wxDELETE(httpStream);
	get.Close();
}

void wxUpdateThread::StartUpdateProcess(const wxString &new_version, 
										const unsigned long new_version_size)
{
	wxMutexLocker lck(m_Mutex);

	m_NewVersion = new_version;
	m_NewVersionSize = new_version_size;

	wxPostEvent(wxGetApp().GetMainFrame(), wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_START_UPDATE_PROCESS));
}

HRESULT CALLBACK TaskDialogCallbackProc(HWND hwnd,
										UINT uNotification,
										WPARAM wParam,
										LPARAM lParam,
										LONG_PTR dwRefData)
{
   if (TDN_CREATED == uNotification)
	   ::SendMessage(hwnd, TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, IDOK, 1);

   return S_OK;
}


bool wxUpdateThread::AllowUpdate(const wxString &new_version)
{
	bool ret = false;

	if (m_TaskDialogIndirect != NULL)
	{
		int nButtonPressed                  = 0;
		TASKDIALOGCONFIG config             = {0};
		const TASKDIALOG_BUTTON buttons[]   = { 
			{ IDOK,			L"Запустить" },
			{ IDCANCEL,		L"Отмена" }
		};
		wxString inst = wxString::Format(L"Новая Версия %s", new_version);

		config.cbSize                       = sizeof(config);
		config.hInstance                    = ::GetModuleHandle(NULL);
		config.pszWindowTitle				= L"Обновление";
		config.pszMainIcon                  = MAKEINTRESOURCE(TD_SHIELD_ICON);
		config.pszMainInstruction           = inst.c_str();
		config.pszContent                   = L"Доступна новая версия Интернет-Помощника. Загрузить и запустить обновление?";
		config.pButtons                     = buttons;
		config.cButtons                     = ARRAYSIZE(buttons);
		config.pfCallback					= TaskDialogCallbackProc;

		if (SUCCEEDED(m_TaskDialogIndirect(&config, &nButtonPressed, NULL, NULL)) && nButtonPressed == IDOK)
			ret = true;
	}
	else
	{
		if (wxMessageDialog(NULL, 
							L"Доступна новая версия Интернет-Помощника. Загрузить и запустить обновление?", 
							L"Обновление",
							wxYES_NO).ShowModal() == wxID_YES)
		{
			ret = true;
		}							
	}

	return ret;
}

wxString wxUpdateThread::GetNewVersion()
{
	wxMutexLocker lck(m_Mutex);
	wxString ret = m_NewVersion;

	return ret;
}

wxString wxUpdateThread::GetAccountError()
{
	wxMutexLocker lck(m_Mutex);
	wxString ret = m_AccountError;

	return ret;
}

double wxUpdateThread::GetAccountBalance()
{
	wxMutexLocker lck(m_Mutex);
	double ret = m_AccountBalance;

	return ret;
}

wxDateTime wxUpdateThread::GetPendingPaymentDateTime()
{
	wxMutexLocker lck(m_Mutex);
	wxDateTime ret = m_PendingPaymentDateTime;

	return ret;
}

double wxUpdateThread::GetPendingPaymentSum()
{
	wxMutexLocker lck(m_Mutex);
	double ret = m_PendingPaymentSum;

	return ret;
}

wxString wxUpdateThread::GetPendingPaymentServices()
{
	wxMutexLocker lck(m_Mutex);
	wxString ret = m_PendingPaymentServices;

	return ret;
}

double wxUpdateThread::GetAccountCredit()
{
	wxMutexLocker lck(m_Mutex);
	double ret = m_AccountCredit;

	return ret;
}

void wxUpdateThread::ResetAccountTimer()
{
	wxMutexLocker lck(m_Mutex);

	m_LastAccountCheck = wxDateTime::Now() - wxTimeSpan::Minutes(60);
}

void wxUpdateThread::ResetUpdateTimer()
{
	wxMutexLocker lck(m_Mutex);

	m_LastUpdateCheck = wxDateTime::Now() - wxTimeSpan::Minutes(60);
}
