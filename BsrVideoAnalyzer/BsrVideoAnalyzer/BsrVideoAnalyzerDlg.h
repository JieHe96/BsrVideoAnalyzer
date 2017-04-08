
// BsrVideoAnalyzerDlg.h : header file
//
#include "resource.h"
#include "stdafx.h"

#include "NalParse.h"
#include "hexeditctrl.h"
#include "XHeaderCtrl.h"
#include "PlayDlg.h"

#include <vector>
#include "afxwin.h"
using std::vector;

#pragma once

#define APP_NAM "Video Stream Analyzer"

#define MAX_URL_LENGTH 512



class CNewListCtrl : public CListCtrl
{
public:
	CNewListCtrl() { m_bHeaderIsSubclassed = FALSE; };
	virtual ~CNewListCtrl() {};

public:
	void	SubclassHeaderControl()
	{
		if (m_bHeaderIsSubclassed)
			return;

		// if the list control has a header control window, then
		// subclass it

		// Thanks to Alberto Gattegno and Alon Peleg  and their article
		// "A Multiline Header Control Inside a CListCtrl" for easy way
		// to determine if the header control exists.

		CHeaderCtrl* pHeader = GetHeaderCtrl();
		if (pHeader)
		{
			VERIFY(m_HeaderCtrl.SubclassWindow(pHeader->m_hWnd));
			m_bHeaderIsSubclassed = TRUE;
			m_HeaderCtrl.SetListCtrl(this);
		}
	}

	BOOL			m_bHeaderIsSubclassed;

public:
	virtual void PreSubclassWindow();

public:
	CXHeaderCtrl	m_HeaderCtrl;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()

};

// BsrVideoAnalyzerDlg dialog
class BsrVideoAnalyzerDlg : public CDialog
{
// Construction
public:
    BsrVideoAnalyzerDlg(CWnd* pParent = NULL);    // standard constructor
	~BsrVideoAnalyzerDlg();
// Dialog Data
    enum { IDD = IDD_BSRVIDEOANALYZER_DIALOG };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // our own...
public:
    void SystemClear();

    int ShowNLInfo(NALU_t* nalu);
// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
	CNewListCtrl m_h264NalList;
    CHexEdit m_edHexInfo;   // 显示十六进制
	CTreeCtrl m_cTree;
    CPlayDlg* m_pPlayDlg;   // 播放窗口

    void OnBnClickedH264InputurlOpen();
	void UpdateTitle(char *);
	void ShowParsingProgess(uint64_t size0, uint64_t size1, uint64_t len=0);

	void ExpandTree(HTREEITEM hItem)
	{
//only expand 2 levels
#define LEVEL2_CHILD(item)  m_cTree.GetParentItem((m_cTree.GetParentItem(m_cTree.GetParentItem(item))))
		m_cTree.Expand(hItem, TVE_EXPAND);
		HTREEITEM hChild = m_cTree.GetChildItem(hItem);
		while (hChild) {
			if (!LEVEL2_CHILD(hChild))
				ExpandTree(hChild);
			hChild = m_cTree.GetNextSiblingItem(hChild);
		}

	}

	void ExpandNalTree() { ExpandTree(m_cTree.GetChildItem(TVI_ROOT)); };
	void ProcListSelectedItem(int nIndex);

    // our own
private:
    int m_nSliceIndex;
    CString m_strFileUrl;
    CNalParser m_cParser;
    vector<NALU_t> m_vNalTypeVector;
    int m_nValTotalNum; // m_vNalTypeVector有多少个NALU_t

	char m_MoreInfo[512];
	int  m_nTotalDataError;

    HANDLE m_hFileThread;
    HANDLE m_hNALThread;

    HANDLE m_hFileLock;
    HANDLE m_hNALLock;
    CRect m_rectMainWnd;
    videoinfo_t m_cVideoInfo;
    BOOL m_fCanPlay;
	BOOL m_bWillExit;

    static UINT ThreadFuncReadFile(LPVOID lpvoid);

    void ReadFile(void);
    void PaseNal(void);

	int FindPrevPPS(int nIndex);

public:
	virtual void OnOk() {};
	virtual void OnCancel() {};
	BOOL PreTranslateMessage(MSG* pMsg);

    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnLvnItemActivateH264Nallist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMCustomdrawH264Nallist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnFileOpen();
    afx_msg void OnHowtoUsage();
    afx_msg void OnLvnKeydownH264Nallist(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnPlayDlg();
	afx_msg void OnNMDblclkH264Nallist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
	afx_msg void OnLvnGetdispinfoH264Nallist(NMHDR *pNMHDR, LRESULT *pResult);
	CEdit m_cStreamInfo;
	afx_msg void OnUsage();
};
