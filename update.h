
#ifndef __UPDATE_H__
#define __UPDATE_H__

class wxUpdateThread: public wxThread
{
	typedef HRESULT (WINAPI *tpTaskDialogIndirect)(const TASKDIALOGCONFIG *,
												   int *,
												   int *pnRadioButton,
												   BOOL *pfVerificationFlagChecked);
	
	tpTaskDialogIndirect		m_TaskDialogIndirect;

	HMODULE						m_Library;

	wxDateTime					m_LastUpdateCheck,
								m_LastAccountCheck;

	wxMutex						m_Mutex;

	wxString					m_NewVersion;
	unsigned long				m_NewVersionSize;

	wxString					m_AccountLogin,
								m_AccountPassword,
								m_AccountError;

	double						m_AccountBalance,
								m_AccountCredit;

	unsigned long				m_AccountNumber,
								m_AccountBlockStatus;

	wxDateTime					m_PendingPaymentDateTime;
	double						m_PendingPaymentSum;
	wxString					m_PendingPaymentServices;

protected:

	virtual void *Entry();

	bool AllowUpdate(const wxString &new_version);
	void StartUpdateProcess(const wxString &new_version,
							const unsigned long new_version_size);

	void CheckForUpdates();
	void CheckAccountState();

public:

	wxString GetNewVersion();
	unsigned long GetNewVersionSize() const { return m_NewVersionSize; };

	wxString GetAccountError();
	double GetAccountBalance();
	double GetAccountCredit();
	unsigned long GetAccountNumber() const { return m_AccountNumber; };
	unsigned long GetAccountBlockStatus() const { return m_AccountBlockStatus; };

	wxDateTime GetPendingPaymentDateTime();
	double GetPendingPaymentSum();
	wxString GetPendingPaymentServices();

	void SetAccountLogin(const wxString &login) { wxMutexLocker lck(m_Mutex); m_AccountLogin = login; };
	void SetAccountPassword(const wxString &password) { wxMutexLocker lck(m_Mutex); m_AccountPassword = password; };

	void ResetUpdateTimer();
	void ResetAccountTimer();
};

#endif
