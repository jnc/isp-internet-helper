#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "defines.h"
#include "main.h"
#include "diagnosis.h"

// wxDiagnosis
//

wxDiagnosis::wxDiagnosis()
:	m_Library(NULL),
	m_IcmpCreateFile(NULL),
	m_IcmpSendEcho(NULL),
	m_IcmpCloseHandle(NULL)
{
	// try iphlpapi.dll first
	//

	m_Library = ::LoadLibrary(L"iphlpapi.dll");

	if (m_Library != NULL)
	{
		m_IcmpCreateFile = (tpIcmpCreateFile)::GetProcAddress(m_Library, "IcmpCreateFile");
		m_IcmpSendEcho = (tpIcmpSendEcho)::GetProcAddress(m_Library, "IcmpSendEcho");
		m_IcmpCloseHandle = (tpIcmpCloseHandle)::GetProcAddress(m_Library, "IcmpCloseHandle");
	}

	if (m_IcmpCreateFile == NULL ||
		m_IcmpSendEcho == NULL ||
		m_IcmpCloseHandle == NULL)
	{
		::FreeLibrary(m_Library);
		
		m_Library = NULL;
		m_IcmpCreateFile = NULL;
		m_IcmpSendEcho = NULL;
		m_IcmpCloseHandle = NULL;
	}

	// try icmp.dll otherwise (Windows 2000)
	//

	if (m_Library == NULL)
	{
		m_Library = ::LoadLibrary(L"icmp.dll");

		if (m_Library != NULL)
		{
			m_IcmpCreateFile = (tpIcmpCreateFile)::GetProcAddress(m_Library, "IcmpCreateFile");
			m_IcmpSendEcho = (tpIcmpSendEcho)::GetProcAddress(m_Library, "IcmpSendEcho");
			m_IcmpCloseHandle = (tpIcmpCloseHandle)::GetProcAddress(m_Library, "IcmpCloseHandle");
		}

		if (m_IcmpCreateFile == NULL ||
			m_IcmpSendEcho == NULL ||
			m_IcmpCloseHandle == NULL)
		{
			::FreeLibrary(m_Library);

			m_Library = NULL;
			m_IcmpCreateFile = NULL;
			m_IcmpSendEcho = NULL;
			m_IcmpCloseHandle = NULL;
		}
	}
}

wxDiagnosis::~wxDiagnosis()
{
	if (m_Library != NULL)
		::FreeLibrary(m_Library);
}

bool wxDiagnosis::Ping(const wxString &address, 
					   unsigned int size, 
					   unsigned int count, 
					   double *loss,
					   unsigned int *rtt,
					   int ttl,
					   wxString *reply_address)
{
	HANDLE hIcmpFile;
	unsigned long ipaddr = INADDR_NONE;
	unsigned long *rtt_array;
	DWORD dwRetVal = 0;
	char *SendBuffer;
	LPVOID ReplyBuffer = NULL;
	DWORD ReplySize = 0;
	size_t i, received = 0;
	IP_OPTION_INFORMATION opt, *pOpt = NULL;

	if (loss != NULL)
		*loss = 0.0;

	if (rtt != NULL)
	{
		*rtt = 0;
	}

	rtt_array = new unsigned long [count];

	if (rtt_array == NULL)
		return false;

	if (m_Library == NULL ||
		m_IcmpCreateFile == NULL ||
		m_IcmpSendEcho == NULL ||
		m_IcmpCloseHandle == NULL)
	{
		delete [] rtt_array;
		return false;
	}

	ipaddr = inet_addr(address.mb_str());

	if (ipaddr == INADDR_NONE)
	{
		delete [] rtt_array;
		return false;
	}

	if ((hIcmpFile = m_IcmpCreateFile()) == INVALID_HANDLE_VALUE)
	{
		delete [] rtt_array;
		return false;
	}

	SendBuffer = new char [size];

	if (SendBuffer == NULL)
	{
		m_IcmpCloseHandle(hIcmpFile);
		delete [] rtt_array;

		return false;
	}

	ReplySize = sizeof(ICMP_ECHO_REPLY) + size;
	ReplyBuffer = new char[ReplySize];

	if (ReplyBuffer == NULL)
	{
		m_IcmpCloseHandle(hIcmpFile);
		delete [] SendBuffer;
		delete [] rtt_array;

		return false;
	}

	if (ttl != -1)
	{
		memset(&opt, 0, sizeof(IP_OPTION_INFORMATION));
		opt.Ttl = ttl;

		pOpt = &opt;
	}

	for (i = 0; i < count; i++)
	{
		dwRetVal = m_IcmpSendEcho(hIcmpFile, ipaddr, SendBuffer, size, pOpt, ReplyBuffer, ReplySize, 1000);

		rtt_array[i] = 0xffffffff;

		if (dwRetVal != 0)
		{
			PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;

			if (pEchoReply->Status == IP_SUCCESS || pEchoReply->Status == IP_TTL_EXPIRED_TRANSIT)
			{
				received++;
				rtt_array[i] = pEchoReply->RoundTripTime;

				if (reply_address != NULL)
				{
					struct in_addr in;

					in.s_addr = pEchoReply->Address;

					*reply_address = wxString(inet_ntoa(in), wxConvFile);
				}
			}
		}
	}

	m_IcmpCloseHandle(hIcmpFile);

	if (loss != NULL)
		*loss = ((count * 1.0 - received * 1.0) / count * 1.0) * 100.0;

	if (rtt != NULL)
	{
		unsigned long rtt_sum = 0, rtt_count = 0;

		for (i = 0; i < count; i++)
		{
			if (rtt_array[i] != 0xffffffff)
			{
				rtt_sum += rtt_array[i];
				rtt_count++;
			}
		}

		*rtt = (rtt_count == 0) ? (0) : (rtt_sum / rtt_count);
	}

	delete [] rtt_array;

	return true;
}

