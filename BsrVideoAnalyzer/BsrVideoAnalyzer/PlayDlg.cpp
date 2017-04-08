// PlayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BsrVideoAnalyzer.h"
#include "PlayDlg.h"
#include "afxdialogex.h"
#include "bmp_utils.h"

// CPlayDlg dialog

IMPLEMENT_DYNAMIC(CPlayDlg, CDialogEx)

#define BUTTON_HEIGHT	28
#define BOTTOM_BORDER	5
#define PROGRESS_BAR_HEIGHT 18
#define OTHER_HEIGHT (BUTTON_HEIGHT+BOTTOM_BORDER+PROGRESS_BAR_HEIGHT)

CPlayDlg::CPlayDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CPlayDlg::IDD, pParent)
{
    m_fShowBlack = FALSE;
    m_fPlayed = FALSE;
    m_fClosed = TRUE;
    m_fLoop = FALSE;
    m_fInit = FALSE;
    m_nWidth = 0;
    m_nHeight = 0;
    m_nTotalFrame = 0;
    m_nFrameCount = 0;
    m_fFps = 0.0;
    m_strPathName.Empty();
    m_pParentWnd = NULL;

	m_bDisplayQP = FALSE;
	m_bDisplayQC = FALSE;
	m_bDisplayMB = FALSE;
	m_nDisplaySize = 0;

	m_pListInfo = NULL;
	m_nDisplaySizeBeforeZoom = 0;
	InitSDL();
}

CPlayDlg::~CPlayDlg()
{

	EndSDL();
	
	if (hSDL_DLL)
	{
		FreeLibrary(hSDL_DLL); 
		hSDL_DLL = NULL;
	}
}

void CPlayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BT_PLAY, m_bPlay);
	DDX_Control(pDX, IDC_BT_SAVE, m_bSaveFrame);
	DDX_Control(pDX, IDC_BT_STOP, m_bStop);
	DDX_Control(pDX, IDC_BT_LAST, m_bLastFrame);
	DDX_Control(pDX, IDC_BT_NEXT, m_bNextFrame);
	DDX_Control(pDX, IDC_EDIT_WIDTH, m_editWidth);
	DDX_Control(pDX, IDC_EDIT_HEIGHT, m_editHeight);
	DDX_Control(pDX, IDC_EDIT_FRAME_BEGIN, m_editFrameB);
	DDX_Control(pDX, IDC_EDIT_FRAME_END, m_editFrameE);
	DDX_Control(pDX, IDC_SLIDER_PLAY, m_sliderPlay);
}

BEGIN_MESSAGE_MAP(CPlayDlg, CDialogEx)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_BT_PLAY, &CPlayDlg::OnBnClickedBtPlay)
    ON_WM_CLOSE()
    ON_WM_TIMER()
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_BT_NEXT, &CPlayDlg::OnBnClickedBtNext)
    ON_BN_CLICKED(IDC_BT_SAVE, &CPlayDlg::OnBnClickedBtSave)
    ON_BN_CLICKED(IDC_BT_STOP, &CPlayDlg::OnBnClickedBtStop)
    ON_BN_CLICKED(IDC_CK_LOOP, &CPlayDlg::OnBnClickedCkLoop)
	ON_BN_CLICKED(IDC_BUTTON_100, &CPlayDlg::OnBnClickedButtonDisplay100)
	ON_BN_CLICKED(IDC_BUTTON_50, &CPlayDlg::OnBnClickedButtonDisplay50)
	ON_BN_CLICKED(IDC_BUTTON_QP, &CPlayDlg::OnBnClickedButtonQp)
	ON_BN_CLICKED(IDC_BUTTON_QP2, &CPlayDlg::OnBnClickedButtonQp2)
	ON_BN_CLICKED(IDC_BUTTON_MP, &CPlayDlg::OnBnClickedButtonMb)
	ON_BN_CLICKED(IDC_BUTTON_75, &CPlayDlg::OnBnClickedButtonDisplay75)
	ON_BN_CLICKED(IDC_BT_LAST, &CPlayDlg::OnBnClickedBtLast)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify) //添加消息
	ON_WM_HSCROLL()
	ON_WM_SYSCOMMAND()
END_MESSAGE_MAP()


// CPlayDlg message handlers
BOOL CPlayDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

	CWnd *pWnd = GetDlgItem(IDC_VIDEO);
	
	
	
	Graphics grf(pWnd->m_hWnd);
	grf.Clear(Color::LightGray);

    m_vStartX.resize(2);
    m_vStartX[0].push_back(IDC_BT_PLAY);
    m_vStartX[0].push_back(IDC_BT_STOP);
	m_vStartX[0].push_back(IDC_BT_LAST);
	m_vStartX[0].push_back(IDC_BT_NEXT);
    m_vStartX[0].push_back(IDC_BT_SAVE);
    m_vStartX[0].push_back(IDC_CK_LOOP);
	m_vStartX[0].push_back(IDC_EDIT_WIDTH);
	m_vStartX[0].push_back(IDC_EDIT_HEIGHT);
	m_vStartX[0].push_back(IDC_EDIT_FRAME_BEGIN);
	m_vStartX[0].push_back(IDC_EDIT_FRAME_END);
	m_vStartX[0].push_back(IDC_BUTTON_100);
	m_vStartX[0].push_back(IDC_BUTTON_75);
	m_vStartX[0].push_back(IDC_BUTTON_50);
	m_vStartX[0].push_back(IDC_BUTTON_QP);
	m_vStartX[0].push_back(IDC_BUTTON_QP2);
	m_vStartX[0].push_back(IDC_BUTTON_MP);
	m_vStartX[0].push_back(IDC_STATIC_FRAME);
	m_vStartX[0].push_back(IDC_STATIC_SIZE);
	m_vStartX[0].push_back(IDC_S_DEBUG);

    CRect rect;
    for (unsigned int i = 0; i < m_vStartX[0].size(); i++)
    {
        GetDlgItem(m_vStartX[0][i])->GetWindowRect(rect);
        ScreenToClient(rect);
        m_vStartX[1].push_back(rect.left);
    }

    // 贴图
    m_bPlay.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_PLAY)));
    m_bPlay.EnableWindow(TRUE);
    m_bStop.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_STOP)));
    m_bStop.EnableWindow(TRUE);
    m_bNextFrame.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_NEXT)));
    m_bNextFrame.EnableWindow(TRUE);
	m_bLastFrame.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_LAST)));
	m_bLastFrame.EnableWindow(TRUE);
	m_bSaveFrame.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_SAVE)));
    m_bSaveFrame.EnableWindow(TRUE);

    m_fInit = TRUE;

	m_tooltip.Create(this);// GetDlgItem(IDC_SLIDER_PLAY));
	m_tooltip.SetMaxTipWidth(600);

	m_tooltip.AddTool(GetDlgItem(IDC_SLIDER_PLAY), LPSTR_TEXTCALLBACK); //表示tip内容可以动态改变！！
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_QP), LPSTR_TEXTCALLBACK); 
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_QP2), LPSTR_TEXTCALLBACK);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_MP), LPSTR_TEXTCALLBACK);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_100), LPSTR_TEXTCALLBACK);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_75), LPSTR_TEXTCALLBACK);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON_50), LPSTR_TEXTCALLBACK);
	m_tooltip.AddTool(GetDlgItem(IDC_EDIT_WIDTH), LPSTR_TEXTCALLBACK);
	m_tooltip.AddTool(GetDlgItem(IDC_EDIT_HEIGHT), LPSTR_TEXTCALLBACK);
	m_tooltip.AddTool(GetDlgItem(IDC_EDIT_FRAME_BEGIN), LPSTR_TEXTCALLBACK);
	m_tooltip.AddTool(GetDlgItem(IDC_EDIT_FRAME_END), LPSTR_TEXTCALLBACK);
	m_tooltip.AddTool(GetDlgItem(IDC_BT_SAVE), LPSTR_TEXTCALLBACK);


	//setting the original playing size:1280x720
	SetPlayWindowSize();

    return TRUE;  // return TRUE unless you set the focus to a control

}

