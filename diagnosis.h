
#ifndef __DIAGNOSIS_H__
#define __DIAGNOSIS_H__

#include <iphlpapi.h>
#include <icmpapi.h>

class wxDiagnosis: public wxObject
{
	typedef	HANDLE (WINAPI *tpIcmpCreateFile)();
	typedef DWORD (WINAPI *tpIcmpSendEcho)(HANDLE,
										   IPAddr,
										   LPVOID,
										   WORD,
										   PIP_OPTION_INFORMATION,
										   LPVOID,
										   DWORD,
										   DWORD);
	typedef BOOL (WINAPI *tpIcmpCloseHandle)(HANDLE);

	HMODULE			m_Library;

	tpIcmpCreateFile
					m_IcmpCreateFile;

	tpIcmpSendEcho	m_IcmpSendEcho;

	tpIcmpCloseHandle
					m_IcmpCloseHandle;

public:
	wxDiagnosis();
	virtual ~wxDiagnosis();

	bool Ping(const wxString &address, 
			  unsigned int size, 
			  unsigned int count, 
			  double *loss,
			  unsigned int *rtt,
			  int ttl = -1,
			  wxString *reply_address = NULL);
};

#endif