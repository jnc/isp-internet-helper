#include "wx/defs.h"

#include <stdio.h>

#include "wx/string.h"
#include "wx/file.h"
#include "wx/app.h"
#include "wx/log.h"
#include "wx/apptrait.h"
#include "wx/platinfo.h"
#include "wx/textfile.h"
#include "wx/tokenzr.h"

int main(int argc, char **argv)
{
#if wxUSE_UNICODE
    wxChar **wxArgv = new wxChar *[argc + 1];

    {
        int n;

        for (n = 0; n < argc; n++ )
        {
            wxMB2WXbuf warg = wxConvertMB2WX(argv[n]);
            wxArgv[n] = wxStrdup(warg);
        }

        wxArgv[n] = NULL;
    }
#else // !wxUSE_UNICODE
    #define wxArgv argv
#endif // wxUSE_UNICODE/!wxUSE_UNICODE

    wxInitializer initializer;
    if ( !initializer )
    {
        fprintf(stderr, "Failed to initialize the wxWidgets library, aborting.\n");

        return -1;
    }

    wxString src;

    if (argc != 2)
    {
        fprintf(stderr, "version.h path not specified, aborting.\n");

        return -1;
    }

    wxTextFile f(wxArgv[1]);
    wxString cur;

    if (!f.Open(wxConvLibc))
    {
        fprintf(stderr, "Failed to open version.h \"%S\", aborting.\n", wxArgv[1]);

        return -1;
    }

    unsigned long verl[4] = { 0 };

    for (cur = f.GetFirstLine(); !f.Eof(); cur = f.GetNextLine())
    {
        fprintf(stdout, "processing \"%S\"\n", cur.c_str());

        wxStringTokenizer tkz(cur, L"\t ");
        wxString ver;

        while (tkz.HasMoreTokens())
        {
            if (tkz.GetNextToken() == L"PRODUCTVER")
            {
                fprintf(stdout, "    PRODUCTVER spotted\n");

                if (tkz.HasMoreTokens())
                {
                    ver = tkz.GetNextToken();

                    fprintf(stdout, "        is %S\n", ver.c_str());

                    // parse ver
                    //

                    wxStringTokenizer tkzv(ver, L", ");

                    if (tkzv.HasMoreTokens())
                       tkzv.GetNextToken().ToULong(&verl[0]);

                    if (tkzv.HasMoreTokens())
                       tkzv.GetNextToken().ToULong(&verl[1]);

                    if (tkzv.HasMoreTokens())
                       tkzv.GetNextToken().ToULong(&verl[2]);

                    if (tkzv.HasMoreTokens())
                       tkzv.GetNextToken().ToULong(&verl[3]);

                    fprintf(stdout, "        deciphered: %d.%d.%d.%d\n", verl[0], verl[1], verl[2], verl[3]);
                }
            }
        }
    }

    if (verl[0] == 0 && verl[1] == 0 && verl[2] == 0 && verl[3] == 0)
    {
        ;
    }
    else
    {
        verl[3]++;

        f.Clear();

        f.AddLine(L"#ifndef __VERSION_H__");
        f.AddLine(L"#define __VERSION_H__");

        f.AddLine(wxString::Format(L"#define FILEVER        %d,%d,%d,%d", verl[0], verl[1], verl[2], verl[3]));
        f.AddLine(wxString::Format(L"#define PRODUCTVER     %d,%d,%d,%d", verl[0], verl[1], verl[2], verl[3]));
        f.AddLine(wxString::Format(L"#define STRFILEVER     \"%d, %d, %d, %d\\0\"", verl[0], verl[1], verl[2], verl[3]));
        f.AddLine(wxString::Format(L"#define STRPRODUCTVER  \"%d, %d, %d, %d\\0\"", verl[0], verl[1], verl[2], verl[3]));

        f.AddLine(L"#endif");

        f.Write(wxTextFileType_None, wxConvLibc);
    }
}
