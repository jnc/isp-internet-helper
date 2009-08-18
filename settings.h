
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <wx/spinctrl.h>

class wxSettingsDialog : public wxDialog
{
public:
   wxSettingsDialog(wxWindow *parent, wxConfig *cfg);

   void OnOK(wxCommandEvent &event);

private:

	wxAuiNotebook				*m_Notebook;

	wxConfig					*m_Config;

	wxString					 m_AccountLogin,
								 m_AccountPassword;

	wxSpinCtrl					*m_PendingPaymentCheckIntervalCtrl,
								*m_PendingPaymentAdvanceIntervalCtrl;

	int							 m_PendingPaymentCheckInterval,
								 m_PendingPaymentAdvanceInterval;

	void AddAccountPage();

	DECLARE_EVENT_TABLE()
	DECLARE_DYNAMIC_CLASS(wxSettingsDialog)
	DECLARE_NO_COPY_CLASS(wxSettingsDialog)
};


#endif