char *dll_name[] = {
	"SDL_Init",
	"SDL_CreateWindowFrom",
	"SDL_CreateRenderer",
	"SDL_SetHint",
	"SDL_CreateTexture",
	"SDL_DestroyTexture",
	"SDL_UpdateYUVTexture",
	"SDL_GetWindowSize",
	"SDL_RenderClear",
	"SDL_RenderCopy",
	"SDL_RenderPresent",
	"SDL_DestroyRenderer",
	"SDL_DestroyWindow",
	"SDL_Quit",
	"SDL_GetError",
	"SDL_SetWindowBrightness",
	"SDL_GetWindowBrightness",
	"SDL_SetTextureColorMod",
	"SDL_GetTextureColorMod",
	"SDL_LockTexture",
	"SDL_UnlockTexture",
	"SDL_AddTimer", "SDL_RemoveTimer",
	"SDL_SetTextureBlendMode",
	"SDL_SetRenderTarget",
	"SDL_SetRenderDrawBlendMode",
	"SDL_SetRenderDrawColor",
	"SDL_RenderFillRect",
	"SDL_RenderDrawRect"

};

void CPlayDlg::InitSDL()
{
	m_pSdlWindow = NULL;
	m_pSdlTexture = NULL;
	m_pSdlRenderer = NULL;

	m_nPixelFormat = SDL_PIXELFORMAT_UNKNOWN;
	m_nPitch = 0;
	m_nSdlWidth = 0;
	m_nSdlHeight = 0;

	x_block = 0;
	y_block = 0;


	m_pLockBuf = NULL;

	char cTempDir[512]; GetTempPath(512, cTempDir);
	sprintf(cTempDir, "%s%s", cTempDir, "sdl2.xxx");
	hSDL_DLL = LoadLibrary(_T(cTempDir));

	SDL_FUNC_LOAD_ALL;
}

