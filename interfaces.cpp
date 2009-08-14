#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "main.h"
#include "interfaces.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(wxNetworkInterfaceArray);

// wxNetworkInterface
//

wxNetworkInterface::wxNetworkInterface()
:	
	m_Index(-1),
	m_InterfaceIndex(-1),
	m_NetConnectionID(),
	m_Name(),
	m_ManagerErrorCode(-1),
	m_MAC(),
	m_DefaultInterface(false)
{

}

wxNetworkInterface::wxNetworkInterface(const wxNetworkInterface &src)
{
	m_Index = src.GetIndex();
	m_InterfaceIndex = src.GetInterfaceIndex();
	m_NetConnectionID = src.GetNetConnectionID();
	m_Name = src.GetName();
	m_ManagerErrorCode = src.GetManagerErrorCode();
	m_MAC = src.GetMAC();
	m_DefaultInterface = src.GetDefaultInterface();
}

wxNetworkInterface::~wxNetworkInterface()
{

}

void wxNetworkInterface::GetAddressing(IWbemServices *wbem_services)
{
	HRESULT hres;
	IEnumWbemClassObject *pEnumObject = NULL;
	bool error_occurred = false;

	// query WMI for interfaces details
	//

	hres = wbem_services->ExecQuery(_bstr_t(L"WQL"), 
		_bstr_t(wxString::Format(L"SELECT * FROM Win32_NetworkAdapterConfiguration WHERE Index = %d", m_Index)),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumObject);

	if (FAILED(hres))
	{
		error_occurred = true;
	}
	else
	{
		HRESULT hrEnum = WBEM_S_NO_ERROR;

		while (WBEM_S_NO_ERROR == hrEnum)
		{
			ULONG ulReturned;
			IWbemClassObject *pInstance = NULL;

			hrEnum = pEnumObject->Next(WBEM_INFINITE, 1, &pInstance, &ulReturned);

			if (SUCCEEDED(hrEnum) && ulReturned == 1 && pInstance != NULL)
			{
				VARIANT vDHCPEnabled, vIPEnabled, vIPAddress, vIPSubnet, vDefaultIPGateway, vDNSServerSearchOrder;

				VariantInit(&vDHCPEnabled);
				VariantInit(&vIPEnabled);
				VariantInit(&vIPAddress);
				VariantInit(&vIPSubnet);
				VariantInit(&vDefaultIPGateway);
				VariantInit(&vDNSServerSearchOrder);

				if (SUCCEEDED(pInstance->Get(_bstr_t(L"DHCPEnabled"), 0, &vDHCPEnabled, NULL, NULL)) &&
					SUCCEEDED(pInstance->Get(_bstr_t(L"IPEnabled"), 0, &vIPEnabled, NULL, NULL)) &&
					SUCCEEDED(pInstance->Get(_bstr_t(L"IPAddress"), 0, &vIPAddress, NULL, NULL)) &&
					SUCCEEDED(pInstance->Get(_bstr_t(L"IPSubnet"), 0, &vIPSubnet, NULL, NULL)) &&
					SUCCEEDED(pInstance->Get(_bstr_t(L"DefaultIPGateway"), 0, &vDefaultIPGateway, NULL, NULL)) &&
					SUCCEEDED(pInstance->Get(_bstr_t(L"DNSServerSearchOrder"), 0, &vDNSServerSearchOrder, NULL, NULL)))
				{
					if (V_VT(&vDHCPEnabled) == VT_BOOL &&
						V_VT(&vIPEnabled) == VT_BOOL)
					{
						m_DHCP = V_BOOL(&vDHCPEnabled) == VARIANT_TRUE;
						m_IP = V_BOOL(&vIPEnabled) == VARIANT_TRUE;

						if (V_VT(&vIPAddress) == (VT_ARRAY | VT_BSTR) &&
							V_VT(&vIPSubnet) == (VT_ARRAY | VT_BSTR))
						{
							LONG lUBoundIPAddress = 0, lUBoundIPSubnet = 0, i;
							BSTR bstrIPAddress, bstrIPSubnet;

							if (SUCCEEDED(::SafeArrayGetUBound(V_ARRAY(&vIPAddress), 1, &lUBoundIPAddress)) &&
								SUCCEEDED(::SafeArrayGetUBound(V_ARRAY(&vIPSubnet), 1, &lUBoundIPSubnet)) &&
								lUBoundIPAddress == lUBoundIPSubnet)
							{
								for (i = 0; i <= lUBoundIPAddress; i++)
								{
									if (SUCCEEDED(::SafeArrayGetElement(V_ARRAY(&vIPAddress), &i, &bstrIPAddress)) &&
										SUCCEEDED(::SafeArrayGetElement(V_ARRAY(&vIPSubnet), &i, &bstrIPSubnet)))
									{
										m_IPAddress.Add(bstrIPAddress);
										m_IPSubnet.Add(bstrIPSubnet);
									}
								}
							}
						}

						if (V_VT(&vDefaultIPGateway) == (VT_ARRAY | VT_BSTR))
						{
							LONG lUBound = 0, i;
							BSTR bstrDefaultIPGateway;

							if (SUCCEEDED(::SafeArrayGetUBound(V_ARRAY(&vDefaultIPGateway), 1, &lUBound)))
							{
								for (i = 0; i <= lUBound; i++)
								{
									if (SUCCEEDED(::SafeArrayGetElement(V_ARRAY(&vDefaultIPGateway), 
																		&i, 
																		&bstrDefaultIPGateway)))
									{
										m_Gateway.Add(bstrDefaultIPGateway);
									}
								}
							}
						}

						if (V_VT(&vDNSServerSearchOrder) == (VT_ARRAY | VT_BSTR))
						{
							LONG lUBound = 0, i;
							BSTR bstrDNSServerSearchOrder;

							if (SUCCEEDED(::SafeArrayGetUBound(V_ARRAY(&vDNSServerSearchOrder), 1, &lUBound)))
							{
								for (i = 0; i <= lUBound; i++)
								{
									if (SUCCEEDED(::SafeArrayGetElement(V_ARRAY(&vDNSServerSearchOrder), 
																				&i, 
																				&bstrDNSServerSearchOrder)))
									{
										m_DNS.Add(bstrDNSServerSearchOrder);
									}
								}
							}
						}
					}
				}

				VariantClear(&vDNSServerSearchOrder);
				VariantClear(&vDefaultIPGateway);
				VariantClear(&vIPSubnet);
				VariantClear(&vIPAddress);
				VariantClear(&vIPEnabled);
				VariantClear(&vDHCPEnabled);
			}

			if (pInstance != NULL)
				pInstance->Release();
		}

		pEnumObject->Release();
	}
}

