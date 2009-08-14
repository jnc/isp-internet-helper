#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "../sample.xpm"
#endif

#include "main.h"
#include "routes.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(wxRoutingEntryArray);

// wxRoutingEntry
//

wxRoutingEntry::wxRoutingEntry()
:	
	m_InterfaceIndex(-1),
	m_Metric(-1),
	m_Type(-1),
	m_Destination(),
	m_Mask(),
	m_NextHop()
{

}

wxRoutingEntry::wxRoutingEntry(const wxRoutingEntry &src)
{
	m_InterfaceIndex = src.GetInterfaceIndex();
	m_Metric = src.GetMetric();
	m_Type = src.GetType();

	m_Destination = src.GetDestination();
	m_Mask = src.GetMask();
	m_NextHop = src.GetNextHop();
}

wxRoutingEntry::~wxRoutingEntry()
{

}

// IHFrame
//

void IHFrame::EnumerateRoutes(wxProgressDialog &progress)
{
	HRESULT hres;
	IEnumWbemClassObject *pEnumObject = NULL;

	// clear up currently enumerated routes
	//

	m_RoutingTable.Clear();

	// query WMI for interfaces
	//

	progress.Pulse();

	hres = m_WbemServices->ExecQuery(_bstr_t(L"WQL"), 
		_bstr_t(L"SELECT * FROM Win32_IP4RouteTable"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumObject);

	if (FAILED(hres))
	{
		wxMessageDialog(this, L"Ошибка опроса таблицы маршрутизации!", L"Ошибка", wxOK | wxICON_ERROR).ShowModal();

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
			// query routing entry parameters
			//

			if (true)
			{
				VARIANT vDestination, vInterfaceIndex, vMask, vMetric1, vNextHop, vType;

				VariantInit(&vDestination);
				VariantInit(&vInterfaceIndex);
				VariantInit(&vMask);
				VariantInit(&vMetric1);
				VariantInit(&vNextHop);
				VariantInit(&vType);
				
				if (SUCCEEDED(pInstance->Get(_bstr_t(L"Destination"), 0, &vDestination, NULL, NULL)) && 
					SUCCEEDED(pInstance->Get(_bstr_t(L"InterfaceIndex"), 0, &vInterfaceIndex, NULL, NULL)) && 
					SUCCEEDED(pInstance->Get(_bstr_t(L"Mask"), 0, &vMask, NULL, NULL)) &&
					SUCCEEDED(pInstance->Get(_bstr_t(L"Metric1"), 0, &vMetric1, NULL, NULL)) &&
					SUCCEEDED(pInstance->Get(_bstr_t(L"NextHop"), 0, &vNextHop, NULL, NULL)) &&
					SUCCEEDED(pInstance->Get(_bstr_t(L"Type"), 0, &vType, NULL, NULL)))
				{
					if (V_VT(&vDestination) == VT_BSTR && 
						V_VT(&vInterfaceIndex) == VT_I4 && 
						V_VT(&vMask) == VT_BSTR &&
						V_VT(&vMetric1) == VT_I4 &&
						V_VT(&vNextHop) == VT_BSTR &&
						V_VT(&vType) == VT_I4)
					{
						wxRoutingEntry *rt = new wxRoutingEntry();

						rt->SetDestination(V_BSTR(&vDestination));
						rt->SetInterfaceIndex(V_I4(&vInterfaceIndex));
						rt->SetMask(V_BSTR(&vMask));
						rt->SetMetric(V_I4(&vMetric1));
						rt->SetNextHop(V_BSTR(&vNextHop));
						rt->SetType(V_I4(&vType));

						m_RoutingTable.Add(rt);
					}
				}

				VariantClear(&vType);
				VariantClear(&vNextHop);
				VariantClear(&vMetric1);
				VariantClear(&vMask);
				VariantClear(&vInterfaceIndex);
				VariantClear(&vDestination);
			}

			if (pInstance != NULL)
				pInstance->Release();
		}
	}

	pEnumObject->Release();
}

int IHFrame::GetDefaultInterface(wxString &nexthop) const
{
	size_t i, j;
	int best_interface_index = -1, best_routing_metric = 0x7FFFFFFF;
	wxString best_route;

	for (i = 0; i < m_RoutingTable.Count(); i++)
	{
		if ((inet_addr(m_RoutingTable[i].GetDestination().mb_str()) & 
			 inet_addr(m_RoutingTable[i].GetMask().mb_str())) == 0 &&
			m_RoutingTable[i].GetType() == 0x4 /* indirect */)
		{
			for (j = 0; j < m_Interfaces.Count(); j++)
			{
				if (m_Interfaces[j].GetInterfaceIndex() != -1)
				{
					// Windows Vista, Windows 2008 and higher
					//

					if (m_RoutingTable[i].GetInterfaceIndex() == m_Interfaces[j].GetInterfaceIndex())
					{
						if (m_RoutingTable[i].GetMetric() < best_routing_metric)
						{
							best_interface_index = j;
							best_route = m_RoutingTable[i].GetNextHop();
						}
					}
				}
				else
				{
					// Windows XP, Windows 2003, Windows 2000, Windows NT 4.0
					//

					if (m_Interfaces[j].IsAddressWithinAddressSpace(m_RoutingTable[i].GetNextHop()))
					{
						if (m_RoutingTable[i].GetMetric() < best_routing_metric)
						{
							best_interface_index = j;
							best_route = m_RoutingTable[i].GetNextHop();
						}
					}
				}
			}
		}
	}

	if (best_interface_index != -1)
	{
		m_Interfaces[best_interface_index].SetDefaultInterface(true);
		nexthop = best_route;
	}

	return best_interface_index;
}
