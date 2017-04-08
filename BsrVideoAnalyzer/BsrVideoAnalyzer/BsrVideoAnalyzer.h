
// BsrVideoAnalyzer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"        // main symbols

// BsrVideoAnalyzerApp:
// See BsrVideoAnalyzer.cpp for the implementation of this class
//

class BsrVideoAnalyzerApp : public CWinApp
{
public:
    BsrVideoAnalyzerApp();

// Overrides
public:
    virtual BOOL InitInstance();

// Implementation

    DECLARE_MESSAGE_MAP()

    ULONG_PTR m_gdiplusToken;
    virtual int ExitInstance();


};

extern BsrVideoAnalyzerApp theApp;