#define BLOCK_SIZE	64
#define ALIGN64(x) (((x)+63)-((x)+63)%64)
int CPlayDlg::StartSDL(int nId, int image_w, int image_h)
{
	if (!SDL_Init) return -1;
	if (SDL_Init(SDL_INIT_VIDEO)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	m_pSdlWindow = SDL_CreateWindowFrom((void *)(GetDlgItem(nId)->GetSafeHwnd()));
	if (!m_pSdlWindow) {
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}

	m_pSdlRenderer = SDL_CreateRenderer(m_pSdlWindow, -1, 0);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");


	m_nSdlWidth = ALIGN64(image_w);
	m_nSdlHeight = image_h;

	m_pSdlTexture = SDL_CreateTexture(m_pSdlRenderer, m_nPixelFormat, SDL_TEXTUREACCESS_STREAMING, m_nSdlWidth, m_nSdlHeight);
	m_pSdlOverlayTexture = SDL_CreateTexture(m_pSdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, BLOCK_SIZE, BLOCK_SIZE);

	return 0;
}

int CPlayDlg::UpdateSDL()
{
	if (m_pSdlTexture != NULL)
		SDL_DestroyTexture(m_pSdlTexture);
	if (m_pSdlRenderer != NULL)
		m_pSdlTexture = SDL_CreateTexture(m_pSdlRenderer, m_nPixelFormat, SDL_TEXTUREACCESS_STREAMING, m_nSdlWidth, m_nSdlHeight);
	

	return 0;
}

void CPlayDlg::OverlayTexture(SDL_Renderer *renderer, SDL_Texture *texture, int r, int g, int b, int a)
{
	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_RenderFillRect(renderer, NULL);

	SDL_Rect sdlRect;
	sdlRect.x = 0; sdlRect.y = 0;
	sdlRect.w = BLOCK_SIZE; sdlRect.h = BLOCK_SIZE;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(renderer, &sdlRect);
}

void CPlayDlg::PrepareForRendering(SDL_Renderer *renderer)
{
	SDL_SetRenderTarget(renderer, NULL);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
}

int CPlayDlg::ShowPictureSDL(const BYTE **buffer, int pitch)
{
	SDL_Rect sdlRect, texRect;

	sdlRect.x = 0;
	sdlRect.y = 0;

	if (ALIGN64(m_nWidth) != m_nSdlWidth || m_nHeight != m_nSdlHeight)
	{
		m_nSdlWidth = ALIGN64(m_nWidth);
		m_nSdlHeight = m_nHeight;
		UpdateSDL();
	}
	//if the picture data of yuv is continuous:
	//SDL_UpdateTexture(m_pSdlTexture, NULL, buffer, pitch);
	//otherwise:
	//SDL_UpdateYUVTexture(m_pSdlTexture, NULL, buffer[0], pitch, buffer[1], pitch / 2, buffer[2], pitch / 2);
	//==> OR
	//uint8_t *lockBuf;
	//int lockPitch;
	//SDL_LockTexture(m_pSdlTexture, NULL, (void **)&lockBuf, &lockPitch);
	//memcpy(lockBuf, buffer[0], m_nSdlWidth * m_nSdlHeight);
	//memcpy(lockBuf + m_nSdlWidth * m_nSdlHeight, buffer[1], m_nSdlWidth * m_nSdlHeight / 4);
	//memcpy(lockBuf + m_nSdlWidth * m_nSdlHeight * 5 / 4, buffer[2], m_nSdlWidth * m_nSdlHeight / 4);
	//lockPitch = pitch;
	SDL_UnlockTexture(m_pSdlTexture);
	SDL_GetWindowSize(m_pSdlWindow, &(sdlRect.w), &sdlRect.h);

	SDL_RenderClear(m_pSdlRenderer);



	OverlayTexture(m_pSdlRenderer, m_pSdlOverlayTexture, 0, 122, 204, 144);
	PrepareForRendering(m_pSdlRenderer);

	texRect.x = texRect.y = 0;
	texRect.w = m_nWidth;
	texRect.h = m_nHeight;
	SDL_RenderCopy(m_pSdlRenderer, m_pSdlTexture, &texRect, &sdlRect);
	sdlRect.x = x_block; sdlRect.y = y_block;
	sdlRect.w = BLOCK_SIZE; sdlRect.h = BLOCK_SIZE;
	SDL_SetTextureBlendMode(m_pSdlOverlayTexture, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(m_pSdlRenderer, m_pSdlOverlayTexture, NULL, &sdlRect);


	//SDL_RenderCopy(m_pSdlRenderer, m_pSdlTexture, NULL, &sdlRect);
	SDL_RenderPresent(m_pSdlRenderer);

	return 0;
}

int CPlayDlg::GetSDLLockBuffer(void **pLockBuf, int *pitch)
{
	if (ALIGN64(m_nWidth) != m_nSdlWidth || m_nHeight != m_nSdlHeight)
	{
		m_nSdlWidth = ALIGN64(m_nWidth);
		m_nSdlHeight = m_nHeight;
		UpdateSDL();
	}

	SDL_LockTexture(m_pSdlTexture, NULL, pLockBuf, pitch);

	return 0;
}

int CPlayDlg::RefreshSDLTexture()
{
	if (!m_pSdlWindow || !m_pSdlRenderer || !m_pSdlTexture) return -1;

	SDL_Rect sdlRect;

	sdlRect.x = 0;
	sdlRect.y = 0;
	SDL_GetWindowSize(m_pSdlWindow, &(sdlRect.w), &sdlRect.h);

	SDL_RenderClear(m_pSdlRenderer);
	SDL_RenderCopy(m_pSdlRenderer, m_pSdlTexture, NULL, &sdlRect);
	SDL_RenderPresent(m_pSdlRenderer);

	return 0;
}

int CPlayDlg::EndSDL()
{
	if (m_pSdlTexture != NULL)
	{
		SDL_DestroyTexture(m_pSdlTexture);
		m_pSdlTexture = NULL;
	}

	if (m_pSdlRenderer != NULL)
	{
		SDL_DestroyRenderer(m_pSdlRenderer);
		m_pSdlRenderer = NULL;
	}

	if (m_pSdlOverlayTexture != NULL)
	{
		SDL_DestroyTexture(m_pSdlOverlayTexture);
		m_pSdlOverlayTexture = NULL;
	}

	if (NULL != m_pSdlWindow)
	{
		SDL_DestroyWindow(m_pSdlWindow);
		m_pSdlWindow = NULL;
	}

	m_pLockBuf = NULL;

	if (SDL_QUIT)
		SDL_Quit();

	m_nPixelFormat = SDL_PIXELFORMAT_UNKNOWN;
	m_nPitch = 0;
	return 0;
}

void CPlayDlg::SetPlayWindowSize(int percent)
{

	if (percent == 0)
	{
		RECT rc;
		::SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rc, 0);
		percent = 100;
		int nW = m_nWidth*percent / 100, nH = m_nHeight*percent / 100 + OTHER_HEIGHT;

		do {
			if (nW + 16 <= rc.right + 1 && nH + 50 <= rc.bottom + 1) break;
			percent -= 10;
			nW = m_nWidth*percent / 100; nH = m_nHeight*percent / 100 + OTHER_HEIGHT;
		} while (1);
	}

	if (percent < 30) percent = 30;
	if (percent > 100 && m_nWidth >= 1280) percent = 100;
	if (percent > 200) percent = 200;

	if (percent == m_nDisplaySize) return;

	m_nDisplaySize = percent;
	int nW = m_nWidth*m_nDisplaySize / 100, nH = m_nHeight*m_nDisplaySize / 100;
	int addX = 0, addY = 0;
	CRect rect0, rect1;
	CWnd *pWnd = GetDlgItem(IDC_VIDEO);
	pWnd->GetClientRect(rect0);


	addX = nW - (rect0.right - rect0.left + 1);
	GetClientRect(rect0);
	addY = (nH + OTHER_HEIGHT) - (rect0.bottom - rect0.top + 1);

	GetWindowRect(rect1);

	rect1.left -= addX / 2;
	rect1.top -= addY / 2;
	rect1.right += addX / 2;
	rect1.bottom += addY / 2;

	MoveWindow(rect1, TRUE);

	pWnd->GetClientRect(rect0);
	GetWindowRect(rect1);

	rect1.right += nW - (rect0.right - rect0.left + 1);
	rect1.bottom += nH - (rect0.bottom - rect0.top + 1);

	CPoint xx = CPoint(0, 0);
	::ClientToScreen(this->m_hWnd, &xx);


	rect1.right = (rect1.left < 0-xx.x) ? rect1.right - rect1.left : rect1.right;
	rect1.left = (rect1.left < 0 - xx.x) ? xx.x : rect1.left;
	rect1.bottom = (rect1.top < 0) ? rect1.bottom - rect1.top : rect1.bottom;
	rect1.top = (rect1.top < 0) ? 0 : rect1.top;

	MoveWindow(rect1, TRUE);
	InvalidateRect(NULL, TRUE);

	UpdateTitle();
	UpdateData();
}

void CPlayDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting

	if(m_pLockBuf)
		Show(m_nWidth, m_nHeight);
}

void CPlayDlg::Show(int nWidth, int nHeight)
{
	if (m_pSdlWindow == NULL)
	{
		SetSDLMode();
		if (StartSDL(IDC_VIDEO, nWidth, nHeight) == -1)
			return;
	}

	if (m_pSdlWindow)
	{
		ShowPictureSDL(NULL, m_nPitch);

		return;
	}

}

int CPlayDlg::SetVideoInfo(CString strFileName, int nWidth, int nHeight, int nTotalFrame, float nFps)
{
    int ret = 0;
	BOOL bSizeChanged = ((m_nWidth != nWidth) || (m_nHeight != nHeight));
    m_strPathName = strFileName;
    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_nTotalFrame = nTotalFrame;
    m_fFps = nFps;

    if (m_fFps <= 0 ) m_fFps = 25.0;

	m_sliderPlay.SetRange(1, m_nTotalFrame, 1);
	m_sliderPlay.SetPageSize(0);

	if (bSizeChanged)
	{
		m_nDisplaySize = 0;
		SetPlayWindowSize();
	}

    m_fPlayed = TRUE;
    m_bPlay.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_PLAY)));

	if (m_strPathName.IsEmpty())
    {
        MessageBox("Sorry, you open no file...");
        return -1;
    }
    if (m_fClosed)
    {
        ret = m_cDecoder.openVideoFile(m_strPathName.GetBuffer());
		m_strPathName.ReleaseBuffer();
        if (ret < 0)
        {
            MessageBox("Sorry, open video decoder failed.");
            return -1;
        }
        m_fClosed = FALSE;
        m_nFrameCount = 0;
    }

    return 0;
}

void CPlayDlg::ShowFirstFrame()
{
    OnBnClickedBtNext();
}

char *strFrameType[] =
{
	"NA",
	"I-Frame",
	"P-Frame",
	"B-Frame",
	"S-Frame",
	"SI",
	"SP",
	"BI"
};

