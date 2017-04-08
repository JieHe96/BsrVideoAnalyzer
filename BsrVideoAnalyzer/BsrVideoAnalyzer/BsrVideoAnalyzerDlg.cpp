
// BsrVideoAnalyzerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BsrVideoAnalyzer.h"
#include "BsrVideoAnalyzerDlg.h"
#include "afxdialogex.h"

#include "NaLParse.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
//#define new DEBUG_NEW
//#define new new(__FILE__, __LINE__)//DEBUG_NEW
#endif


void CNewListCtrl::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	// for Dialog based applications, this is a good place
	// to subclass the header control because the OnCreate()
	// function does not get called.

	SubclassHeaderControl();
}

afx_msg int CNewListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
	{
		ASSERT(FALSE);
		return -1;
	}

	// When the CXListCtrl object is created via a call to Create(), instead
	// of via a dialog box template, we must subclass the header control
	// window here because it does not exist when the PreSubclassWindow()
	// function is called.

	SubclassHeaderControl();

	return 0;
}

BEGIN_MESSAGE_MAP(CNewListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CNewListCtrl)
	ON_WM_CREATE()
END_MESSAGE_MAP()


BsrVideoAnalyzerDlg::BsrVideoAnalyzerDlg(CWnd* pParent /*=NULL*/)
    : CDialog(BsrVideoAnalyzerDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_hFileThread = INVALID_HANDLE_VALUE;
    m_hNALThread = INVALID_HANDLE_VALUE;
    m_hFileLock = INVALID_HANDLE_VALUE;
    m_hNALLock = INVALID_HANDLE_VALUE;
    m_strFileUrl.Empty();
    m_pPlayDlg = NULL;
    m_fCanPlay = FALSE;
	m_bWillExit = FALSE;
    memset(&m_cVideoInfo, '\0', sizeof(videoinfo_t));
}

BsrVideoAnalyzerDlg::~BsrVideoAnalyzerDlg()
{
	if (m_pPlayDlg)
		delete m_pPlayDlg;

	// the thread m_hFileThread is still running, we must let it exit
	// otherwise it will cause memory leak :\src\mfc\strcore.cpp(156)
	

	if (m_hFileThread && m_hFileThread != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileThread);
	if (m_hFileLock && m_hFileLock != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileLock);
	if (m_hNALLock && m_hNALLock != INVALID_HANDLE_VALUE)
		CloseHandle(m_hNALLock);
}

void BsrVideoAnalyzerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_H264_NALLIST, m_h264NalList);
	DDX_Control(pDX, IDC_EDIT_HEX, m_edHexInfo);
	DDX_Control(pDX, IDC_TREE1, m_cTree);
	DDX_Control(pDX, IDC_EDIT_SIMINFO, m_cStreamInfo);
}

BEGIN_MESSAGE_MAP(BsrVideoAnalyzerDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_DROPFILES()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_FILE, &BsrVideoAnalyzerDlg::OnFileOpen)
	ON_COMMAND(ID_PLAY, &BsrVideoAnalyzerDlg::OnPlayDlg)
	ON_COMMAND(ID_USAGE, &BsrVideoAnalyzerDlg::OnUsage)
	ON_COMMAND(ID_QUIT_APP, &BsrVideoAnalyzerDlg::OnClose)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_H264_NALLIST, &BsrVideoAnalyzerDlg::OnLvnItemActivateH264Nallist)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_H264_NALLIST, &BsrVideoAnalyzerDlg::OnNMCustomdrawH264Nallist)
	ON_NOTIFY(LVN_KEYDOWN, IDC_H264_NALLIST, &BsrVideoAnalyzerDlg::OnLvnKeydownH264Nallist)
	ON_NOTIFY(NM_DBLCLK, IDC_H264_NALLIST, &BsrVideoAnalyzerDlg::OnNMDblclkH264Nallist)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_H264_NALLIST, &BsrVideoAnalyzerDlg::OnLvnGetdispinfoH264Nallist)
END_MESSAGE_MAP()

// BsrVideoAnalyzerDlg message handlers

BOOL BsrVideoAnalyzerDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);            // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // TODO: Add extra initialization here
    
	//整行选择；有表格线；表头；单击激活
    DWORD dwExStyle=LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_ONECLICKACTIVATE;
    //报表风格；单行选择；高亮显示选择行
    m_h264NalList.ModifyStyle(0,LVS_SINGLESEL|LVS_REPORT|LVS_SHOWSELALWAYS);
    m_h264NalList.SetExtendedStyle(dwExStyle);

    // 左对齐
    m_h264NalList.InsertColumn(0,_T("No."),LVCFMT_LEFT,50,0);
    m_h264NalList.InsertColumn(1,_T("Offset"),LVCFMT_LEFT,70,0);
    m_h264NalList.InsertColumn(2,_T("Length"),LVCFMT_LEFT,60,0);
    m_h264NalList.InsertColumn(3,_T("Start Code"),LVCFMT_LEFT,100,0);
    m_h264NalList.InsertColumn(4,_T("NAL Type"),LVCFMT_LEFT,280,0);
    m_h264NalList.InsertColumn(5,_T("Info"),LVCFMT_LEFT,140,0);
	m_h264NalList.SetColumnWidth(5, LVSCW_AUTOSIZE_USEHEADER);

	m_h264NalList.SubclassHeaderControl();
	m_h264NalList.m_HeaderCtrl.SetTextColor(RGB(240, 10, 40));
	//m_h264NalList.m_HeaderCtrl.EnableDividerLines(FALSE);

    m_nSliceIndex = 0;

    m_nValTotalNum = 0;

    m_strFileUrl.Empty();

    m_edHexInfo.SetOptions(1, 1, 1, 1);
    m_edHexInfo.SetBPR(24); // 16字节

	RECT rc;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rc, 0);

	CRect winPos;
	GetWindowRect(winPos);

	// 主窗口大小
	GetClientRect(&m_rectMainWnd);
	//ScreenToClient(m_rectMainWnd);

	CPoint xx = CPoint(0, 0);
	::ClientToScreen(this->m_hWnd, &xx);


	SetWindowPos(&wndNoTopMost, 0-xx.x, 0, winPos.Width(), rc.bottom, /*SWP_NOSIZE | */SWP_SHOWWINDOW);

    if (m_hFileLock == INVALID_HANDLE_VALUE)
    {
        m_hFileLock = CreateEvent(NULL,FALSE,FALSE,NULL);
    }
    if (m_hNALLock == INVALID_HANDLE_VALUE)
    {
        m_hNALLock = CreateEvent(NULL,FALSE,FALSE,NULL);
    }
    if (m_hFileThread == INVALID_HANDLE_VALUE)
    {
        m_hFileThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFuncReadFile, this, NULL, NULL );
    }

