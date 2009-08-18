#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "main.h"
#include "account.h"

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

		if (m_UpdateThread->GetPendingPaymentServices().Length() != 0)
		{
			current.y += 40;

			text = new wxStaticText(panel, wxID_ANY, L"Платёж до:", current); text->SetFont(bold);
			new wxStaticText(panel, 
							 wxID_ANY, 
							 wxString::Format(L"%s, %s", 
											  m_UpdateThread->GetPendingPaymentDateTime().FormatDate(),
											  m_UpdateThread->GetPendingPaymentDateTime().FormatTime()),
							 current + shift);

			current.y += 20;

			new wxStaticText(panel, 
							 wxID_ANY, 
							 m_UpdateThread->GetPendingPaymentServices(),
							 current + shift);

			current.y += 20;

			new wxStaticText(panel, 
							 wxID_ANY, 
							 wxString::Format(L"%.2f руб.", m_UpdateThread->GetPendingPaymentSum()),
							 current + shift);
		}
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