int CPlayDlg::ShowingFrame()
{
    int ret = 0;
	//for SDL
	if (m_pSdlWindow == NULL)
	{
		SetSDLMode();
		if (StartSDL(IDC_VIDEO, m_nWidth, m_nHeight) == -1)
			return -1;
	}

	if (!m_cDecoder.NeedFlushCodec())
	{
		//for sdl
		GetSDLLockBuffer((void **)&m_pLockBuf, &m_nLockPitch);
		ret = m_cDecoder.getFrame(&m_pLockBuf, NULL, &m_nPitch);
	}

    if ( ret > 0)
    {
		m_nFrameCount++;
		//m_nFrameCount += m_cDecoder.GetErrFrames(); m_cDecoder.SetErrFrames();
		UpdateTitle();
		Show(m_nWidth, m_nHeight);
    }
    else if(ret == 0)
    {
		//for sdl
		GetSDLLockBuffer((void **)&m_pLockBuf, &m_nLockPitch);
		
		ret = m_cDecoder.getSkippedFrame(&m_pLockBuf, NULL, &m_nPitch);

        if ( ret > 0)
        {
			m_nFrameCount++;
			UpdateTitle();
			Show(m_nWidth, m_nHeight);
        }
    }

	return ret;
}

int CPlayDlg::SeekVideo(int64_t pos)
{
	int ret = 0;

	ret = m_cDecoder.seek(pos);

	return ret;
}

// 文件名带%d %05d等表示保存所有图片
static bool IsSingleFile(const char* filename)
{
    std::string pathname = filename;

    std::string::size_type pos = pathname.find_first_of('%');
    if (pos == std::string::npos)
    {
        return true;
    }
	/*
    std::string::size_type pos1 = pathname.find_first_of('d', pos);

    if (pos1 == std::string::npos)
    {
        return true;
    }
	*/
    return false;
}

#define MAX_YUV_FRAMES	200000
int CPlayDlg::SaveYUVFile(const char* pFileName)
{
    int ret = 0;
    if (IsSingleFile(pFileName))
    {
        m_cDecoder.writeYUVFile(pFileName);
    }
    else
    {
		int sPos = 0;
		int ePos = 0;
		int ww = 0;
		int hh = 0;
		int len = 0;

		char str_data[30];
		memset(str_data, 0, 30);
		len = m_editFrameB.LineLength();
		m_editFrameB.GetLine(0, str_data, len);
		sPos = atoi(str_data);

		memset(str_data, 0, 30);
		len = m_editFrameE.LineLength();
		m_editFrameE.GetLine(0, str_data, len);
		ePos = atoi(str_data);

		memset(str_data, 0, 30);
		len = m_editWidth.LineLength();
		m_editWidth.GetLine(0, str_data, len);
		ww = atoi(str_data);

		memset(str_data, 0, 30);
		len = m_editHeight.LineLength();
		m_editHeight.GetLine(0, str_data, len);
		hh = atoi(str_data);

		if ((ww <= 64 || hh <= 64 || ww > m_nWidth || hh > m_nHeight) && !(ww == 0 && hh == 0))
			return -1;
		
		if (ePos < sPos || ePos < 0 || sPos < 0 || ePos - sPos>MAX_YUV_FRAMES || sPos > m_nTotalFrame || ePos > m_nTotalFrame)
			return -2;


		BsrVideoDecoder foo;
		foo.openVideoFile(m_strPathName);
		char szFileName[512] = { 0 };
		char tmpFileName[512] = { 0 };
		char *szSuffix = "from_%d_to_%d(%dfs)_seg#%04d.yuv";
		sprintf(tmpFileName, "%s", pFileName);

		for (unsigned int i = 0; i < strlen(tmpFileName); i++)
		{
			if (*(tmpFileName + i) == '%')
			{
				sprintf(tmpFileName + i, "%s", szSuffix);
				break;
			}
		}

		int wFileSegmentNo = 1;
		sprintf(szFileName, tmpFileName, sPos, ePos, ePos - sPos + 1, wFileSegmentNo);

		FILE *fp = fopen(szFileName, "wb");
		if (fp == NULL) return -4;


		int cnt = 1;
		fpos_t seekOffset = 0;
		int retSeek= SeekToIFrame(sPos, &cnt, &seekOffset);
		if (retSeek == -1) cnt = 1;
		foo.seek(seekOffset);

		char copyInfo[128];

		sprintf(copyInfo, "Started saving ...");

		DebugInfo(copyInfo);

        while (foo.getFrame() > 0)
        {
			if(cnt >= sPos && cnt <= ePos)
				foo.writeYUVFile(fp, ww, hh);
            //Sleep(1);
			cnt++;

			if(cnt%100 == 0)
			{ 
				sprintf(copyInfo, "Started saving: ( %d / %d )", cnt - sPos + 1, ePos - sPos + 1);
				DebugInfo(copyInfo);
			}

			if (cnt > ePos || (cnt-sPos+1) > MAX_YUV_FRAMES) break;

			fpos_t fileSize = 0;
			fgetpos(fp, &fileSize);
			if (fileSize > 1024 * 1024 * 1024)
			{
				fclose(fp);
				wFileSegmentNo++;
				sprintf(szFileName, tmpFileName, sPos, ePos, ePos - sPos + 1, wFileSegmentNo);

				FILE *fp = fopen(szFileName, "wb");
				if (fp == NULL) return -5;
			}
        }
        while (foo.getSkippedFrame() > 0)
        {
			if (cnt >= sPos && cnt <= ePos)
				foo.writeYUVFile(fp, ww, hh);
			//Sleep(1);
			cnt++;

			if (cnt % 100 == 0)
			{
				sprintf(copyInfo, "Started saving: ( %d / %d )", cnt - sPos + 1, ePos - sPos + 1);
				DebugInfo(copyInfo);
			}

			if (cnt > ePos || (cnt - sPos + 1) > MAX_YUV_FRAMES) break;
		}

		fclose(fp);

    }

    return ret;
}

int CPlayDlg::SaveBMPFile(const char* pFileName)
{
    int ret = 0;
	if (IsSingleFile(pFileName))
	{
		m_cDecoder.writeBMPFile2(pFileName);
	}
	else
		ret = -3;

	return ret;
}

int CPlayDlg::SaveVideoFile(const char* pFileName)
{
	int ret = 0;
	H264BS2Video cSaveVideo;

	ret = cSaveVideo.openVideoFile(m_strPathName.GetBuffer(), pFileName, m_nWidth, m_nHeight);
	if (ret < 0)
	{
		MessageBox("Open video file failed");
		return -1;
	}
	ret = cSaveVideo.writeFrame();
	if (ret < 0)
	{
		MessageBox("Write to video file failed");
		return -1;
	}

	cSaveVideo.close();

	return 0;
}

// 窗口关闭
void CPlayDlg::OnClose()
{
    OnBnClickedBtStop();

	ShowWindow(SW_SHOWNORMAL);

    CDialogEx::OnClose();
	parentWnd->SetFocus();
}


