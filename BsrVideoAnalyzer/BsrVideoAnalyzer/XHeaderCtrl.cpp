// XHeaderCtrl.cpp  Version 1.4
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// This code is based on "Outlook 98-Style FlatHeader Control" 
// by Maarten Hoeben.
//
// See http://www.codeguru.com/listview/FlatHeader.shtml
//
// License:
//     This software is released into the public domain.  You are free to use
//     it in any way you like, except that you may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this 
//     software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XHeaderCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXHeaderCtrl

IMPLEMENT_DYNCREATE(CXHeaderCtrl, CHeaderCtrl)

BEGIN_MESSAGE_MAP(CXHeaderCtrl, CHeaderCtrl)
	//{{AFX_MSG_MAP(CXHeaderCtrl)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SYSCOLORCHANGE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(HDM_LAYOUT, OnLayout)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// ctor
CXHeaderCtrl::CXHeaderCtrl()
{
	m_cr3DHighLight   = ::GetSysColor(COLOR_3DHIGHLIGHT);
	m_cr3DShadow      = ::GetSysColor(COLOR_3DSHADOW);
	m_cr3DFace        = ::GetSysColor(COLOR_3DFACE);
	m_crBtnText       = ::GetSysColor(COLOR_BTNTEXT);

	m_pListCtrl       = NULL;			//+++
	m_nFormat         = DT_DEFAULT;		//+++
	m_rgbText         = m_crBtnText;	//+++
	m_bDividerLines   = TRUE;			//+++
	m_iSpacing        = 6;
	m_bStaticBorder   = FALSE;
	m_bResizing       = FALSE;
	m_nClickFlags     = 0;

	m_bScrollBarVisible = FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// DrawCtrl
void CXHeaderCtrl::DrawCtrl(CDC* pDC)
{
	CRect rectClip;
	if (pDC->GetClipBox(&rectClip) == ERROR)
		return;

	CRect rectClient, rectItem;
	GetClientRect(&rectClient);

	int nWidth = m_bScrollBarVisible ? GetSystemMetrics(SM_CXVSCROLL)-1 : 0;
	rectClient.right -= nWidth;

    pDC->FillSolidRect(rectClip, m_cr3DFace);

	int iItems = GetItemCount();
	ASSERT(iItems >= 0);

	CPen penHighLight(PS_SOLID, 1, m_cr3DHighLight);
	CPen penShadow(PS_SOLID, 1, m_cr3DShadow);
	CPen* pPen = pDC->GetCurrentPen();

	CFont* pFont = pDC->SelectObject(GetFont());

	pDC->SetBkColor(m_cr3DFace);
	pDC->SetTextColor(m_crBtnText);

	int iWidth = 0;

	for (int i = 0; i < iItems; i++)
	{
		int iItem = OrderToIndex(i);

		TCHAR szText[FLATHEADER_TEXT_MAX];

		HDITEM hditem;
		hditem.mask = HDI_WIDTH|HDI_FORMAT|HDI_TEXT|HDI_IMAGE|HDI_BITMAP;
		hditem.pszText = szText;
		hditem.cchTextMax = sizeof(szText);
		VERIFY(GetItem(iItem, &hditem));

		VERIFY(GetItemRect(iItem, rectItem));

		if (rectItem.right >= rectClip.left || rectItem.left <= rectClip.right)
		{
			if (hditem.fmt & HDF_OWNERDRAW)
			{
				DRAWITEMSTRUCT disItem;
				disItem.CtlType = ODT_BUTTON;
				disItem.CtlID = GetDlgCtrlID();
				disItem.itemID = iItem;
				disItem.itemAction = ODA_DRAWENTIRE;
				disItem.itemState = 0;
				disItem.hwndItem = m_hWnd;
				disItem.hDC = pDC->m_hDC;
				disItem.rcItem = rectItem;
				disItem.itemData = 0;

				DrawItem(&disItem);
			}
			else
			{
				rectItem.DeflateRect(m_iSpacing, 0);
				DrawItem(pDC, rectItem, &hditem);
				rectItem.InflateRect(m_iSpacing, 0);

				//if (m_nClickFlags & MK_LBUTTON && m_iHotIndex == iItem && m_hdhtiHotItem.flags & HHT_ONHEADER)
				//	pDC->InvertRect(rectItem);
			}

			if (i < iItems)
			{
				// draw divider lines
				if (m_bDividerLines)		//+++
				{
					pDC->SelectObject(&penShadow);
					pDC->MoveTo(rectItem.right, rectItem.top+3);
					pDC->LineTo(rectItem.right, rectItem.bottom-3);

					pDC->SelectObject(&penHighLight);
					pDC->MoveTo(rectItem.right+1, rectItem.top+3);
					pDC->LineTo(rectItem.right+1, rectItem.bottom-3);
				}
			}
		}

		iWidth += hditem.cxy;
	}

	if (iWidth > 0)
	{
		//rectClient.right = rectClient.left + iWidth;
		//pDC->Draw3dRect(rectClient, m_cr3DHighLight, m_cr3DShadow);
		pDC->Draw3dRect(rectClient, RGB(210, 130, 130), RGB(210, 130, 130));
	}

	pDC->SelectObject(pFont);
	pDC->SelectObject(pPen);

	penHighLight.DeleteObject();
	penShadow.DeleteObject();
}

///////////////////////////////////////////////////////////////////////////////
// DrawItem
void CXHeaderCtrl::DrawItem(LPDRAWITEMSTRUCT)
{
	ASSERT(FALSE);  // must override for self draw header controls
}

///////////////////////////////////////////////////////////////////////////////
// DrawItem
void CXHeaderCtrl::DrawItem(CDC* pDC, CRect rect, LPHDITEM lphdi)
{
	ASSERT(lphdi->mask & HDI_FORMAT);

	int iWidth = 0;

	DrawText(pDC, rect, lphdi);
}

///////////////////////////////////////////////////////////////////////////////
// DrawText
int CXHeaderCtrl::DrawText(CDC* pDC, CRect rect, LPHDITEM lphdi)
{
	CSize size = pDC->GetTextExtent(lphdi->pszText);

#if 0  // -----------------------------------------------------------
	pDC->SetTextColor(RGB(0,0,255));

	if (rect.Width() > 0 && lphdi->mask & HDI_TEXT && lphdi->fmt & HDF_STRING)
	{
		size = pDC->GetTextExtent(lphdi->pszText);

		// always center column headers
		pDC->DrawText(lphdi->pszText, -1, rect, 
			DT_CENTER|DT_END_ELLIPSIS|DT_SINGLELINE|DT_VCENTER);
	}
#endif // -----------------------------------------------------------

	//+++

	if (rect.Width() > 0 && lphdi->mask & HDI_TEXT && lphdi->fmt & HDF_STRING)
	{
		pDC->SetTextColor(m_cr3DShadow);// m_rgbText);

		UINT nFormat = m_nFormat;

		if (nFormat == DT_DEFAULT)
		{
			// default to whatever alignment the column is set to

			if (lphdi->fmt & LVCFMT_CENTER)
				nFormat = DT_CENTER;
			else if (lphdi->fmt & LVCFMT_RIGHT)
				nFormat = DT_RIGHT;
			else
				nFormat = DT_LEFT;
		}

		pDC->DrawText(lphdi->pszText, -1, rect, 
			nFormat | DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);
		
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(m_rgbText);
		rect.left--; rect.right--; rect.top--; rect.bottom--;
		pDC->DrawText(lphdi->pszText, -1, rect,
			nFormat | DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER);
	}

	size.cx = rect.Width() > size.cx ? size.cx : rect.Width();

	return size.cx > 0 ? size.cx : 0;
}

///////////////////////////////////////////////////////////////////////////////
// OnLayout
LRESULT CXHeaderCtrl::OnLayout(WPARAM, LPARAM lParam)
{
	LPHDLAYOUT lphdlayout = (LPHDLAYOUT)lParam;

	if (m_bStaticBorder)
		lphdlayout->prc->right += GetSystemMetrics(SM_CXBORDER)*2;

	return CHeaderCtrl::DefWindowProc(HDM_LAYOUT, 0, lParam);
}

///////////////////////////////////////////////////////////////////////////////
// OnSysColorChange
void CXHeaderCtrl::OnSysColorChange() 
{
	CHeaderCtrl::OnSysColorChange();
	
	m_cr3DHighLight = ::GetSysColor(COLOR_3DHIGHLIGHT);
	m_cr3DShadow    = ::GetSysColor(COLOR_3DSHADOW);
	m_cr3DFace      = ::GetSysColor(COLOR_3DFACE);
	m_crBtnText     = ::GetSysColor(COLOR_BTNTEXT);
}

///////////////////////////////////////////////////////////////////////////////
// OnEraseBkgnd
BOOL CXHeaderCtrl::OnEraseBkgnd(CDC* pDC) 
{
	UNUSED_ALWAYS(pDC);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// OnPaint
void CXHeaderCtrl::OnPaint() 
{
    CPaintDC dc(this);

	DrawCtrl(&dc);
}
