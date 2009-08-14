#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "main.h"
#include "account.h"

// wxSettingsDialog
//

BEGIN_EVENT_TABLE(wxSettingsDialog, wxDialog)
   EVT_BUTTON(wxID_OK, wxSettingsDialog::OnOK)
END_EVENT_TABLE()

IMPLEMENT_CLASS(wxSettingsDialog, wxDialog)

wxSettingsDialog::wxSettingsDialog(wxWindow *parent, wxConfig *cfg)
				: wxDialog(parent, wxID_ANY, _("Настройки"), wxDefaultPosition, wxSize(550, 450),
						   wxDEFAULT_DIALOG_STYLE),
				  m_Config(cfg)
{
	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);
	SetIcon(wxICON(IHIcon));

	m_Notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, this->GetClientSize(), 
		wxAUI_NB_TOP | wxAUI_NB_WINDOWLIST_BUTTON | wxNO_BORDER);

	cfg->Read(L"/Account/Login", &m_AccountLogin, L"");
	cfg->Read(L"/Account/PasswordClear", &m_AccountPassword, L"");
	cfg->Read(L"/Account/PendingPaymentCheckInterval", &m_PendingPaymentCheckInterval, 60);
	cfg->Read(L"/Account/PendingPaymentAdvanceInterval", &m_PendingPaymentAdvanceInterval, 3);

	AddAccountPage();
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

void wxSettingsDialog::OnOK(wxCommandEvent & WXUNUSED(event))
{
	if (Validate() && TransferDataFromWindow())
	{
		m_PendingPaymentCheckInterval = m_PendingPaymentCheckIntervalCtrl->GetValue();
		m_PendingPaymentAdvanceInterval = m_PendingPaymentAdvanceIntervalCtrl->GetValue();

		m_Config->Write(L"/Account/Login", m_AccountLogin);
		m_Config->Write(L"/Account/PasswordClear", m_AccountPassword);
		m_Config->Write(L"/Account/PendingPaymentCheckInterval", m_PendingPaymentCheckInterval);
		m_Config->Write(L"/Account/PendingPaymentAdvanceInterval", m_PendingPaymentAdvanceInterval);

		EndModal(wxID_OK);
	}
}


// IHFrame
//

void IHFrame::OnAccountSelected()
{
	wxPanel *panel = new wxPanel(m_Notebook, wxID_ANY);
	wxPoint current(15, 15);
	wxPoint shift(100, 0);
	wxStaticText *text;
	wxFont bold;

	// construct root page
	//

	if (m_UpdateThread->GetAccountError().Length() == 0)
	{
		text = new wxStaticText(panel, wxID_ANY, L"Номер счёта:", current);
		bold = text->GetFont();
		bold.SetWeight(wxFONTWEIGHT_BOLD);
		text->SetFont(bold);

		new wxStaticText(panel, 
						 wxID_ANY, 
						 wxString::Format(L"%d", m_UpdateThread->GetAccountNumber()), 
						 current + shift);

		current.y += 20;

		text = new wxStaticText(panel, wxID_ANY, L"Баланс:", current); text->SetFont(bold);
		new wxStaticText(panel, 
						 wxID_ANY, 
						 wxString::Format(L"%.2f", m_UpdateThread->GetAccountBalance()), 
						 current + shift);

		current.y += 20;

		text = new wxStaticText(panel, wxID_ANY, L"Кредит:", current); text->SetFont(bold);
		new wxStaticText(panel, 
						 wxID_ANY, 
						 wxString::Format(L"%.2f", m_UpdateThread->GetAccountCredit()), 
						 current + shift);

		current.y += 20;

		text = new wxStaticText(panel, wxID_ANY, L"Состояние:", current); text->SetFont(bold);
		new wxStaticText(panel, 
						 wxID_ANY, 
						 (m_UpdateThread->GetAccountBlockStatus() == 0) ? L"разблокирован" : L"заблокирован",
						 current + shift);
	}
	else
	{
		text = new wxStaticText(panel, wxID_ANY, L"Ошибка:", current);
		bold = text->GetFont();
		bold.SetWeight(wxFONTWEIGHT_BOLD);
		text->SetFont(bold);

		new wxStaticText(panel, 
						 wxID_ANY, 
						 m_UpdateThread->GetAccountError(),
						 current + shift);
	}

	// add page
	//

	m_Notebook->Freeze();

	while (m_Notebook->GetPageCount() > 0)
		m_Notebook->DeletePage(0);

	m_Notebook->AddPage(panel, L"Состояние Счёта");

	m_Notebook->Thaw();
}