#if 0
#define AddTreeItem(item, buffer) m_cTree.InsertItem(buffer,item)


    HTREEITEM hItem = m_cTree.InsertItem("根节点NAL",TVI_ROOT);///root
    CString strTemp;
    strTemp.Format("NAL头节点nal_unit_header");
    HTREEITEM hSubItem = AddTreeItem(hItem, strTemp.GetBuffer());

    strTemp.Format("forbidden_zero_bit \t\t:0 (1 bit)");
    AddTreeItem(hSubItem, strTemp.GetBuffer());
    strTemp.Format("nal_unit_type \t\t:32 (6 bit)");
    AddTreeItem(hSubItem, strTemp.GetBuffer());
    strTemp.Format("nal_ref_idc \t\t:0 (6 bit)");
    AddTreeItem(hSubItem, strTemp.GetBuffer());
    strTemp.Format("nuh_temporal_id_plus1 \t\t:0 (3 bit)");
    AddTreeItem(hSubItem, strTemp.GetBuffer());

    strTemp.Format("VPS节点video_parameter_set_rbsp()");
    HTREEITEM hItem1 = AddTreeItem(hItem, strTemp.GetBuffer());

    strTemp.Format("header()");
    HTREEITEM hItem2 = AddTreeItem(hItem1, strTemp.GetBuffer());
    strTemp.Format("fist slice)");
    AddTreeItem(hItem2, strTemp.GetBuffer());
    strTemp.Format("no output");
    AddTreeItem(hItem2, strTemp.GetBuffer());

    strTemp.Format("data()");
    AddTreeItem(hItem1, strTemp.GetBuffer());
#endif


    /*
    CMenu popMenu;
    CMenu *pPopup;
    popMenu.LoadMenu(IDR_MENU);

    pPopup=popMenu.GetSubMenu(1);

    pPopup->EnableMenuItem(ID_PLAY_PLAY, MF_BYCOMMAND|MF_ENABLED);
    pPopup->Detach();
    popMenu.DestroyMenu();
    */

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void BsrVideoAnalyzerDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR BsrVideoAnalyzerDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void BsrVideoAnalyzerDlg::UpdateTitle(char *info)
{
	CString strTemp;
	strTemp.Format("[%s] - %s -%s ", info, m_strFileUrl, APP_NAM);
	this->SetWindowText(strTemp);
}

void BsrVideoAnalyzerDlg::SystemClear()
{
    //m_vNalInfoVector.clear();
    m_vNalTypeVector.clear();
    m_h264NalList.DeleteAllItems();
    m_nSliceIndex = 0;
    m_nValTotalNum = 0;
}

// 打开H264码流文件
void BsrVideoAnalyzerDlg::OnBnClickedH264InputurlOpen()
{
    // TODO: Add your control notification handler code here
    SystemClear();
    UpdateData();

    if(m_strFileUrl.IsEmpty()==TRUE)
    {
        AfxMessageBox(_T("文件路径为空，请打开文件！！"));
        return;
    }

	GetDlgItem(IDC_EDIT_SIMINFO)->SetWindowText("");

    m_fCanPlay = FALSE;
	if (m_pPlayDlg)
	{
		m_pPlayDlg->SendMessage(WM_CLOSE);
	}
    SetEvent(m_hFileLock);
}

UINT BsrVideoAnalyzerDlg::ThreadFuncReadFile(LPVOID lpvoid)
{
    BsrVideoAnalyzerDlg* dlg = static_cast<BsrVideoAnalyzerDlg*>(lpvoid);
    dlg->ReadFile();
    return 0;
}

void BsrVideoAnalyzerDlg::ShowParsingProgess(uint64_t size0, uint64_t size1, uint64_t len)
{
	char str_mess[256];
#define SIZE10M (1024*1024*10)
	if (len > 0)
	{
		sprintf(str_mess, "Parsinging raw video file: %d%%", size1 * 100 / len);
		UpdateTitle(str_mess);
	}
	else
	if (size0 / SIZE10M != size1 / SIZE10M)
	{
		sprintf(str_mess, "Making raw video file: %d MB", size1*10/SIZE10M);
		UpdateTitle(str_mess);
	}

}