void CPlayDlg::Pause()
{
    KillTimer(1);

    m_fPlayed = TRUE;
    m_bPlay.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_PLAY)));

}

void CPlayDlg::CloseVideo()
{
    if (m_fClosed == TRUE)
    {
        m_nFrameCount = 0;
		m_cDecoder.closeVideoFile();
    }  
}

// 使用关闭文件再打开的方式，是因为没有封装ffmpeg的复位操作
void CPlayDlg::ReOpenVideo()
{
    CloseVideo();

    if (m_fClosed == TRUE)
    {
        if (m_cDecoder.openVideoFile(m_strPathName.GetBuffer()) < 0)
        {
            MessageBox("Sorry, open video decoder failed.");
            return;
        }

        m_fClosed = FALSE;
    }
}

void CPlayDlg::OnBnClickedBtStop()
{
    Pause();

    m_fClosed = TRUE;

    CloseVideo();

	UpdateTitle();
}

void CPlayDlg::SeekPlay(int64_t pos, int index)
{
	Pause();

	m_fClosed = TRUE;

	ReOpenVideo();
	SeekVideo(pos);

	int step = index >> 20;

	if (step > 0)
		m_cDecoder.SetSkipStep(step);
	
	m_nFrameCount = (index&0xfffff)+step;

	UpdateTitle();

	ShowingFrame();
}

#define IS_IFRAME(x) \
( \
	((x).nalType == NAL_UNIT_TYPE_CODED_SLICE_IDR) || \
	(((x).nalType == NAL_UNIT_TYPE_CODED_SLICE_NON_IDR) && ((x).sliceType == 2 || (x).sliceType == 7) ) \
)

int CPlayDlg::SeekToIFrame(int begFrame, int *posIFrame, fpos_t *offset)
{
	unsigned int i = 0;
	int gop = -1;
	int gopFirstPos = 0;
	*posIFrame = -1;
	for (i = 0; i <= m_pListInfo->size() - 1; i++)
	{
		
		gopFirstPos = (gop != (*m_pListInfo)[i].gop) ? (*m_pListInfo)[i].num : gopFirstPos;
		gop = (*m_pListInfo)[i].gop;

		if ((*m_pListInfo)[i].sliceIndex == begFrame-1)
		{
			*posIFrame = (*m_pListInfo)[gopFirstPos].sliceIndex + 1;
			*offset = ((*m_pListInfo)[gopFirstPos].offset);
			break;
		}
	}

	if (*posIFrame == -1) {
		*offset = 0;
		return -1;
	}

	return 0;
}

int CPlayDlg::SeekToPrevIFrame(BsrVideoDecoder *dec, int frameNo, int *newPos)
{
	if ((*m_pListInfo)[0].sliceIndex == frameNo - 1) return -1;
#if 0
	unsigned int i = 0;
	int gop = -1;
	fpos_t offset = 0;
	int gopFirstPos = -1;
	int gopFirstPos_last = 0;
	*newPos = -1;
	for (i = 0; i <= m_pListInfo->size() - 1; i++)
	{
		gopFirstPos_last = (gop != (*m_pListInfo)[i].gop) ? gopFirstPos : gopFirstPos_last;
		gopFirstPos = (gop != (*m_pListInfo)[i].gop) ? i : gopFirstPos;
		gop = (*m_pListInfo)[i].gop;

		if ((*m_pListInfo)[i].sliceIndex >= (unsigned int)frameNo - 1)
		{
			break;
		}
	}

	if (gop < 0) return -1;

	if ((*m_pListInfo)[gopFirstPos].sliceIndex < (unsigned int)frameNo - 1)
	{
		*newPos = (*m_pListInfo)[gopFirstPos].sliceIndex + 1;
		offset = ((*m_pListInfo)[gopFirstPos].offset);
	}
	else if(gopFirstPos_last >= 0)
	{
		*newPos = (*m_pListInfo)[gopFirstPos_last].sliceIndex + 1;
		offset = ((*m_pListInfo)[gopFirstPos_last].offset);
	}
#else
	int i = 0;
	int gop = -1;
	fpos_t offset = 0;
	int gopFirstPos = -1;
	int gopFirstPos_last = 0;
	*newPos = -1;
	for (i = 0; i <= m_pListInfo->size() - 1; i++)
	{
		gopFirstPos_last = (gop != (*m_pListInfo)[i].gop) ? gopFirstPos : gopFirstPos_last;
		gopFirstPos = (gop != (*m_pListInfo)[i].gop) ? i : gopFirstPos;
		gop = (*m_pListInfo)[i].gop;

		if ((*m_pListInfo)[i].sliceIndex >= (unsigned int)frameNo - 1)
		{
			break;
		}
	}

	if (gop < 0) return -1;

	for (--i; i >= 0; i--)
	{
		if (IS_IFRAME((*m_pListInfo)[i]))
		{
			*newPos = (*m_pListInfo)[i].sliceIndex + 1;
			offset = ((*m_pListInfo)[i].offset);
			break;
		}
	}

#endif
	if (*newPos == -1) {
		return -1;
	}

	if (m_fClosed) 
		ReOpenVideo();

	while (dec->getSkippedFrame() > 0) {};

	dec->seek(offset);

	while (dec->getFrame() <= 0) {};

	return 0;
}

int CPlayDlg::SeekToNextIFrame(BsrVideoDecoder *dec, int frameNo, int *newPos)
{
	unsigned int i = 0;
	int gop = -1;
	fpos_t offset = 0;
	*newPos = -1;
	for (i = 0; i <= m_pListInfo->size() - 1; i++)
	{

		if ((*m_pListInfo)[i].sliceIndex == frameNo - 1)
		{
			gop = (*m_pListInfo)[i].gop;
			break;
		}

	}

	if (gop < 0) {
		return -1;
	}

	for (; i <= m_pListInfo->size() - 1; i++)
	{
		if ((*m_pListInfo)[i].gop == gop + 1)
		{
			*newPos = (*m_pListInfo)[i].sliceIndex + 1;
			offset = ((*m_pListInfo)[i].offset);
			break;
		}
	}

	if (*newPos == -1) {
		return -1;
	}

	while (dec->getSkippedFrame() > 0) {};

	dec->seek(offset);

	while (dec->getFrame() == 0) {};

	return 0;
}