void wxNetworkInterface::CreatePage(wxAuiNotebook *notebook)
{
	wxPanel *panel = new wxPanel(notebook, wxID_ANY);
	wxPoint current(15, 15);
	wxStaticText *text;
	wxFont bold;
	size_t i;
	wxString status;

	text = new wxStaticText(panel, wxID_ANY, L"Название:", current); 
	bold = text->GetFont();
	bold.SetWeight(wxFONTWEIGHT_BOLD);
	text->SetFont(bold);
	new wxStaticText(panel, wxID_ANY, wxString::Format(L"%s (%d)", m_NetConnectionID, m_Index), 
		wxPoint(current.x + 100, current.y));

	current.y += 20;
	text = new wxStaticText(panel, wxID_ANY, L"Модель:", current); text->SetFont(bold);
	new wxStaticText(panel, wxID_ANY, m_Name, wxPoint(current.x + 100, current.y));

	if (m_ManagerErrorCode == 0)
		status = L"включено";
	else if (m_ManagerErrorCode == 22)
		status = L"выключено";
	else
		status = wxString::Format(L"ошибка, код %d", m_ManagerErrorCode);

	current.y += 20;
	text = new wxStaticText(panel, wxID_ANY, L"Статус:", current); text->SetFont(bold);
	new wxStaticText(panel, wxID_ANY, status, wxPoint(current.x + 100, current.y));

	if (m_MAC.Length() != 0)
	{
		current.y += 20;
		text = new wxStaticText(panel, wxID_ANY, L"MAC адрес:", current); text->SetFont(bold);
		new wxStaticText(panel, wxID_ANY, m_MAC, wxPoint(current.x + 100, current.y));
	}

	current.y += 40;
	text = new wxStaticText(panel, wxID_ANY, L"DHCP:", current); text->SetFont(bold);
	new wxStaticText(panel, wxID_ANY, m_DHCP ? L"включен" : L"выключен", wxPoint(current.x + 100, current.y));

	current.y += 20;
	text = new wxStaticText(panel, wxID_ANY, L"IP рабочий:", current); text->SetFont(bold);
	new wxStaticText(panel, wxID_ANY, m_IP ? L"да" : L"нет", wxPoint(current.x + 100, current.y));

	current.y += 20;

	for (i = 0; i < m_IPAddress.Count(); i++)
	{
		current.y += 20;

		text = new wxStaticText(panel, wxID_ANY, wxString::Format(L"IP адрес (%d):", i + 1), current); 
		text->SetFont(bold);

		new wxStaticText(panel, wxID_ANY, wxString::Format(L"%s / %s", m_IPAddress[i], m_IPSubnet[i]), 
			wxPoint(current.x + 100, current.y));
	}

	current.y += 20;

	for (i = 0; i < m_Gateway.Count(); i++)
	{
		current.y += 20;

		text = new wxStaticText(panel, wxID_ANY, wxString::Format(L"Шлюз (%d):", i + 1), current); 
		text->SetFont(bold);

		new wxStaticText(panel, wxID_ANY, m_Gateway[i], wxPoint(current.x + 100, current.y));
	}

	current.y += 20;

	for (i = 0; i < m_DNS.Count(); i++)
	{
		current.y += 20;

		text = new wxStaticText(panel, wxID_ANY, wxString::Format(L"DNS (%d):", i + 1), current); 
		text->SetFont(bold);

		new wxStaticText(panel, wxID_ANY, m_DNS[i], wxPoint(current.x + 100, current.y));
	}
	
	notebook->AddPage(panel, m_NetConnectionID);
}