void BsrVideoAnalyzerDlg::ReadFile(void)
{
    CString strFilePath;
    CString strSimpleInfo;
    CString strProfileInfo;
    CString strLevelInfo;
    CString strTierInfo;
    CString strVideoFormat;
    CString strBitDepth;
    CString strMaxNalNum;
    int nMaxNalNum = -1;
    SPSInfo_t sps = {0};
    PPSInfo_t pps = {0};

    strTierInfo.Empty();

    while (true)
    {
        WaitForSingleObject(m_hFileLock, INFINITE);

		if (m_bWillExit)
		{
			m_fCanPlay = TRUE;
			PostMessage(WM_CLOSE, 0, 0);
			break;
		}
        
		int ret = m_cParser.init(m_strFileUrl, &m_cTree, m_MoreInfo);
		if (ret < 0)
		{
			MessageBox("Maybe not H264 or H265 file or No Video Stream ?");
			continue;
		}
		if (m_cParser.isWillBreak()) continue;

		m_strFileUrl.Format(_T("%s"), m_cParser.getParsingFilename());

		
		m_cParser.probeNALU(m_vNalTypeVector, nMaxNalNum);

        m_nValTotalNum = m_vNalTypeVector.size();
		m_nSliceIndex = m_cParser.cInfo.frames;
        m_cParser.getVideoInfo(&m_cVideoInfo);
        
		// H.265
        if (m_cVideoInfo.type)
        {
            // profile类型
            switch (m_cVideoInfo.profile_idc)
            {
            case PROFILE_NONE:
                strProfileInfo.Format(_T("None"));
                break;
            case PROFILE_MAIN:
                strProfileInfo.Format(_T("Main"));
                break;
            case PROFILE_MAIN10:
                strProfileInfo.Format(_T("Main10"));
                break;
            case PROFILE_MAINSTILLPICTURE:
                strProfileInfo.Format(_T("Main Still Picture"));
                break;
            case PROFILE_MAINREXT:
                strProfileInfo.Format(_T("Main RExt"));
                break;
            case PROFILE_HIGHTHROUGHPUTREXT:
                strProfileInfo.Format(_T("High Throughput RExt"));
                break;
            default:
                strProfileInfo.Format(_T("Unkown"));
                break;
            }
            switch (m_cVideoInfo.level_idc)
            {
            case LEVEL_NONE:
                strLevelInfo.Format(_T("none(%d)"), LEVEL_NONE);
                break;
            case LEVEL1:
                strLevelInfo.Format(_T("1(%d)"), LEVEL1);
                break;
            case LEVEL2:
                strLevelInfo.Format(_T("2(%d)"), LEVEL2);
                break;
            case LEVEL2_1:
                strLevelInfo.Format(_T("2.1(%d)"), LEVEL2_1);
                break;
            case LEVEL3:
                strLevelInfo.Format(_T("3(%d)"), LEVEL3);
                break;
            case LEVEL3_1:
                strLevelInfo.Format(_T("3.1(%d)"), LEVEL3_1);
                break;
            case LEVEL4:
                strLevelInfo.Format(_T("4(%d)"), LEVEL4);
                break;
            case LEVEL4_1:
                strLevelInfo.Format(_T("4.1(%d)"), LEVEL4_1);
                break;
            case LEVEL5:
                strLevelInfo.Format(_T("5(%d)"), LEVEL5);
                break;
            case LEVEL5_1:
                strLevelInfo.Format(_T("5.1(%d)"), LEVEL5_1);
                break;
            case LEVEL5_2:
                strLevelInfo.Format(_T("5.2(%d)"), LEVEL5_2);
                break;
            case LEVEL6:
                strLevelInfo.Format(_T("6(%d)"), LEVEL6);
                break;
            case LEVEL6_1:
                strLevelInfo.Format(_T("6.1(%d)"), LEVEL6_1);
                break;
            case LEVEL6_2:
                strLevelInfo.Format(_T("6.2(%d)"), LEVEL6_2);
                break;
            case LEVEL8_5:
                strLevelInfo.Format(_T("8.5(%d)"), LEVEL8_5);
                break;
            default:
                strLevelInfo.Format(_T("Unkown"));
                break;
            }
            switch (m_cVideoInfo.tier_idc)
            {
            case 1:
                strTierInfo.Format(_T("Tier High"));
                break;
            case 0:
            default:
                strTierInfo.Format(_T("Tier Main"));
                break;
            }
        }
        else // h264
        {
            // profile类型
            switch (m_cVideoInfo.profile_idc)
            {
            case 66:
                strProfileInfo.Format(_T("Baseline"));
                break;
            case 77:
                strProfileInfo.Format(_T("Main"));
                break;
            case 88:
                strProfileInfo.Format(_T("Extended"));
                break;
            case 100:
                strProfileInfo.Format(_T("High"));
                break;
            case 110:
                strProfileInfo.Format(_T("High 10"));
                break;
            case 122:
                strProfileInfo.Format(_T("High 422"));
                break;
            case 144:
                strProfileInfo.Format(_T("High 444"));
                break;
            default:
                strProfileInfo.Format(_T("Unkown"));
                break;
            }
            strTierInfo.Empty();
            strLevelInfo.Format(_T("%d"), m_cVideoInfo.level_idc);
        }

        // common
        // bit depth
        strBitDepth.Format(_T("Luma bit: %d Chroma bit: %d"), m_cVideoInfo.bit_depth_luma, m_cVideoInfo.bit_depth_chroma);

        // chroma format
        switch (m_cVideoInfo.chroma_format_idc)
        {
        case 1:
            strVideoFormat.Format(_T("YUV420"));
            break;
        case 2:
            strVideoFormat.Format(_T("YUV422"));
            break;
        case 3:
            strVideoFormat.Format(_T("YUV444"));
            break;
        case 0:
            strVideoFormat.Format(_T("monochrome"));
            break;
        default:
            strVideoFormat.Format(_T("Unkown"));
            break;
        }

        /*
        "Video Format: xxx\r\n"
        */
		m_cVideoInfo.max_framerate = m_cParser.cInfo.framerate;
        strSimpleInfo.Format(
            "%s File Information\r\n\r\n"
            "Picture Size \t: %dx%d\r\n"
            "  - Cropping Left \t: %d\r\n"
            "  - Cropping Right \t: %d\r\n"
            "  - Cropping Top \t: %d\r\n"
            "  - Cropping Bottom : %d\r\n"
            "Video Format \t: %s %s\r\n"
            "Stream Type \t: %s Profile @ Level %s %s\r\n"
            "Encoding Type \t: %s\r\n"
            "Max fps \t\t: %.03f\r\n"
            "Frame Count \t: %d\r\n"
			"%s",
            m_cVideoInfo.type ? "H.265/HEVC" : "H.264/AVC",
            m_cVideoInfo.width, m_cVideoInfo.height,
            m_cVideoInfo.crop_left, m_cVideoInfo.crop_right,
            m_cVideoInfo.crop_top, m_cVideoInfo.crop_bottom,
            strVideoFormat, strBitDepth, 
            strProfileInfo, strLevelInfo, strTierInfo,
            m_cVideoInfo.encoding_type ? "CABAC" : "CAVLC",
            m_cVideoInfo.max_framerate, m_nSliceIndex,
			m_MoreInfo
            );
        GetDlgItem(IDC_EDIT_SIMINFO)->SetWindowText(strSimpleInfo);
		
		m_nTotalDataError = 0;

		for (int i = 0; i < m_nValTotalNum; i++)
		{
			NALU_t* nalu = &m_vNalTypeVector[i];
			if (nalu->errOffset > 0)
			{
				m_nTotalDataError++;
				char strTemp[128];
				sprintf(strTemp, "ERROR: #%d (Total: %d) ", nalu->num + 1, m_nTotalDataError);
				UpdateTitle(strTemp);
			}
		}
		m_h264NalList.SetItemCount(m_vNalTypeVector.size());

		//checking if the list has vscrollbar
		SCROLLINFO scrollInfo;
		if (m_h264NalList.GetScrollInfo(SB_VERT, &scrollInfo))
		{
			m_h264NalList.SetColumnWidth(5, LVSCW_AUTOSIZE_USEHEADER);

			m_h264NalList.m_HeaderCtrl.SetScrollBarVisible();
			m_h264NalList.m_HeaderCtrl.Invalidate();
		}


		strSimpleInfo.Empty();

		strSimpleInfo.Format(
			"%s File Information\r\n\r\n"
			"Picture Size \t: %dx%d\r\n"
			"  - Cropping Left \t: %d\r\n"
			"  - Cropping Right \t: %d\r\n"
			"  - Cropping Top \t: %d\r\n"
			"  - Cropping Bottom : %d\r\n"
			"Video Format \t: %s %s\r\n"
			"Stream Type \t: %s Profile @ Level %s %s\r\n"
			"Encoding Type \t: %s\r\n"
			"Max fps \t\t: %.03f\r\n"
			"Frame Count \t: %d\r\n"
			"File Size \t\t: %lld Bytes\r\n",
			m_cVideoInfo.type ? "H.265/HEVC" : "H.264/AVC",
			m_cVideoInfo.width, m_cVideoInfo.height,
			m_cVideoInfo.crop_left, m_cVideoInfo.crop_right,
			m_cVideoInfo.crop_top, m_cVideoInfo.crop_bottom,
			strVideoFormat, strBitDepth,
			strProfileInfo, strLevelInfo, strTierInfo,
			m_cVideoInfo.encoding_type ? "CABAC" : "CAVLC",
			m_cVideoInfo.max_framerate,
			m_nSliceIndex,
			m_cParser.cInfo.filesize
			);

		CString tmpString;
		float bRate = ((float)m_cParser.cInfo.filesize * 8 * m_cVideoInfo.max_framerate / m_nSliceIndex / 1000);
		if (m_cParser.cInfo.bitrate > 0)
			tmpString.Format("Avg Bitrate \t: %.02f kbps [%d kbps]\r\n", bRate, m_cParser.cInfo.bitrate/1000);
		else
			tmpString.Format("Avg Bitrate \t: %.02f kbps\r\n", bRate);
		strSimpleInfo.Append(tmpString);
		strSimpleInfo.Append(m_MoreInfo);
		GetDlgItem(IDC_EDIT_SIMINFO)->SetWindowText(strSimpleInfo);
		
		//after listing, set selected item on first item
		ProcListSelectedItem(0);
		m_h264NalList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

		m_fCanPlay = TRUE;
    }
}