int CPlayDlg::SeekToAnyFrame(BsrVideoDecoder *dec, int frameNo)
{
	unsigned int i = 0;
	int gop = -1;
	int gopFirstPos = 0;
	int posIFrame = -1;

	BOOL hasSPS = FALSE;
	BOOL hasPPS = FALSE;
	BOOL hasVPS = FALSE;

	int iFramePos = -1;

	fpos_t offset = 0;
	for (i = 0; i <= m_pListInfo->size() - 1; i++)
	{

		gopFirstPos = (gop != (*m_pListInfo)[i].gop) ? (*m_pListInfo)[i].num : gopFirstPos;
		gop = (*m_pListInfo)[i].gop;

		if ((*m_pListInfo)[i].type == 0)
		{
			if ((*m_pListInfo)[i].nalType == NAL_UNIT_TYPE_SPS)
			{
				hasSPS = TRUE; iFramePos = -1;
			}
			if ((*m_pListInfo)[i].nalType == NAL_UNIT_TYPE_PPS)
				hasPPS = TRUE;


			if (IS_IFRAME((*m_pListInfo)[i]))
				iFramePos = i;


			if ((*m_pListInfo)[i].sliceIndex == frameNo - 1 && hasSPS && hasPPS && iFramePos >= 0)
			{
				posIFrame = (*m_pListInfo)[iFramePos/*gopFirstPos*/].sliceIndex + 1;
				offset = ((*m_pListInfo)[iFramePos/*gopFirstPos*/].offset);
			}
			if ((*m_pListInfo)[i].sliceIndex > (UINT)frameNo - 1) break;			
		}
		else
		{
			if ((*m_pListInfo)[i].nalType == NAL_UNIT_SPS)
				hasSPS = TRUE;
			if ((*m_pListInfo)[i].nalType == NAL_UNIT_PPS)
				hasPPS = TRUE;
			if ((*m_pListInfo)[i].nalType == NAL_UNIT_VPS)
				hasVPS = TRUE;

			if ((*m_pListInfo)[i].sliceIndex == frameNo - 1 && hasVPS && hasSPS && hasPPS)
			{
				posIFrame = (*m_pListInfo)[gopFirstPos].sliceIndex + 1;
				offset = ((*m_pListInfo)[gopFirstPos].offset);
			}
			if ((*m_pListInfo)[i].sliceIndex > (UINT)frameNo - 1) break;
		}
	}

	if (posIFrame == -1) {
		return -2; //can't find proper frame
	}

	if (m_fClosed == TRUE)
	{
		ReOpenVideo();
	}
	while (dec->getSkippedFrame() > 0) {};

	dec->seek(offset);

	while (dec->getFrame() > 0)
	{
		if (posIFrame == frameNo) 
		{
			m_nFrameCount = frameNo;
			break;
		}
		posIFrame++;
	}

	if (posIFrame < frameNo)
	{
		while (dec->getSkippedFrame() > 0)
		{
			if (posIFrame == frameNo)
			{
				m_nFrameCount = frameNo;
				break;
			}
			posIFrame++;
		}
	}

	if (posIFrame < frameNo) return -1;

	return 0;
}

void CPlayDlg::OnTimer(UINT_PTR nIDEvent)
{
    int ret = ShowingFrame();

    if (ret == 0 || m_nFrameCount > m_nTotalFrame)
    {
        Pause();
        
        m_fClosed = TRUE;
        CloseVideo();
        if (m_fLoop) 
        {
            m_fPlayed = TRUE;
            OnBnClickedBtPlay();
        }
        return;
    }
}

void CPlayDlg::OnSize(UINT nType, int cx, int cy)
{
    if (!m_fInit) return;

    CDialogEx::OnSize(nType, cx, cy);

	//SetPlayWindowSize(-1);
	//ShowWindow(SW_HIDE);

    CWnd *pWnd = GetDlgItem(IDC_VIDEO);
	if (pWnd)
    {
		Graphics grf(pWnd->m_hWnd);
		grf.Clear(Color::LightGray);

		if (IsZoomed())
		{
			float aspect_video = (float)m_nWidth / m_nHeight;
			float aspect_win = (float)cx / (cy - OTHER_HEIGHT);

			CRect rect0;
			pWnd->GetClientRect(rect0);

			m_nDisplaySizeBeforeZoom = m_nDisplaySize;
			if (aspect_video > aspect_win)
			{
				int diff = ((cy - OTHER_HEIGHT) - (int)((float)cx / aspect_video)) ;
				pWnd->MoveWindow(0, diff/2, cx, cy - OTHER_HEIGHT - diff);

				m_nDisplaySize = (cx * 100) / m_nWidth;
			}
			else
			{
				int diff = (cx - (int)((float)(cy - OTHER_HEIGHT) * aspect_video));
				pWnd->MoveWindow(diff/2, 0, cx - diff, cy - OTHER_HEIGHT);

				m_nDisplaySize = (cy - OTHER_HEIGHT) * 100 / m_nHeight;
			}
		}
		else
		{
			m_nDisplaySize = m_nDisplaySizeBeforeZoom ? m_nDisplaySizeBeforeZoom : m_nDisplaySize;
			m_nDisplaySizeBeforeZoom = 0;
			pWnd->MoveWindow(0, 0, cx, cy - OTHER_HEIGHT);
		}
        pWnd->InvalidateRect(NULL, TRUE);
        pWnd->UpdateData();

		GetDlgItem(IDC_SLIDER_PLAY)->MoveWindow(0, cy - OTHER_HEIGHT + 1, cx, 18);
    }
	//ShowWindow(SW_SHOW);
	UpdateTitle();
    int startx = 1;
    // 水平位置相同的按钮
    for (unsigned int i = 0; i < m_vStartX[0].size(); i++)
    {
        pWnd = GetDlgItem(m_vStartX[0][i]);
        if (pWnd)
        {
			int newY = cy - BUTTON_HEIGHT;
			newY -= (i+1 == m_vStartX[0].size() ? 3 : 0);
			pWnd->SetWindowPos(NULL, startx + m_vStartX[1][i], newY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
    }
}

// 播放
void CPlayDlg::OnBnClickedBtPlay()
{
    if (m_strPathName.IsEmpty())
    {
        MessageBox("Sorry, you open no file.");
        return;
    }

    ReOpenVideo();

    if (m_fPlayed)
    {
        m_bPlay.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_PAUSE))); // show pause...
        SetTimer(1,(UINT)((float)1000/2/m_fFps),NULL);
    }
    else
    {
        m_bPlay.SetBitmap(LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BM_PLAY))); // show play...
        KillTimer(1);
    }

    m_fPlayed = !m_fPlayed;
}

