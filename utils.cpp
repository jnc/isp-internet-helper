#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include <wincrypt.h>

#include "utils.h"

// IHUtils
//

wxString IHUtils::GetMD5(const wxString &str)
{
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTHASH hHash = NULL;
	unsigned char aHash[16];
	DWORD dwHashLength = 16;
	wxString hash, digits = L"0123456789abcdef";

	if (::CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) &&
		::CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash) &&
		::CryptHashData(hHash, (const BYTE *)(const char *)str.mb_str(), str.Length(), 0) &&
		::CryptGetHashParam(hHash, HP_HASHVAL, aHash, &dwHashLength, 0))
	{
		for (DWORD i = 0; i < dwHashLength; i++)
			hash += wxString::Format(L"%c%c", digits[aHash[i] >> 4], digits[aHash[i] & 0x0F]);
	}

	if (hHash != NULL)
		::CryptDestroyHash(hHash);

	if (hCryptProv != NULL)
		::CryptReleaseContext(hCryptProv, 0);

	return hash;
}
