#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "main.h"
#include "utils.h"

// IHFrame
//

void IHFrame::Upgrade()
{
	// update cleartext password to MD5 hashsum
	//

	wxString password_clear;

	if (m_Config.Read(L"/Account/PasswordClear", &password_clear))
	{
		UpgradeClearPassword();
	}
}

void IHFrame::UpgradeClearPassword()
{
	wxString password_clear;

	m_Config.Read(L"/Account/PasswordClear", &password_clear);
	m_Config.Write(L"/Account/Password", 
		IHUtils::GetMD5(wxString::Format(L"InternetHelper%scan'tcrackthis", password_clear)));
	m_Config.DeleteEntry(L"/Account/PasswordClear");
}