void CPlayDlg::OnBnClickedBtSave()
{
    char szFilter[] =  "YUV File(*.yuv)|*.yuv|"
		"BMP File(*.bmp)|*.bmp|"
		"Media File(*.mov)|*.mov|"
		"Media File(*.mp4)|*.mp4|"
		"||";
    char szFileName[128] = "foobar";
    char* pExt = _T("yuv");

    CFile cFile;
    CString strFile;

    if (m_nFrameCount < 1)
    {
        MessageBox("Sorry, can't save frame.");
        return;
    }

    Pause();

    _splitpath(m_strPathName, NULL, NULL, szFileName, NULL);
    strFile.Format(_T("%s_%d.%s"), szFileName, m_nFrameCount, pExt);

    CFileDialog fileDlg(FALSE, _T("Save File"), strFile.GetBuffer(), OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT, szFilter, this);
    fileDlg.GetOFN().lpstrTitle = _T("Save File");
    if (fileDlg.DoModal() != IDOK)
        return;

    CString strSaveFile = fileDlg.GetPathName();
    CString strExt = fileDlg.GetFileExt();

    int ret = 0;
	if (!strExt.CompareNoCase(_T("yuv")))
    {
        ret = SaveYUVFile(strSaveFile.GetBuffer());
    }
    else if (!strExt.CompareNoCase(_T("bmp")))
    {
        ret = SaveBMPFile(strSaveFile.GetBuffer());
    }
	else
	{
		ret = SaveVideoFile(strSaveFile.GetBuffer());
	}

#if 1
    CString strDebugInfo;
	if (ret == 0)
		strDebugInfo.Format("保存完毕");
	if (ret == -1)
		strDebugInfo.Format("分辨率输入错误");
	if (ret == -2)
		strDebugInfo.Format("起止帧序号输入错误");
	if (ret == -3)
		strDebugInfo.Format("BMP保存只支持单文件");
	if (ret == -4)
		strDebugInfo.Format("YUV文件创建错误");
	if (ret == -5)
		strDebugInfo.Format("YUV文件分段，创建新YUV文件错误，可能空间不足");

    GetDlgItem(IDC_S_DEBUG)->SetWindowText(strDebugInfo);
#endif
}


void CPlayDlg::OnBnClickedCkLoop()
{
    CButton* pBtn = (CButton*)GetDlgItem(IDC_CK_LOOP);
    m_fLoop = pBtn->GetCheck();
}

//#define VK_OEM_1          0xBA   // ';:' for US
//#define VK_OEM_PLUS       0xBB   // '+' any country
//#define VK_OEM_COMMA      0xBC   // ',' any country
//#define VK_OEM_MINUS      0xBD   // '-' any country
//#define VK_OEM_PERIOD     0xBE   // '.' any country
//#define VK_OEM_2          0xBF   // '/?' for US
//#define VK_OEM_3          0xC0   // '`~' for US

