#pragma once

#include <string>
#include <vector>
#include <map>

#include "afxwin.h"
#include "VideoDecode.h"
#include "H264ToVideo.h"
#include "NalParse.h"
#include "afxcmn.h"

extern "C"
{
#include "SDL.h"
};

#define DLG_TITTLE "Play"

typedef int(*SDL_Init_Dll)(Uint32 flags);
typedef SDL_Window * (*SDL_CreateWindowFrom_Dll)(const void *data);
typedef SDL_Renderer * (*SDL_CreateRenderer_Dll)(SDL_Window * window, int index, Uint32 flags);
typedef SDL_bool(*SDL_SetHint_Dll)(const char *name, const char *value);
typedef SDL_Texture * (*SDL_CreateTexture_Dll)(SDL_Renderer * renderer, Uint32 format, int access, int w, int h);
typedef void(*SDL_DestroyTexture_Dll)(SDL_Texture * texture);
typedef int(*SDL_UpdateYUVTexture_Dll)(SDL_Texture * texture, const SDL_Rect * rect,
	const Uint8 *Yplane, int Ypitch,
	const Uint8 *Uplane, int Upitch,
	const Uint8 *Vplane, int Vpitch);
typedef void(*SDL_GetWindowSize_Dll)(SDL_Window * window, int *w, int *h);
typedef int(*SDL_RenderClear_Dll)(SDL_Renderer * renderer);
typedef int(*SDL_RenderCopy_Dll)(SDL_Renderer * renderer,
	SDL_Texture * texture,
	const SDL_Rect * srcrect,
	const SDL_Rect * dstrect);
typedef void(*SDL_RenderPresent_Dll)(SDL_Renderer * renderer);
typedef void(*SDL_DestroyRenderer_Dll)(SDL_Renderer * renderer);
typedef void(*SDL_DestroyWindow_Dll)(SDL_Window * window);
typedef void(*SDL_Quit_Dll)(void);
typedef const char * (*SDL_GetError_Dll)(void);
typedef int(*SDL_SetWindowBrightness_Dll)(SDL_Window* window, float brightness);
typedef float(*SDL_GetWindowBrightness_Dll)(SDL_Window* window);
typedef int(*SDL_SetTextureColorMod_Dll)(SDL_Texture* texture, Uint8 r, Uint8 g, Uint8 b);
typedef int(*SDL_GetTextureColorMod_Dll)(SDL_Texture* texture, Uint8* r, Uint8* g, Uint8* b);
typedef int(*SDL_LockTexture_Dll)(SDL_Texture*    texture,
	const SDL_Rect* rect,
	void**          pixels,
	int*            pitch);
typedef int(*SDL_UnlockTexture_Dll)(SDL_Texture*    texture);
typedef SDL_TimerID (*SDL_AddTimer_Dll)(Uint32 interval, SDL_TimerCallback callback, void* param);
typedef SDL_bool (*SDL_RemoveTimer_Dll)(SDL_TimerID id);

typedef int(*SDL_SetTextureBlendMode_Dll)(SDL_Texture * texture, SDL_BlendMode blendMode);
typedef int(*SDL_SetRenderTarget_Dll)(SDL_Renderer *renderer, SDL_Texture *texture);
typedef int(*SDL_SetRenderDrawBlendMode_Dll)(SDL_Renderer * renderer, SDL_BlendMode blendMode);
typedef int(*SDL_SetRenderDrawColor_Dll)(SDL_Renderer * renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
typedef int(*SDL_RenderFillRect_Dll)(SDL_Renderer * renderer, const SDL_Rect * rect);
typedef int(* SDL_RenderDrawRect_Dll)(SDL_Renderer * renderer, const SDL_Rect * rect);

#define SDL_FUNC_LOAD(name)		name = (name##_Dll)GetProcAddress(hSDL_DLL, #name)
#define SDL_FUNC_DECLARE(name)	name##_Dll name

#define SDL_FUNC_ALL(TYPE) \
	TYPE(SDL_Init); \
	TYPE(SDL_CreateWindowFrom); \
	TYPE(SDL_CreateRenderer); \
	TYPE(SDL_SetHint); \
	TYPE(SDL_CreateTexture); \
	TYPE(SDL_DestroyTexture); \
	TYPE(SDL_UpdateYUVTexture); \
	TYPE(SDL_GetWindowSize); \
	TYPE(SDL_RenderClear); \
	TYPE(SDL_RenderCopy); \
	TYPE(SDL_RenderPresent); \
	TYPE(SDL_DestroyRenderer); \
	TYPE(SDL_DestroyWindow); \
	TYPE(SDL_Quit); \
	TYPE(SDL_GetError); \
	TYPE(SDL_SetWindowBrightness); \
	TYPE(SDL_GetWindowBrightness); \
	TYPE(SDL_SetTextureColorMod); \
	TYPE(SDL_GetTextureColorMod); \
	TYPE(SDL_LockTexture); \
	TYPE(SDL_UnlockTexture); \
	TYPE(SDL_AddTimer); \
	TYPE(SDL_RemoveTimer); \
	TYPE(SDL_SetTextureBlendMode); \
	TYPE(SDL_SetRenderTarget); \
	TYPE(SDL_SetRenderDrawBlendMode); \
	TYPE(SDL_SetRenderDrawColor); \
	TYPE(SDL_RenderFillRect); \
	TYPE(SDL_RenderDrawRect);


#define SDL_FUNC_DECLARE_ALL SDL_FUNC_ALL(SDL_FUNC_DECLARE)
#define SDL_FUNC_LOAD_ALL SDL_FUNC_ALL(SDL_FUNC_LOAD)


// CPlayDlg dialog

class CPlayDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CPlayDlg)

