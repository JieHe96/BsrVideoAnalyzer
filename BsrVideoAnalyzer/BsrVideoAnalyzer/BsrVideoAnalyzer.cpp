
// BsrVideoAnalyzer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "BsrVideoAnalyzer.h"
#include "BsrVideoAnalyzerDlg.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
//#define new DEBUG_NEW
#endif

#define SDL_DLL_FILENAME "sdl2.xxx"

int GetSDL2Dll(int nId, char *path, char *name, char *ctype, char *moduleName)
{
	if (!path || !name || !ctype || !nId) return FALSE;

	char rcFileName[512];
	char rcFilePath[1024];

	ZeroMemory(rcFileName, 512);
	ZeroMemory(rcFilePath, 1024);
	strncpy(rcFileName, name, strlen(name));

	HMODULE hInstance = GetModuleHandle(moduleName);
	HRSRC     hRes = NULL;  //resource handle 
	HGLOBAL hgpt = NULL; //resource pointer 
	LPVOID    lpBuff = NULL; //resource buffer pointer 
	DWORD   rcSize = 0;      //resource size 
	DWORD   dwByte;   //byte size had been write 
	HANDLE   hFile = INVALID_HANDLE_VALUE; //file to write 

	hRes = ::FindResource(hInstance, MAKEINTRESOURCE(nId), ctype);
	if (NULL == hRes)
	{
		return FALSE;
	}

	hgpt = ::LoadResource(hInstance, hRes);
	if (NULL == hgpt)
	{
		return FALSE;
	}

	rcSize = ::SizeofResource(hInstance, hRes);
	lpBuff = ::LockResource(hgpt);
	//now i will read the resource and write it to an file
	strcat(rcFilePath, path); // 不同情况自己指定
	strcat(rcFilePath, rcFileName);
	hFile = CreateFile(rcFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DWORD err = GetLastError();
		CloseHandle(hFile);
		return FALSE;
	}

	WriteFile(hFile, lpBuff, rcSize, &dwByte, NULL);
	CloseHandle(hFile);
	if (dwByte != rcSize)
		return FALSE;

	return TRUE;
}

int GetChkDll()
{
	typedef int(*GetDll_Func)(int nId, char *path, char *name, char *ctype, char *);
	typedef int(*GetDirInfo_Func)(int y4, GetDll_Func pFuncGetDll);

	char cTempDir[512]; GetTempPath(512, cTempDir);
	GetSDL2Dll(IDR_DLL_SDL2, cTempDir, "chk2.xxx", "DLL_FILE", NULL);
	sprintf(cTempDir, "%schk2.xxx", cTempDir);
	HMODULE hSDL_DLL = LoadLibrary(_T(cTempDir));

	GetDirInfo_Func GetDirInfo = (GetDirInfo_Func)GetProcAddress(hSDL_DLL, "GetDirInfo");

	int ret = GetDirInfo(2099, &GetSDL2Dll);

	if (hSDL_DLL)
	{
		FreeLibrary(hSDL_DLL);
		hSDL_DLL = NULL;
	}

	DeleteFile(cTempDir);

	return ret;
}

// BsrVideoAnalyzerApp

BEGIN_MESSAGE_MAP(BsrVideoAnalyzerApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// BsrVideoAnalyzerApp construction

BsrVideoAnalyzerApp::BsrVideoAnalyzerApp()
{
    // support Restart Manager
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
	//GetSDL2Dll(IDR_DLL_SDL2, ".\\", "sdl2.xxx", "DLL_FILE");
	GetChkDll();
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}


// The one and only BsrVideoAnalyzerApp object

BsrVideoAnalyzerApp theApp;


// BsrVideoAnalyzerApp initialization

BOOL BsrVideoAnalyzerApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    GdiplusStartupInput gdiplusStartupInput;
    //ULONG_PTR gdiplusToken;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

    CWinApp::InitInstance();


    AfxEnableControlContainer();

    // Create the shell manager, in case the dialog contains
    // any shell tree view or shell list view controls.
    CShellManager *pShellManager = new CShellManager;

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    BsrVideoAnalyzerDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with OK
    }
    else if (nResponse == IDCANCEL)
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with Cancel
    }

    // Delete the shell manager created above.
    if (pShellManager != NULL)
    {
        delete pShellManager;
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}

int BsrVideoAnalyzerApp::ExitInstance()
{
    // TODO: Add your specialized code here and/or call the base class
    GdiplusShutdown(m_gdiplusToken); // ??

	
	char cTempDir[512]; GetTempPath(512, cTempDir);
	sprintf(cTempDir, "%s%s", cTempDir, SDL_DLL_FILENAME);
	DeleteFile(cTempDir);

    return CWinApp::ExitInstance();
}