BOOL CPlayDlg::PreTranslateMessage(MSG* pMsg) 
{

	m_tooltip.RelayEvent(pMsg);

	if (m_sliderPlay.m_hWnd == pMsg->hwnd)
	{
		//if (pMsg->message == WM_LBUTTONUP) return TRUE;
	}

    if(pMsg->message == WM_KEYDOWN)
    {
        switch(pMsg->wParam)
        {
        // press esc..
        case VK_ESCAPE:
            OnBnClickedBtStop();
			parentWnd->SetFocus();
            break;
		case VK_OEM_MINUS:
			if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000))
			{
				int percent = (m_nDisplaySize > 30) ? m_nDisplaySize - 10 : m_nDisplaySize;
				SetPlayWindowSize(percent);
			}
			break;
		case VK_OEM_PLUS:
			if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000))
			{
				int percent = (m_nDisplaySize < 100 || (m_nDisplaySize < 200 && m_nWidth < 1280)) ? m_nDisplaySize + 10 : m_nDisplaySize;
				SetPlayWindowSize(percent);
			}
			break;
		case 'B':
			if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000))
			{
				uint8_t R, G, B;
				int ret = SDL_GetTextureColorMod(m_pSdlTexture, &R, &G, &B);
				if ((GetAsyncKeyState(VK_LSHIFT) & 0x8000))
				{
					R = (R > 20) ? R - 10 : R;
					G = (G > 20) ? G - 10 : G;
					B = (B > 20) ? B - 10 : B;
				}
				else
				{
					R = (R < 240) ? R + 10 : R;
					G = (G < 240) ? G + 10 : G;
					B = (B < 240) ? B + 10 : B;
				}
				SDL_SetTextureColorMod(m_pSdlTexture, R, G, B);
				RefreshSDLTexture();
			}
			break;
		default:
            break;
        }
	}

	CWnd *pWnd = GetDlgItem(IDC_VIDEO);
	if (pWnd->GetSafeHwnd() == pMsg->hwnd)
	{
		if (pMsg->message == WM_LBUTTONDOWN)
		{
			CPoint pt;
			CRect rt;
			pt.x = pMsg->pt.x;
			pt.y = pMsg->pt.y;

			pWnd->ScreenToClient(&pt);
			pWnd->GetClientRect(&rt);

			x_block = pt.x + BLOCK_SIZE / 2 > rt.right ? rt.right - BLOCK_SIZE : (pt.x - BLOCK_SIZE / 2 < 0 ? 0 : pt.x - BLOCK_SIZE / 2);
			y_block = pt.y + BLOCK_SIZE / 2 > rt.bottom ? rt.bottom - BLOCK_SIZE : (pt.y - BLOCK_SIZE / 2 < 0 ? 0 : pt.y - BLOCK_SIZE / 2);

			if (m_fPlayed && !m_fClosed)
			{
				m_cDecoder.PostProcPicture(&m_pLockBuf, NULL, &m_nPitch);
				Show(m_nWidth, m_nHeight);
			}
		}
		//	return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CPlayDlg::OnBnClickedButtonDisplay100()
{
	// TODO: 在此添加控件通知处理程序代码
	SetPlayWindowSize(100);
}

void CPlayDlg::OnBnClickedButtonDisplay75()
{
	// TODO: 在此添加控件通知处理程序代码
	SetPlayWindowSize(75);
}

void CPlayDlg::OnBnClickedButtonDisplay50()
{
	// TODO: 在此添加控件通知处理程序代码
	SetPlayWindowSize(50);
}


void CPlayDlg::OnBnClickedButtonQp()
{
	// TODO: 在此添加控件通知处理程序代码
	m_cDecoder.SetDisplayQP();
	
	if (!m_fPlayed || m_fClosed) return;
	
	m_cDecoder.PostProcPicture(&m_pLockBuf, NULL, &m_nPitch);

	Show(m_nWidth, m_nHeight);

	UpdateTitle();
}


void CPlayDlg::OnBnClickedButtonMb()
{
	// TODO: 在此添加控件通知处理程序代码
	m_cDecoder.SetDisplayMB();

	if (!m_fPlayed || m_fClosed) return;

	m_cDecoder.PostProcPicture(&m_pLockBuf, NULL, &m_nPitch);

	Show(m_nWidth, m_nHeight);

	UpdateTitle();
}

void CPlayDlg::DebugInfo(char *debugInfo)
{
	if (strlen(debugInfo))
		GetDlgItem(IDC_S_DEBUG)->SetWindowText(debugInfo);
}

void CPlayDlg::UpdateTitle()
{
	m_sliderPlay.SetPos(m_nFrameCount);

	CString strTittle;
	strTittle.Format("[%s] %d/%d  %0.2ffps [%s : %s] [size: %d%c] --  %s", strFrameType[m_cDecoder.getFrameType()], m_nFrameCount, m_nTotalFrame, m_fFps,
		m_cDecoder.StrStatusQP(),
		m_cDecoder.StrStatusMB(),
		m_nDisplaySize, '%',
		DLG_TITTLE);
	this->SetWindowText(strTittle);

	char *debugInfo = m_cDecoder.getCodecInfo();
	if(strlen(debugInfo))
		GetDlgItem(IDC_S_DEBUG)->SetWindowText(debugInfo);
}

void CPlayDlg::OnBnClickedBtNext()
{
	Pause();

	if (m_nFrameCount == m_nTotalFrame) {
		CString strDebugInfo;
		strDebugInfo.Format("已经是最后一帧，再按一次回到第一帧！");
		GetDlgItem(IDC_S_DEBUG)->SetWindowText(strDebugInfo);
	}

	// loop
	ReOpenVideo();

	OnTimer(-1);

	return;
}

void CPlayDlg::OnBnClickedBtLast()
{
	// TODO: 在此添加控件通知处理程序代码
	Pause();

	if (m_nFrameCount <= 1) 
	{
		CString strDebugInfo;
		strDebugInfo.Format("已经回到第一帧！");
		GetDlgItem(IDC_S_DEBUG)->SetWindowText(strDebugInfo);
		return;
	}
	int ret = SeekToAnyFrame(&m_cDecoder, m_nFrameCount - 1);

	if (ret)
	{
		CString strDebugInfo;
		if (ret == -1)
			strDebugInfo.Format("执行错误");
		if (ret == -2)
			strDebugInfo.Format("已经回到第一个I帧！");
		GetDlgItem(IDC_S_DEBUG)->SetWindowText(strDebugInfo);
		return;
	}

	m_cDecoder.PostProcPicture(&m_pLockBuf, NULL, &m_nPitch);

	Show(m_nWidth, m_nHeight);

	UpdateTitle();
}

void CPlayDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl* pSlider = (CSliderCtrl*)pScrollBar;
	int pos = pSlider->GetPos();
	if (pScrollBar == (CScrollBar*)GetDlgItem(IDC_SLIDER_PLAY))
	{
		int cur_pos = m_sliderPlay.GetPos();
		int new_pos = -1;
		int ret = 0;
		switch (nSBCode)
		{
		case SB_THUMBPOSITION:
			m_sliderPlay.SetPos(pos);

			Pause();

			if (m_nFrameCount == pos) 
				break;

			ret = SeekToAnyFrame(&m_cDecoder, pos - 1);

			if (ret < 0)
			{
				m_sliderPlay.SetPos(m_nFrameCount);
				break;
			}

			m_nFrameCount = pos;

			m_cDecoder.PostProcPicture(&m_pLockBuf, NULL, &m_nPitch);

			Show(m_nWidth, m_nHeight);

			UpdateTitle();

			break;
		case SB_ENDSCROLL:
			if (m_lastSliderAction == SB_PAGELEFT)
			{
				Pause();

				ret = SeekToPrevIFrame(&m_cDecoder, m_nFrameCount, &new_pos);

				if (ret)
				{
					CString strDebugInfo;
					strDebugInfo.Format("已经回到第一帧！");
					GetDlgItem(IDC_S_DEBUG)->SetWindowText(strDebugInfo);
				}
				else
				{
					m_nFrameCount = new_pos;
					//m_sliderPlay.SetPos(m_nFrameCount);

					m_cDecoder.PostProcPicture(&m_pLockBuf, NULL, &m_nPitch);
					Show(m_nWidth, m_nHeight);

					UpdateTitle();
				}
			}
			if (m_lastSliderAction == SB_PAGERIGHT)
			{
				Pause();

				ret = SeekToNextIFrame(&m_cDecoder, m_nFrameCount, &new_pos);

				if (ret)
				{
					CString strDebugInfo;
					strDebugInfo.Format("已经到最后的一个I帧！");
					GetDlgItem(IDC_S_DEBUG)->SetWindowText(strDebugInfo);
				}
				else
				{
					m_nFrameCount = new_pos;
					//m_sliderPlay.SetPos(m_nFrameCount);

					m_cDecoder.PostProcPicture(&m_pLockBuf, NULL, &m_nPitch);
					Show(m_nWidth, m_nHeight);

					UpdateTitle();
				}
			}
			break;
		//we dont't process it because it cause two action
		//we will process it after we received the SB_ENDSCROLL, so we recorded the last action
		case SB_PAGELEFT:
			break;
		case SB_PAGERIGHT:
			break;
		default:
			break;
		}

		m_lastSliderAction = nSBCode;
		//pSlider->SetScrollPos
		m_tooltip.Update();//更新tip，会触发TTN_NEEDTEXT消息 
		return;
	}


	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CPlayDlg::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	TOOLTIPTEXT   *pTTT = (TOOLTIPTEXT  *)pNMHDR;
	//in tagNMTTDISPINFOA
	//LPSTR lpszText;
    //char szText[80];

	UINT   nID = pNMHDR->idFrom;
	if (pTTT->uFlags  &   TTF_IDISHWND)
	{
		nID = ::GetDlgCtrlID((HWND)nID);
		NALU_t *nalu = NULL;
		CString strTipFromResource;
		switch (nID)
		{
		case IDC_SLIDER_PLAY:
			nalu = GetFrameInfo(m_sliderPlay.GetPos());
			if (nalu)
				sprintf(pTTT->lpszText, _T("帧: %d/%d\r%s \rSize: %d\rGOP: %d"), m_sliderPlay.GetPos(), m_sliderPlay.GetRangeMax(),
					nalu->strInfo, nalu->len, nalu->gop);
			else
				sprintf(pTTT->lpszText, _T("No detail info!"));
			break;
		case IDC_EDIT_FRAME_BEGIN:
		case IDC_EDIT_FRAME_END:
		case IDC_EDIT_WIDTH:
		case IDC_EDIT_HEIGHT:
		case IDC_BUTTON_100:
		case IDC_BUTTON_75:
		case IDC_BUTTON_50:
		case IDC_BUTTON_QP:
		case IDC_BUTTON_QP2:
		case IDC_BUTTON_MP:
		case IDC_BT_SAVE:
			//get string from rc file according to the control ID, the string resource has the same name with the control
			strTipFromResource.LoadString(nID); 
			sprintf(pTTT->lpszText, strTipFromResource);
			break;
		default:
			break;
		}
		if (strlen(pTTT->lpszText))//如果现在这个ID是你要显示Tip的控件
		{
			return TRUE;
		}
	}
	return FALSE;
}

NALU_t *CPlayDlg::GetFrameInfo(unsigned int frameNo)
{
	for (UINT i = frameNo; i <= m_pListInfo->size() - 1; i++)
	{
		if ((*m_pListInfo)[i].sliceIndex == frameNo - 1 && notPPS((*m_pListInfo)[i].isFrame))
			return &(*m_pListInfo)[i];
	}
	return 0;
}


void CPlayDlg::OnBnClickedButtonQp2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_cDecoder.SetDisplayQC();

	if (!m_fPlayed || m_fClosed) return;

	m_cDecoder.PostProcPicture(&m_pLockBuf, NULL, &m_nPitch);

	Show(m_nWidth, m_nHeight);

	UpdateTitle();
}


void CPlayDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nID == SC_MAXIMIZE)
	{
		//SetPlayWindowSize(-1);
		//return;
	}

	CDialogEx::OnSysCommand(nID, lParam);
}