// 双击(单击)某一项，进行NAL详细分析
void BsrVideoAnalyzerDlg::OnLvnItemActivateH264Nallist(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: Add your control notification handler code here
    //----------------------
    POSITION ps;
    int nIndex;
    int ret = 0;

    ps=m_h264NalList.GetFirstSelectedItemPosition();
    nIndex=m_h264NalList.GetNextSelectedItem(ps);

	ProcListSelectedItem(nIndex);

    *pResult = 0;

}

#define IS_IFRAME(x) \
( \
	((x).nalType == NAL_UNIT_TYPE_CODED_SLICE_IDR) || \
	(((x).nalType == NAL_UNIT_TYPE_CODED_SLICE_NON_IDR) && ((x).sliceType == 2 || (x).sliceType == 7) ) \
)

int BsrVideoAnalyzerDlg::FindPrevPPS(int nIndex)
{
	BOOL hasSPS = FALSE;
	BOOL hasPPS = FALSE;
	BOOL hasVPS = FALSE;
	int i = nIndex;
	int gop = m_vNalTypeVector[i].gop;

	//nIndex == 0, we will find the first I frame 
	if (nIndex == 0)
	{
		int gopFirstPos = nIndex;
		for (i = nIndex; i <= m_nValTotalNum - 1; i++)
		{
			gopFirstPos = (gop != m_vNalTypeVector[i].gop ? i : gopFirstPos);
			gop = (gop != m_vNalTypeVector[i].gop ? m_vNalTypeVector[i].gop : gop);
			if (m_vNalTypeVector[i].type == 0)
			{
				if (m_vNalTypeVector[i].nalType == NAL_UNIT_TYPE_SPS)
					hasSPS = TRUE;
				if (m_vNalTypeVector[i].nalType == NAL_UNIT_TYPE_PPS)
					hasPPS = TRUE;

				if (hasSPS && hasPPS && IS_IFRAME(m_vNalTypeVector[i]))
					return i;// gopFirstPos;
			}
			else
			{
				if (m_vNalTypeVector[i].nalType == NAL_UNIT_SPS)
					hasSPS = TRUE;
				if (m_vNalTypeVector[i].nalType == NAL_UNIT_PPS)
					hasPPS = TRUE;
				if (m_vNalTypeVector[i].nalType == NAL_UNIT_VPS)
					hasVPS = TRUE;

				if (hasVPS && hasSPS && hasPPS)
					return gopFirstPos;
			}
		}

		return -1;
	}
	//find the first I frame before the nIndex item
	for (i = nIndex; i >= 0; i--)
	{
		if (m_vNalTypeVector[i].type == 0)
		{
			if (m_vNalTypeVector[i].nalType == NAL_UNIT_TYPE_SPS)
				hasSPS = TRUE;
			if (m_vNalTypeVector[i].nalType == NAL_UNIT_TYPE_PPS)
				hasPPS = TRUE;

			if (gop != m_vNalTypeVector[i].gop)
				return i + 1;
			if (/*hasSPS && hasPPS && */IS_IFRAME(m_vNalTypeVector[i]))
				return i;// gopFirstPos;
		}
		else
		{
			if (m_vNalTypeVector[i].nalType == NAL_UNIT_SPS)
				hasSPS = TRUE;
			if (m_vNalTypeVector[i].nalType == NAL_UNIT_PPS)
				hasPPS = TRUE;
			if (m_vNalTypeVector[i].nalType == NAL_UNIT_VPS)
				hasVPS = TRUE;

			if (gop != m_vNalTypeVector[i].gop)
				return i + 1;
			if (hasVPS && hasSPS && hasPPS && (i == 0))
				return i;
		}
	}

	for (i = nIndex; i <= m_nValTotalNum - 1; i++)
	{
		if (IS_IFRAME(m_vNalTypeVector[i])/*gop != m_vNalTypeVector[i].gop*/)
			return i;
	}

	return -1;

}

