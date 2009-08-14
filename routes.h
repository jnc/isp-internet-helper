
#ifndef __ROUTES_H__
#define __ROUTES_H__

#include <wx/dynarray.h>
#include <wx/arrstr.h>

#include <comutil.h>

class wxRoutingEntry;

WX_DECLARE_OBJARRAY(wxRoutingEntry, wxRoutingEntryArray);

class wxRoutingEntry: public wxObject
{
	int					 m_InterfaceIndex,
						 m_Metric,
						 m_Type;

	wxString			 m_Destination,
						 m_Mask,
						 m_NextHop;

public:
	wxRoutingEntry();
	wxRoutingEntry(const wxRoutingEntry &src);
	virtual ~wxRoutingEntry();

	void SetInterfaceIndex(const int index) { m_InterfaceIndex = index; };
	int GetInterfaceIndex() const { return m_InterfaceIndex; };

	void SetMetric(const int metric) { m_Metric = metric; };
	int GetMetric() const { return m_Metric; };

	void SetType(const int type) { m_Type = type; };
	int GetType() const { return m_Type; };

	void SetDestination(const wxString &destination) { m_Destination = destination; }
	const wxString & GetDestination() const { return m_Destination; };

	void SetMask(const wxString &mask) { m_Mask = mask; };
	const wxString & GetMask() const { return m_Mask; };

	void SetNextHop(const wxString &nexthop) { m_NextHop = nexthop; };
	const wxString & GetNextHop() const { return m_NextHop; };
};

#endif
