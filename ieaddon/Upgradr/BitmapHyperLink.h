// BitmapHyperLink.h : declaration of the CBitmapHyperLink class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlctrls.h>
#include <atlctrlx.h>

#define HLINK_LEFTIMAGE		0x00000080
#define HLINK_RIGHTIMAGE	0x00000100

#define X_MARGIN			5

template <class T>
class ATL_NO_VTABLE CBitmapHyperLinkImpl : public CHyperLinkImpl<T> {
private:
	CBitmap m_Bitmap;
	CSize m_sizeBmp;			// bitmap's size
	CPoint m_ptBmpLocation;		// bitmap's location

	COLORREF m_clrHover;		// text color when the mouse hovers over the control 

public:
	CBitmapHyperLinkImpl() 
	{
		m_sizeBmp.SetSize(0, 0);
		m_ptBmpLocation.SetPoint(0, 0);
		m_clrHover = RGB(155, 155, 155);	// default
	}

	~CBitmapHyperLinkImpl()	{}

	BEGIN_MSG_MAP(CBitmapHyperLinkImpl<T>)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		CHAIN_MSG_MAP(CHyperLinkImpl<T>)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	// Loads a bitmap
	HBITMAP AddBitmap(_U_STRINGorID bitmap)
	{
		return m_Bitmap.LoadBitmap(bitmap);
	}

	// Deletes a bitmap, after it's deleted, the handle is no longer valid
	BOOL RemoveBitmap()
	{
		return m_Bitmap.DeleteObject();
	}