// IHFrame
//

void IHFrame::OnDiagnosisSelected()
{
	wxProgressDialog progress(L"Диагностика Подключения К Сети Интернет", 
		L"Выполняется диагностика подключения к сети Интернет...",
		100, this, wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_ELAPSED_TIME);
	wxPanel *panel = new wxPanel(m_Notebook, wxID_ANY);
	wxPoint current(15, 15), shift(200, 0);
	wxStaticText *text;
	wxFont bold;
	wxString status;
	wxIPV4address ipv4_address;
	double loss;
	unsigned int rtt;
	wxColour colour;

	this->QueryAddressing(progress);

	// ping default gateway
	//

	progress.Pulse(L"Проверка доступности шлюза...");

	text = new wxStaticText(panel, wxID_ANY, L"Доступность шлюза:", current); 
	bold = text->GetFont();
	bold.SetWeight(wxFONTWEIGHT_BOLD);
	text->SetFont(bold);

	colour = wxTheColourDatabase->Find(L"BLUE");

	if (m_Diagnosis.Ping(m_DefaultGateway, 1000, 5, &loss, &rtt))
	{
		if (loss > 0.0)
		{
			status = wxString::Format(L"проблема (%.0f%% потерь, %d мс)", loss, rtt);

			if (loss == 100.0)
				colour = wxTheColourDatabase->Find(L"RED");
			else
				colour = wxTheColourDatabase->Find(L"BROWN");
		}
		else
		{
			status = wxString::Format(L"доступен (0%% потерь, %d мс)", rtt);
		}
	}
	else
	{
		status = L"проблема (системная ошибка)";
		colour = wxTheColourDatabase->Find(L"RED");
	}

	(new wxStaticText(panel, wxID_ANY, status, current + shift))->SetForegroundColour(colour);

	// ping DNS servers
	//

	progress.Pulse(L"Проверка доступности DNS-серверов...");

	current.y += 20;

	for (size_t i = 0; i < m_Interfaces.Count(); i++)
	{
		if (m_Interfaces[i].GetDefaultInterface())
		{
			for (size_t j = 0; j < m_Interfaces[i].GetDNSCount(); j++)
			{
				current.y += 20;

				text = new wxStaticText(panel, wxID_ANY, wxString::Format(L"Доступность DNS-сервера (%d):", j), 
					current); text->SetFont(bold);
				colour = wxTheColourDatabase->Find(L"BLUE");

				if (m_Diagnosis.Ping(m_Interfaces[i].GetDNS(j), 1000, 5, &loss, &rtt))
				{
					if (loss > 0.0)
					{
						status = wxString::Format(L"проблема (%.0f%% потерь, %d мс)", loss, rtt);

						if (loss == 100.0)
							colour = wxTheColourDatabase->Find(L"RED");
						else
							colour = wxTheColourDatabase->Find(L"BROWN");
					}
					else
						status = wxString::Format(L"доступен (0%% потерь, %d мс)", rtt);
				}
				else
				{
					status = L"проблема (системная ошибка)";
					colour = wxTheColourDatabase->Find(L"RED");
				}

				(new wxStaticText(panel, wxID_ANY, status, current + shift))->SetForegroundColour(colour);
			}
		}
	}

	// test system DNS name resolution
	//

	current.y += 20;

	progress.Pulse(L"Проверка работы DNS-серверов (системный метод)...");

	text = new wxStaticText(panel, wxID_ANY, L"Работа DNS-серверов (метод 1):", current); text->SetFont(bold);
	colour = wxTheColourDatabase->Find(L"BLUE");

	if (ipv4_address.Hostname(L"ya.ru"))
	{
		status = L"работает";
	}
	else
	{
		status = L"не работает";
		colour = wxTheColourDatabase->Find(L"RED");
	}

	(new wxStaticText(panel, wxID_ANY, status, current + shift))->SetForegroundColour(colour);

	// test Internet reachability
	//

	current.y += 40;

	progress.Pulse(L"Проверка доступности Интернет (ya.ru)...");

	text = new wxStaticText(panel, wxID_ANY, L"Доступность Интернет (ya.ru):", current); text->SetFont(bold);
	colour = wxTheColourDatabase->Find(L"BLUE");

	if (m_Diagnosis.Ping(ipv4_address.IPAddress(), 1000, 5, &loss, &rtt))
	{
		if (loss > 0.0)
		{
			status = wxString::Format(L"проблема (%.0f%% потерь, %d мс)", loss, rtt);

			if (loss == 100.0)
				colour = wxTheColourDatabase->Find(L"RED");
			else
				colour = wxTheColourDatabase->Find(L"BROWN");
		}
		else
			status = wxString::Format(L"доступен (0%% потерь, %d мс)", rtt);
	}
	else
	{
		status = L"проблема (системная ошибка)";
		colour = wxTheColourDatabase->Find(L"RED");
	}

	(new wxStaticText(panel, wxID_ANY, status, current + shift))->SetForegroundColour(colour);

	// traceroute
	//

	current.y += 40;	

	text = new wxStaticText(panel, wxID_ANY, L"Трассировка (ya.ru):", current); text->SetFont(bold);

	new wxButton(panel, ID_TRACEROUTE, L"Запустить", current + shift - wxPoint(0, 5));

	// add page
	//

	m_Notebook->Freeze();

	while (m_Notebook->GetPageCount() > 0)
		m_Notebook->DeletePage(0);

	m_Notebook->AddPage(panel, L"Диагностика");

	m_Notebook->Thaw();
}