void BsrVideoAnalyzerDlg::ProcListSelectedItem(int nIndex)
{
	if (nIndex < 0 || nIndex > m_nValTotalNum - 1) return;

	char* nalData = NULL;
	char* nalInfo = NULL;
	int ret = m_cParser.parseNALU(m_vNalTypeVector[nIndex], &nalData, &nalInfo);
	if (ret < 0)
	{
		AfxMessageBox("解析NAL时出错，可能是文件读取出错。");
	}

	// 显示十六进制
	m_edHexInfo.SetData((LPBYTE)nalData, m_vNalTypeVector[nIndex].len);

	ExpandNalTree();

	if (m_vNalTypeVector[nIndex].errOffset > 0)
	{
		m_edHexInfo.ResetPos();
		m_edHexInfo.ScrollIntoView(m_vNalTypeVector[nIndex].errOffset);
	}
	::SendMessage(GetDlgItem(IDC_EDIT_HEX)->m_hWnd, WM_KILLFOCUS, -1, 0); // 不要控件焦点
}


void BsrVideoAnalyzerDlg::OnLvnKeydownH264Nallist(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLVKEYDOWN pLVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    POSITION ps;
    int nIndex = 0;
    int ret = 0;

    // 不是上下光标的，不响应
    if (pLVKeyDown->wVKey != VK_UP && pLVKeyDown->wVKey != VK_DOWN)
    {
        return;
    }

    ps=m_h264NalList.GetFirstSelectedItemPosition();
    if (ps == NULL)
    {
        AfxMessageBox("No items were selected!");
        return;
    }
    else
    {
        while (ps)
        {
            nIndex=m_h264NalList.GetNextSelectedItem(ps);
        }
    }
    // i don't know how this works...
    // but it just ok
    if (pLVKeyDown->wVKey == VK_UP)
    {
        nIndex--;
    }
    else if (pLVKeyDown->wVKey == VK_DOWN)
    {
        nIndex++;
    }

    if (nIndex < 0) nIndex = 0;
    if (nIndex > m_nValTotalNum - 1) nIndex = m_nValTotalNum - 1;

	ProcListSelectedItem(nIndex);

    *pResult = 0;
}

