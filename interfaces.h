
#ifndef __INTERFACES_H__
#define __INTERFACES_H__

#include <wx/dynarray.h>
#include <wx/arrstr.h>

#include <comutil.h>

class wxNetworkInterface;

WX_DECLARE_OBJARRAY(wxNetworkInterface, wxNetworkInterfaceArray);

class wxNetworkInterface: public wxObject
{
	int					 m_Index,
						 m_InterfaceIndex,
						 m_ManagerErrorCode;

	wxString			 m_NetConnectionID,
						 m_Name;

	wxString			 m_MAC;

	bool				 m_DHCP,
						 m_IP,
						 m_DefaultInterface;

	wxArrayString		 m_IPAddress,
						 m_IPSubnet,
						 m_DNS,
						 m_Gateway;



public:
	wxNetworkInterface();
	wxNetworkInterface(const wxNetworkInterface &src);
	virtual ~wxNetworkInterface();

	void SetIndex(const int index) { m_Index = index; };
	int GetIndex() const { return m_Index; };

	void SetInterfaceIndex(const int index) { m_InterfaceIndex = index; };
	int GetInterfaceIndex() const { return m_InterfaceIndex; };

	void SetNetConnectionID(const wxString &id) { m_NetConnectionID = id; };
	const wxString & GetNetConnectionID() const { return m_NetConnectionID; };

	void SetName(const wxString &name) { m_Name = name; };
	const wxString & GetName() const { return m_Name; };

	void SetManagerErrorCode(const int code) { m_ManagerErrorCode = code; };
	int GetManagerErrorCode() const { return m_ManagerErrorCode; };

	void SetMAC(const wxString &mac) { m_MAC = mac; };
	const wxString & GetMAC() const { return m_MAC; };

	void SetDefaultInterface(const bool default) { m_DefaultInterface = default; };
	bool GetDefaultInterface() const { return m_DefaultInterface; };

	size_t GetDNSCount() const { return m_DNS.GetCount(); };
	const wxString GetDNS(size_t item) const { if (item < m_DNS.Count()) return m_DNS[item]; else return L""; };

	bool IsAddressWithinAddressSpace(const wxString &address);

	void GetAddressing(IWbemServices *wbem_services);
	void CreatePage(wxAuiNotebook *notebook);

};

#endif
