
#ifndef __UTILS_H__
#define __UTILS_H__

class IHUtils: public wxObject
{
public:
	static wxString GetMD5(const wxString &str);
};

#endif