void BsrVideoAnalyzerDlg::OnNMCustomdrawH264Nallist(NMHDR *pNMHDR, LRESULT *pResult)
{
    //This code based on Michael Dunn's excellent article on
    //list control custom draw at http://www.codeproject.com/listctrl/lvcustomdraw.asp

    COLORREF clrNewTextColor, clrNewBkColor;
    int nItem;
    NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    // Take the default processing unless we set this to something else below.
    *pResult = CDRF_DODEFAULT;

    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.
    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
    {
        *pResult = CDRF_NOTIFYITEMDRAW;
    }
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
    {
        // This is the notification message for an item.  We'll request
        // notifications before each subitem's prepaint stage.

        *pResult = CDRF_NOTIFYSUBITEMDRAW;
    }
    else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
    {
        clrNewTextColor = RGB(0,0,0);
        clrNewBkColor = RGB(255,255,255);

        nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

        CString strTemp = m_h264NalList.GetItemText(nItem,5);   // 第5列是类型，判断之
        if(strncmp(strTemp,"SLICE", 5)==0)
        {
            clrNewBkColor = RGB(0,255,255);       //青色
        }
        else if(strncmp(strTemp,"VPS", 3)==0)
        {
            clrNewBkColor = RGB(0,192,0);        //紫色
        }
        else if(strncmp(strTemp,"SPS", 3)==0)
        {
            clrNewBkColor = RGB(255,255,0);        //黄色
        }
        else if(strncmp(strTemp,"PPS", 3)==0)
        {
            clrNewBkColor = RGB(255,153,0);        //咖啡色
        }
        else if(strncmp(strTemp,"SEI", 3)==0)
        {
            clrNewBkColor = RGB(128,128,192);       //灰色
        }
        else if(strncmp(strTemp,"IDR", 3)==0)
        {
            //clrNewBkColor = RGB(192, 192, 192);          //gray
			if (pLVCD->iSubItem == 5)
				clrNewTextColor = RGB(255, 0, 0); // Red
		}
        else if(strncmp(strTemp,"P Slice", 7)==0)
        {
            // 只有第5列才显示这里设置的颜色
            if (pLVCD->iSubItem == 5)
                clrNewTextColor = RGB(0,0,255); // Blue
        }
        else if(strncmp(strTemp,"B Slice", 7)==0)
        {
            if (pLVCD->iSubItem == 5)
                clrNewTextColor = RGB(0, 128, 0); // Green
        }
        else if(strncmp(strTemp,"I Slice", 7)==0)
        {
            if (pLVCD->iSubItem == 5)
                clrNewTextColor = RGB(255,0,0); // Red
        }
		//by hcg
		else if (strncmp(strTemp, "ERR", 3) == 0)
		{
			//if (pLVCD->iSubItem == 5)
			clrNewBkColor = RGB(255, 0, 0); // Red
		}

        pLVCD->clrText = clrNewTextColor;
        pLVCD->clrTextBk = clrNewBkColor;

        // Tell Windows to paint the control itself.
        *pResult = CDRF_DODEFAULT;
    }
}


// 主界面需要设置Accept Files为TRUE
void BsrVideoAnalyzerDlg::OnDropFiles(HDROP hDropInfo)
{
    CDialog::OnDropFiles(hDropInfo);

    char* pFilePathName =(char *)malloc(MAX_URL_LENGTH);
    ::DragQueryFile(hDropInfo, 0, (LPSTR)pFilePathName, MAX_URL_LENGTH);  // 获取拖放文件的完整文件名，最关键！

    ::DragFinish(hDropInfo);   // 注意这个不能少，它用于释放Windows 为处理文件拖放而分配的内存

    FILE* fp = fopen(pFilePathName, "r+b");
    if (fp == NULL)
    {
        AfxMessageBox("Error open file.");
		free(pFilePathName);
		return;
    }
    fclose(fp);

	m_cParser.SetCallerPointer((void *)this);
#if 0	
	int ret = m_cParser.init(pFilePathName, &m_cTree, m_MoreInfo);
    if (ret < 0)
    {
        MessageBox("Maybe not H264 or H265 file?");
		free(pFilePathName);
		return;
    }

	if (ret >= FILE_BSR264)
		m_strFileUrl.Format(_T("%s.bin"), pFilePathName);
	else
		m_strFileUrl.Format(_T("%s"), pFilePathName);
#endif

	m_strFileUrl.Format(_T("%s"), pFilePathName);
	free(pFilePathName);

    OnBnClickedH264InputurlOpen();
}

void BsrVideoAnalyzerDlg::OnFileOpen()
{
    CString strFilePath;
	char szFilter[] = "H.264 or H.265 Files(*.bsr;*.h264;*.264;*.h265;*.265;*.bin)|*.h264;*.264;*.h265;*.265;*.hevc;*.bsr;*.bin;*.avi;*.asf;*.mkv;*.mp4;*.flv|All Files(*.*)|*.*||";
    CFileDialog *fileDlg = new CFileDialog(TRUE, "H.264", NULL, OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST, szFilter, this);
    fileDlg->GetOFN().lpstrTitle = _T("选择H.264或H.265码流文件");   // 标题

	if (fileDlg->DoModal() != IDOK)
    {
		delete fileDlg;
        return;
    }

    //m_strFileUrl = fileDlg.GetPathName();
	char pFilePathName[MAX_URL_LENGTH];
	sprintf(pFilePathName, "%s", fileDlg->GetPathName());

	delete fileDlg;
	
	FILE* fp = fopen(pFilePathName, "r+b");
    if (fp == NULL)
    {
        AfxMessageBox("Error open file.");
        return;
    }
    fclose(fp);

	m_cParser.SetCallerPointer((void *)this);
#if 0
    int ret = m_cParser.init(pFilePathName, &m_cTree, m_MoreInfo);
    if (ret < 0)
    {
        MessageBox("Maybe not H264 or H265 file?");
        return;
    }
	
	m_strFileUrl.Format(_T("%s"), m_cParser.getParsingFilename());
#endif
	m_strFileUrl.Format(_T("%s"), pFilePathName);// m_cParser.getParsingFilename());
	OnBnClickedH264InputurlOpen();
}


void BsrVideoAnalyzerDlg::OnHowtoUsage()
{
    // TODO: Add your command handler code here
}