bool wxNetworkInterface::IsAddressWithinAddressSpace(const wxString &address)
{
	unsigned long addr = inet_addr(address.mb_str()), subnet;

	for (size_t i = 0; i < m_IPAddress.Count(); i++)
	{
		subnet = inet_addr(m_IPSubnet[i].mb_str());

		if ((inet_addr(m_IPAddress[i].mb_str()) & subnet) == (addr & subnet))
			return true;
	}

	return false;
}

// IHFrame
//

void IHFrame::EnumerateInterfaces(wxProgressDialog &progress)
{
	HRESULT hres;
	IEnumWbemClassObject *pEnumObject = NULL;

	// clear up currently enumerated interfaces
	//

	m_Interfaces.Clear();

	// query WMI for interfaces
	//

	progress.Pulse();

	hres = m_WbemServices->ExecQuery(_bstr_t(L"WQL"), 
		_bstr_t(L"SELECT * FROM Win32_NetworkAdapter"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumObject);

	if (FAILED(hres))
	{
		wxMessageDialog(this, L"Ошибка опроса сетевых интерфейсов!", L"Ошибка", wxOK | wxICON_ERROR).ShowModal();

		return;
	}

	HRESULT hrEnum = WBEM_S_NO_ERROR;

	while (WBEM_S_NO_ERROR == hrEnum)
	{
		ULONG ulReturned;
		IWbemClassObject *pInstance = NULL;

		progress.Pulse();

		hrEnum = pEnumObject->Next(WBEM_INFINITE, 1, &pInstance, &ulReturned);

		if (SUCCEEDED(hrEnum) && ulReturned == 1 && pInstance != NULL)
		{
			bool skip = false;

			// skip non-physical adapters
			//

			VARIANT vPhysicalAdapter, vAdapterTypeId;

			VariantInit(&vPhysicalAdapter);
			VariantInit(&vAdapterTypeId);

			if (SUCCEEDED(pInstance->Get(_bstr_t(L"PhysicalAdapter"), 0, &vPhysicalAdapter, NULL, NULL)) ||
				SUCCEEDED(pInstance->Get(_bstr_t(L"AdapterTypeId"), 0, &vAdapterTypeId, NULL, NULL)))
			{
				if (V_VT(&vPhysicalAdapter) == VT_BOOL)
				{
					// Windows Vista, Windows 2008 and higher
					//

					if (V_BOOL(&vPhysicalAdapter) != VARIANT_TRUE)
						skip = true;
				}
				else if (V_VT(&vAdapterTypeId) == VT_I4)
				{
					// Windows XP, Windows 2003, Windows 2000, Windows NT 4.0
					//

					if (V_I4(&vAdapterTypeId) != 0)
						skip = true;
				}
				else
				{
					// unknown Windows version
					//

					skip = true;
				}
			}
			else
			{
				// unknown Windows version
				//

				skip = true;
			}

			VariantClear(&vAdapterTypeId);
			VariantClear(&vPhysicalAdapter);

			// query interface parameters
			//

			if (!skip)
			{
				VARIANT vName, vIndex, vInterfaceIndex, vNetConnectionID, vConfigManagerErrorCode, vMACAddress;
				wxNetworkInterface *intf;

				VariantInit(&vName);
				VariantInit(&vIndex);
				VariantInit(&vInterfaceIndex);
				VariantInit(&vNetConnectionID);
				VariantInit(&vConfigManagerErrorCode);
				VariantInit(&vMACAddress);

				if (SUCCEEDED(pInstance->Get(_bstr_t(L"Index"), 0, &vIndex, NULL, NULL)) && 					
					SUCCEEDED(pInstance->Get(_bstr_t(L"Name"), 0, &vName, NULL, NULL)) && 
					SUCCEEDED(pInstance->Get(_bstr_t(L"NetConnectionID"), 0, &vNetConnectionID, NULL, NULL)) &&
					SUCCEEDED(pInstance->Get(_bstr_t(L"ConfigManagerErrorCode"), 0, &vConfigManagerErrorCode, NULL, NULL)) &&
					SUCCEEDED(pInstance->Get(_bstr_t(L"MACAddress"), 0, &vMACAddress, NULL, NULL)))
				{
					if (V_VT(&vName) == VT_BSTR && 						
						V_VT(&vIndex) == VT_I4 && 
						V_VT(&vNetConnectionID) == VT_BSTR &&
						V_VT(&vConfigManagerErrorCode) == VT_I4)
					{
						intf = new wxNetworkInterface();

						intf->SetIndex(V_I4(&vIndex));
						intf->SetName(V_BSTR(&vName));
						intf->SetNetConnectionID(V_BSTR(&vNetConnectionID));
						intf->SetManagerErrorCode(V_I4(&vConfigManagerErrorCode));

						if (V_VT(&vMACAddress) == VT_BSTR)
							intf->SetMAC(V_BSTR(&vMACAddress));

						if (SUCCEEDED(pInstance->Get(_bstr_t(L"InterfaceIndex"), 0, &vInterfaceIndex, NULL, NULL)) &&
							V_VT(&vInterfaceIndex) == VT_I4)
						{
							intf->SetInterfaceIndex(V_I4(&vInterfaceIndex));
						}

						m_Interfaces.Add(intf);
					}
				}

				VariantClear(&vMACAddress);
				VariantClear(&vConfigManagerErrorCode);
				VariantClear(&vNetConnectionID);
				VariantClear(&vInterfaceIndex);
				VariantClear(&vIndex);
				VariantClear(&vName);
			}

			if (pInstance != NULL)
				pInstance->Release();
		}
	}

	pEnumObject->Release();
}

void IHFrame::OnInterfacesSelected()
{
	wxProgressDialog progress(L"Опрос системы", L"Выполняется опрос сетевых интерфейсов...",
		100, this, wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_ELAPSED_TIME);
	size_t default_interface_page = 0;

	// query addressing
	//

	this->QueryAddressing(progress);

	// create notebook pages
	//

	m_Notebook->Freeze();

	while (m_Notebook->GetPageCount() > 0)
		m_Notebook->DeletePage(0);

	for (size_t i = 0; i < m_Interfaces.GetCount(); i++)
	{
		progress.Pulse();
		m_Interfaces[i].CreatePage(m_Notebook);

		if (m_Interfaces[i].GetDefaultInterface())
			default_interface_page = i;
	}

	m_Notebook->SetSelection(default_interface_page);

	m_Notebook->Thaw();
}