public:
    CPlayDlg(CWnd* pParent = NULL);   // standard constructor
    ~CPlayDlg();

// Dialog Data
    enum { IDD = IDD_PLAYDLG };

private:
	CWnd *parentWnd;
public:
    // 从主窗口拿到视频文件信息
    int SetVideoInfo(CString strFileName, int nWidth, int nHeight, int nTotalFrame, float nFps);
	void SetVideoSize(int nWidth, int nHeight) 
	{
		m_nWidth = nWidth;
		m_nHeight = nHeight;
	};
	void SetRealParent(void *pWnd) { parentWnd = (CWnd *)pWnd; };
    void ShowFirstFrame();

private:

    void Show(int nWidth, int nHeight);

    int ShowingFrame();
    void Pause();
    void CloseVideo();
    void ReOpenVideo();

    //int SaveJPGFile(const char* pFileName);
    int SaveYUVFile(const char* pFileName);
    int SaveBMPFile(const char* pFileName);
    //int SaveRGBFile(const char* pFileName);
    //int SaveBGRFile(const char* pFileName);
    int SaveVideoFile(const char* pFileName);

	void SetPlayWindowSize(int percent=0);
	void UpdateTitle();
	void DebugInfo(char *debugInfo);

	int SeekToIFrame(int begFrame, int *posIFrame, fpos_t *offset);
	int SeekToAnyFrame(BsrVideoDecoder *dec, int begFrame);
	int SeekToPrevIFrame(BsrVideoDecoder *dec, int begFrame, int *newPos);
	int SeekToNextIFrame(BsrVideoDecoder *dec, int begFrame, int *newPos);

	NALU_t *GetFrameInfo(unsigned int frameNo);


public:
	int SeekVideo(int64_t pos);
	void SeekPlay(int64_t pos, int index);
	void SetListInfo(vector<NALU_t> *pListInfo) { m_pListInfo = pListInfo; };

private:
    BOOL m_fShowBlack;
    BOOL m_fPlayed;
    BOOL m_fClosed;
    BOOL m_fLoop;
    BOOL m_fInit;
    INT m_nWidth;
    INT m_nHeight;
    INT m_nTotalFrame;
    INT m_nFrameCount;
    float m_fFps;
    CString m_strPathName;   // 视频文件

	//hcg
	BOOL m_bDisplayQP;
	BOOL m_bDisplayQC;
	BOOL m_bDisplayMB;
	int  m_nDisplaySize;
	int  m_nDisplaySizeBeforeZoom;

	vector<NALU_t> *m_pListInfo;

    BsrVideoDecoder m_cDecoder;    // 解码器

    std::vector<std::vector<int> > m_vStartX;

protected: //for SDL
	//DLL
	HMODULE hSDL_DLL;

	SDL_FUNC_DECLARE_ALL;


	SDL_Window* m_pSdlWindow;
	SDL_Renderer* m_pSdlRenderer;
	SDL_Texture* m_pSdlTexture;
	SDL_Texture *m_pSdlOverlayTexture;


	Uint32 m_nPixelFormat;
	int m_nSdlWidth, m_nSdlHeight;
	int m_nPitch;
	int x_block, y_block;

	uint8_t *m_pLockBuf;
	int m_nLockPitch;

	void OverlayTexture(SDL_Renderer *renderer, SDL_Texture *texture, int r, int g, int b, int a);
	void PrepareForRendering(SDL_Renderer *renderer);

	void SetSDLMode() { m_nPixelFormat = SDL_PIXELFORMAT_IYUV; };
	void InitSDL();
	int StartSDL(int nId, int image_w, int image_h);
	int ShowPictureSDL(const BYTE **buffer, int pitch);
	int UpdateSDL();
	int RefreshSDLTexture();
	int GetSDLLockBuffer(void **, int *);
	int EndSDL();


protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();
    afx_msg void OnBnClickedBtPlay();
    afx_msg void OnClose();
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    CButton m_bPlay;
    CButton m_bSaveFrame;
    CButton m_bStop;
    CButton m_bNextFrame;
    afx_msg void OnBnClickedBtNext();
    afx_msg void OnBnClickedBtSave();
    afx_msg void OnBnClickedBtStop();
    afx_msg void OnBnClickedCkLoop();

    afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	CEdit m_editWidth;
	CEdit m_editHeight;
	CEdit m_editFrameB;
	CEdit m_editFrameE;
	afx_msg void OnBnClickedButtonDisplay100();
	afx_msg void OnBnClickedButtonDisplay50();
	afx_msg void OnBnClickedButtonQp();
	afx_msg void OnBnClickedButtonMb();
	afx_msg void OnBnClickedButtonDisplay75();
	afx_msg void OnBnClickedBtLast();
	CButton m_bLastFrame;
	CSliderCtrl m_sliderPlay;
	UINT m_lastSliderAction;

	CToolTipCtrl m_tooltip;
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButtonQp2();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
};