/**
主窗口使用GetClientRect是因为需要与本函数的cx、cy计算缩放比例。
控件使用GetWindowRect和ScreenToClient是为了得到相对的坐标。
note：
固定的数值是测试得到的经验值，无理论依据
*/
void BsrVideoAnalyzerDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    // 非法值，什么也不做
    if (cx <= 0 || cy <= 0) return;

    CRect rectList, rectHex, rectTxt, rectInfo, rectTree, rectMainWnd;
    CWnd *pWnd = NULL;
	pWnd = GetDlgItem(IDC_H264_NALLIST);
	if (pWnd == NULL) return;
    GetDlgItem(IDC_H264_NALLIST)->GetWindowRect(&rectList);
    ScreenToClient(rectList);
    GetDlgItem(IDC_STATIC)->GetWindowRect(&rectTxt);
    ScreenToClient(rectTxt);
    GetDlgItem(IDC_EDIT_HEX)->GetWindowRect(&rectHex);
    ScreenToClient(rectHex);
    GetDlgItem(IDC_EDIT_SIMINFO)->GetWindowRect(&rectInfo);
    ScreenToClient(rectInfo);
    GetDlgItem(IDC_TREE1)->GetWindowRect(&rectTree);
    ScreenToClient(rectTree);


    float fXRatio = (float)cx / (float)(m_rectMainWnd.Width());
    float fYRatio = (float)cy / (float)(m_rectMainWnd.Height());

    int nNewWidth = 0;
    int nNewHeight = 0;

    // 列表框
    pWnd = GetDlgItem(IDC_H264_NALLIST);
    nNewWidth = (int)(fXRatio * (float)rectList.Width());
    nNewHeight = (int)(fYRatio * (float)rectList.Height());
    pWnd->MoveWindow(rectList.left, rectList.top, nNewWidth, nNewHeight);
    pWnd->Invalidate();
    pWnd->UpdateData();
    pWnd->GetWindowRect(&rectList);
    ScreenToClient(rectList);

    // 列表框里面的
    nNewWidth = (int)(m_h264NalList.GetColumnWidth(4) * fXRatio);
    m_h264NalList.SetColumnWidth(4, nNewWidth);
    nNewWidth = (int)(m_h264NalList.GetColumnWidth(5) * fXRatio);
    m_h264NalList.SetColumnWidth(5, nNewWidth);
    ///////////////////////

    // "Hex View" 文本
    pWnd = GetDlgItem(IDC_STATIC);
    pWnd->GetWindowRect(&rectTxt);
    ScreenToClient(rectTxt);
    pWnd->MoveWindow(rectTxt.left, rectList.Height(), rectTxt.Width(), rectTxt.Height());
    
    pWnd->Invalidate();
    pWnd->SetWindowText("Hex View");
    pWnd->UpdateData();
    pWnd->GetWindowRect(&rectTxt);
    ScreenToClient(rectTxt);

    // 十六进制框
    pWnd = GetDlgItem(IDC_EDIT_HEX);
    nNewWidth = (int)(fXRatio * (float)rectHex.Width());
    nNewHeight = (int)(fYRatio * (float)rectHex.Height());
    pWnd->MoveWindow(rectHex.left, rectList.Height()+rectTxt.Height(), nNewWidth, cy - rectList.Height() - 18);
    pWnd->Invalidate();
    pWnd->UpdateData();
    pWnd->GetWindowRect(&rectHex);
    ScreenToClient(rectHex);

    // 信息框
    pWnd = GetDlgItem(IDC_EDIT_SIMINFO);
    nNewWidth = (int)(fXRatio * (float)rectInfo.Width());
    nNewHeight = (int)(fYRatio * (float)rectInfo.Height());
    pWnd->MoveWindow(rectList.Width()+5, rectInfo.top, cx - rectList.Width() - 7, nNewHeight);
    pWnd->Invalidate();
    pWnd->UpdateData();
    pWnd->GetWindowRect(&rectInfo);
    ScreenToClient(rectInfo);

    // 树形控件框
    pWnd = GetDlgItem(IDC_TREE1);
    nNewWidth = (int)(fXRatio * (float)rectTree.Width());
    nNewHeight = (int)(fYRatio * (float)rectTree.Height());
    pWnd->MoveWindow(rectList.Width()+5, rectInfo.Height()+3, cx - rectList.Width() - 7, cy - rectInfo.Height() - 6);
    pWnd->Invalidate();
    pWnd->UpdateData();

	//::GetBkColor(::GetDC(m_cStreamInfo.m_hWnd))
	m_cTree.SetBkColor(GetSysColor(COLOR_3DFACE));

    // 更新当前主窗口大小
    GetClientRect(&m_rectMainWnd);
}

void BsrVideoAnalyzerDlg::OnPlayDlg()
{
	if (m_strFileUrl.IsEmpty() == TRUE)
	{
		MessageBox("Please select a media file ...");
		return;
	}
	if (!m_fCanPlay)
    {
        MessageBox("Parsing NALU, wait a momnent...");
        return;
    }
    // 非模态对话框
    if (m_pPlayDlg == NULL)
    {
        m_pPlayDlg = new CPlayDlg();
		m_pPlayDlg->SetVideoSize(m_cVideoInfo.width, m_cVideoInfo.height);
		m_pPlayDlg->SetRealParent((void *)this);
		m_pPlayDlg->Create(IDD_PLAYDLG, GetDesktopWindow());
	}
	
	// 把主窗口打开的文件信息传到子窗口
	int ret = m_pPlayDlg->SetVideoInfo(m_strFileUrl, m_cVideoInfo.width, m_cVideoInfo.height, m_nSliceIndex, m_cVideoInfo.max_framerate);
	if (ret < 0) return;

	m_pPlayDlg->SetListInfo(&m_vNalTypeVector);

	if (!m_pPlayDlg->IsWindowVisible())
		m_pPlayDlg->ShowWindow(SW_SHOW);

	int sIndex = FindPrevPPS(0);

	if (sIndex >= 0)
	{
		m_pPlayDlg->SeekPlay(m_vNalTypeVector[sIndex].offset, (m_vNalTypeVector[sIndex].sliceIndex));
	}

	m_pPlayDlg->SetFocus();


    //m_pPlayDlg->ShowFirstFrame(); // 注：在调用ShowWindow后再显示第一帧图像
}