	DWORD SetExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		return SetHyperLinkExtendedStyle(dwExtendedStyle, dwMask);
	}

	void SetLinkColor(COLORREF clrNormal)
	{
		m_clrLink = clrNormal;
	}

	void SetVisitedColor(COLORREF clrVisited)
	{
		m_clrVisited = clrVisited;
	}

	void SetHoverColor(COLORREF clrHover)
	{
		m_clrHover = clrHover;
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		// Show "hand" cursor when over the bitmap
		if (IsBitmap())
		{
			CRect rcBmp(m_ptBmpLocation, m_sizeBmp);
			if (::PtInRect(&rcBmp, pt))
			{
				::SetCursor(m_hCursor);
				if (IsUnderlineHover())
				{
					if (!m_bHover)
					{
						m_bHover = true;
						InvalidateRect(&m_rcLink);
						UpdateWindow();
#ifndef _WIN32_WCE
						StartTrackMouseLeave();
#endif //!_WIN32_WCE
					}
				}
				else
				{
					if (IsUnderlineHover())
					{
						if (m_bHover)
						{
							m_bHover = false;
							InvalidateRect(&m_rcLink);
							UpdateWindow();
						}
					}
					bHandled = FALSE;
				}
				return 0;
			}
			else
				return CHyperLinkImpl<T>::OnMouseMove(uMsg, wParam, lParam, bHandled);
		}
		else
			return CHyperLinkImpl<T>::OnMouseMove(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

		if (IsBitmap())
		{
			CRect rcBmp(m_ptBmpLocation, m_sizeBmp);

			// Set the mouse capture
			if (::PtInRect(&rcBmp, pt))
			{
				SetFocus();
				SetCapture();
			}
		}
		return CHyperLinkImpl<T>::OnLButtonDown(uMsg, wParam, lParam, bHandled);
	}

	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if (GetCapture() == m_hWnd)
		{
			ReleaseCapture();
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

			CRect rcBmp;

			if (IsBitmap())
				rcBmp.SetRect(m_ptBmpLocation.x, m_ptBmpLocation.y, m_ptBmpLocation.x + m_sizeBmp.cx, m_ptBmpLocation.y + m_sizeBmp.cy);
			else
				rcBmp.SetRectEmpty();

			if (::PtInRect(&m_rcLink, pt) || ::PtInRect(&rcBmp, pt))
			{
				T* pT = static_cast<T*>(this);
				pT->Navigate();
			}
		}
		return 0;
	}

	// Do we have a bitmap?
	bool IsBitmap()
	{
		return (m_Bitmap.IsNull() ? false : true);
	}

	void DoPaint(CDCHandle dc)
	{
		COLORREF clrOld = 0;
		RECT rcClient = { 0 };
		GetClientRect(&rcClient);

		// Display the bitmap
		if (IsBitmap())
		{
			CSize sizeImage(0);
			CPoint ptLocation(0, 0);

			// Get the bitmap's size
			m_Bitmap.GetSize(sizeImage);

			if (m_dwExtendedStyle & HLINK_LEFTIMAGE)	// bitmap is on the left
				::SetRect(&m_rcLink, sizeImage.cx + X_MARGIN, m_rcLink.top, rcClient.right, rcClient.bottom);
			else if (m_dwExtendedStyle & HLINK_RIGHTIMAGE)	// bitmap is on the right
			{
				//ptLocation.x = m_rcLink.right + X_MARGIN;	// if you want a bitmap to be right next to the text	
				ptLocation.x = rcClient.right - sizeImage.cx;
				::SetRect(&m_rcLink, m_rcLink.left, m_rcLink.top, rcClient.right - (sizeImage.cx), rcClient.bottom);
			}

			// Draw the image
			dc.DrawState(ptLocation, sizeImage, m_Bitmap.m_hBitmap, DST_BITMAP);
			m_sizeBmp = sizeImage;
			m_ptBmpLocation = ptLocation;
		}

		if (IsUsingTags())
		{
			// find tags and label parts
			LPTSTR lpstrLeft = NULL;
			int cchLeft = 0;
			LPTSTR lpstrLink = NULL;
			int cchLink = 0;
			LPTSTR lpstrRight = NULL;
			int cchRight = 0;

			T* pT = static_cast<T*>(this);
			pT->CalcLabelParts(lpstrLeft, cchLeft, lpstrLink, cchLink, lpstrRight, cchRight);

			// get label part rects
			dc.SetBkMode(TRANSPARENT);
			HFONT hFontOld = dc.SelectFont(m_hFontNormal);

			if(lpstrLeft != NULL)
				dc.DrawText(lpstrLeft, cchLeft, &rcClient, DT_LEFT | DT_WORDBREAK);

			if (IsUnderlineHover() && m_bHover)	// change font's color
				clrOld = dc.SetTextColor(IsWindowEnabled() ? (m_bHover ? m_clrHover : m_clrLink) : (::GetSysColor(COLOR_GRAYTEXT)));
			else
				clrOld = dc.SetTextColor(IsWindowEnabled() ? (m_bVisited ? m_clrVisited : m_clrLink) : (::GetSysColor(COLOR_GRAYTEXT)));

			if (m_hFont != NULL && (!IsUnderlineHover() || (IsUnderlineHover() && m_bHover)))
				dc.SelectFont(m_hFont);
			else
				dc.SelectFont(m_hFontNormal);

			dc.DrawText(lpstrLink, cchLink, &m_rcLink, DT_LEFT | DT_WORDBREAK);

			dc.SetTextColor(clrOld);
			dc.SelectFont(m_hFontNormal);
			if (lpstrRight != NULL)
			{
				RECT rcRight = { m_rcLink.right, m_rcLink.top, rcClient.right, rcClient.bottom };
				dc.DrawText(lpstrRight, cchRight, &rcRight, DT_LEFT | DT_WORDBREAK);
			}

			if(GetFocus() == m_hWnd)
				dc.DrawFocusRect(&m_rcLink);

			dc.SelectFont(hFontOld);
		}
		else
		{
			dc.SetBkMode(TRANSPARENT);
			if (IsUnderlineHover() && m_bHover)	// change font's color
				clrOld = dc.SetTextColor(IsWindowEnabled() ? (m_bHover ? m_clrHover : m_clrLink) : (::GetSysColor(COLOR_GRAYTEXT)));
			else
				clrOld = dc.SetTextColor(IsWindowEnabled() ? (m_bVisited ? m_clrVisited : m_clrLink) : (::GetSysColor(COLOR_GRAYTEXT)));

			HFONT hFontOld = NULL;
			if (m_hFont != NULL && (!IsUnderlineHover() || (IsUnderlineHover() && m_bHover)))
				hFontOld = dc.SelectFont(m_hFont);
			else
				hFontOld = dc.SelectFont(m_hFontNormal);

			LPTSTR lpstrText = (m_lpstrLabel != NULL) ? m_lpstrLabel : m_lpstrHyperLink;

			DWORD dwStyle = GetStyle();
			int nDrawStyle = DT_LEFT;
			if (dwStyle & SS_CENTER)
				nDrawStyle = DT_CENTER;
			else if (dwStyle & SS_RIGHT)
				nDrawStyle = DT_RIGHT;

			dc.DrawText(lpstrText, -1, &m_rcLink, nDrawStyle | DT_WORDBREAK);

			if(GetFocus() == m_hWnd)
				dc.DrawFocusRect(&m_rcLink);

			dc.SetTextColor(clrOld);
			dc.SelectFont(hFontOld);
		}
	}
};

class CBitmapHyperLink : public CBitmapHyperLinkImpl<CBitmapHyperLink> {
public:
	DECLARE_WND_CLASS(_T("WTL_BitmapHyperLink"))
};