void BsrVideoAnalyzerDlg::OnNMDblclkH264Nallist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	POSITION ps;
	int nIndex;
	int ret = 0;

	if (m_strFileUrl.IsEmpty() == TRUE)
		return;
	if (!m_fCanPlay)
	{
		MessageBox("Parsing NALU, wait a momnent...");
		return;
	}

	ps = m_h264NalList.GetFirstSelectedItemPosition();
	nIndex = m_h264NalList.GetNextSelectedItem(ps);

	ProcListSelectedItem(nIndex);

	if (!m_pPlayDlg)
	{
		OnPlayDlg();
	}
	else
		if (!m_pPlayDlg->IsWindowVisible())
		{
			int ret = m_pPlayDlg->SetVideoInfo(m_strFileUrl, m_cVideoInfo.width, m_cVideoInfo.height, m_nSliceIndex, m_cVideoInfo.max_framerate);
			if (ret < 0) return;

			m_pPlayDlg->SetListInfo(&m_vNalTypeVector);

			m_pPlayDlg->ShowWindow(SW_SHOW);

		}

	int sIndex = FindPrevPPS(nIndex);

	if (sIndex >= 0)
	{
		if (sIndex >= nIndex)
		{
			m_pPlayDlg->SeekPlay(m_vNalTypeVector[sIndex].offset, (m_vNalTypeVector[sIndex].sliceIndex));
		}
		else
		{
			m_pPlayDlg->SeekPlay(m_vNalTypeVector[sIndex].offset,
				((m_vNalTypeVector[nIndex].sliceIndex - (m_vNalTypeVector[sIndex].sliceIndex)) << 20) | (m_vNalTypeVector[sIndex].sliceIndex));
		}

		m_pPlayDlg->SetFocus();
	}

	*pResult = 0;
}


void BsrVideoAnalyzerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值


	m_bWillExit = TRUE;
	m_cParser.setWillBreak();
	SetEvent(m_hFileLock);

	if (!m_fCanPlay) return;

	for (;;)
	{
		DWORD exitCode1 = 0;
		GetExitCodeThread(m_hFileThread, &exitCode1);
		if (exitCode1 == STILL_ACTIVE)
			Sleep(10);
		if (exitCode1 != STILL_ACTIVE)
			break;
	}

	EndDialog(IDCANCEL);	//关闭窗口

	CDialog::OnClose();
}

BOOL BsrVideoAnalyzerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_ESCAPE:
			break;
		case 'O':
		case 'F':
			if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000))
			{
				OnFileOpen();
			}
			break;
		case 'H':
			if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000))
			{
				OnUsage();
			}
			break;
		case 'Q':
			if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000))
			{
				OnClose();
			}
			break;
		case 'P':
			if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000))
			{
				OnPlayDlg();
			}
			break;
		default:
			break;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}



void BsrVideoAnalyzerDlg::OnLvnGetdispinfoH264Nallist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码


	LV_ITEM* pItem = &(pDispInfo)->item;
	int  iItemIndex = pItem->iItem;   //行号  
	int	 col = pItem->iSubItem;   //列号  


	NALU_t* nalu = &m_vNalTypeVector[iItemIndex];


	if (pItem->mask & LVIF_TEXT)
	{
		switch (col)
		{
		case 0: // 序号
			sprintf(pItem->pszText, _T("%d"), nalu->num + 1);
			break;
		case 1:
			sprintf(pItem->pszText, _T("%08X"), nalu->offset);
			break;
		case 2: // 长度
			sprintf(pItem->pszText, _T("%d"), nalu->len);
			break;
		case 3: // 起始码
			sprintf(pItem->pszText, _T("%s"), nalu->startcodeBuffer);
			break;
		case 4: //
			lstrcpyn(pItem->pszText, nalu->strType, pItem->cchTextMax);
			break;
		case 5:
			if (nalu->errOffset > 0)
				sprintf(pItem->pszText, _T("ERR-%s"), nalu->strInfo);
			else
				lstrcpyn(pItem->pszText, nalu->strInfo, pItem->cchTextMax);
			break;
		default:
			break;
		}
	}


	*pResult = 0;
}


void BsrVideoAnalyzerDlg::OnUsage()
{
	// TODO: 在此添加命令处理程序代码
	char* help = "说明：\r\n"
		"对于通用的媒体文件，在打开时，会将其中的视频流数据分离，\r\n"
		"形成一个单独的文件，文件名命名是在源文件名后面加上.h264或.h265，\r\n"
		"所以多次查看分析结果时，可以直接打开这些分离的文件。\r\n"
		"支持分析大于4G的文件。\r\n\r\n"
		"使用：\r\n"
		"1、	打开文件：file菜单、Ctrl+F、将文件拖到程序界面\r\n"
		"2、	程序自动解析mp4、mov、h264、h265以及bsr文件\r\n"
		"3、	程序将显示流的基本信息，双击某一项NAL，可显示详细信息\r\n"
		"4、	打开文件后，执行菜单Play、Ctrl+P、双击信息列表等操作，\r\n"
		"	程序将弹出播放窗口，如果是双击的信息列表，则显示的视\r\n"
		"	频是从双击的位置开始，其它则从第一帧视频开始\r\n"
		"5、	播放窗口，MP按钮显示宏块预测类型，QP显示量化值\r\n"
		"	QC按钮用颜色标识量化值，相同的颜色越亮，量化值越低\r\n"
		"6、	播放窗口，Ctrl+'+'/'-'进行放大、缩小\r\n"
		"7、	可另存为.mp4.mov.yuv.bmp文件,.bmp保存当前显示的帧\r\n"
		"8、	另存为yuv文件时,在P的旁边设置起止帧,S旁边设置图像大小,\r\n"
		"	默认是保存当前显示帧的yuv图片文件\r\n"
		"9、	保存连续的yuv帧序列时，必须在弹出文件保存对话框时更改\r\n"
		"	文件名：在.yuv前面添加%\r\n"
		"	在保存yuv序列时，自动按1G大小进行分段，文件名自带序号\r\n"
		"\r\n仅支持分析H264/H265编码的视频流，其它码流无法分析\r\n";
	MessageBox(help);
}
