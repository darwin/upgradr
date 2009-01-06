////////////////////////////////////////////////////////////////////////////
//	Copyright (C): 2003, Ilya Solnyshkin
//  www.bizon.org/ilya
//
//	Email :isolnyshkin@yahoo.com
//
//	File: 
//	Date: 07.10.2003 16:56:01
//	Description : NONE
//	Update History: NONE
//	      
//	      
//	Do expect bugs.
//	Please let me know of any bugs/mods/improvements.
//	and I will try to fix/incorporate them into this file.
//	Enjoy!
//
//	Description : Programming - It's my Life 
////////////////////////////////////////////////////////////////////////////
//
// Modifications:
//
//	2004-05-31	By Pascal Hurni
//				Added WTL namespace prefix so that this code can mix in an MFC app.
//				OnPaint(): the heap allocated CFont was not deleted. Fixed.
//
////////////////////////////////////////////////////////////////////////////
#ifndef _GRADIENT_STATIC_H_
#define _GRADIENT_STATIC_H_

#if _MSC_VER > 1000 
#pragma once
#endif // _MSC_VER > 1000

#include <atlmisc.h>

class CGradientStaticT : public CWindowImpl< CGradientStaticT>
{
protected:
	CString m_sCaption;
	int		m_iLeftSpacing;
	long	clLeft;
	long	clRight;
	long	clText;
	int		m_iAlign;
private:
	void DrawGradRect( WTL::CPaintDC *pDC, CRect r, COLORREF cLeft, COLORREF cRight)
	{
	   CRect stepR;					// rectangle for color's band
	   COLORREF color;				// color for the bands
	   float fStep = ((float)r.Width())/255.0f;	// width of color's band

	   for(int iOnBand = 0; iOnBand < 255; iOnBand++) 
	   {
			// set current band
			SetRect(&stepR,
				r.left+(int)(iOnBand * fStep), 
				r.top,
				r.left+(int)((iOnBand+1)* fStep), 
				r.bottom);	
		
			// set current color
			color = RGB((GetRValue(cRight)-GetRValue(cLeft))*((float)iOnBand)/255.0f+GetRValue(cLeft),
				(GetGValue(cRight)-GetGValue(cLeft))*((float)iOnBand)/255.0f+GetGValue(cLeft),
				(GetBValue(cRight)-GetBValue(cLeft))*((float)iOnBand)/255.0f+GetBValue(cLeft));
			// fill current band
			pDC->FillSolidRect(stepR,color);
	   }
	}
public:
	CGradientStaticT ()
	{
		m_iLeftSpacing	= 10;
		clLeft			= GetSysColor( COLOR_INACTIVECAPTION /*COLOR_ACTIVECAPTION*/ );
		clRight			= GetSysColor( COLOR_BTNFACE );
		clText			= GetSysColor( COLOR_CAPTIONTEXT );
	
		m_iAlign = 0;
	}

	void SetWindowText(LPCTSTR a_lpstr)
	{
		::SetWindowText( m_hWnd, a_lpstr );
		Invalidate();
	}
	void SetColor(long cl){ clLeft = cl; };
	void SetGradientColor(long cl){ clRight = cl; };
	void SetTextColor(long cl){ clText = cl; };
	void SetLeftSpacing(int iNoOfPixels){ m_iLeftSpacing = iNoOfPixels; };
	void SetTextAlign(int iAlign ) { m_iAlign = iAlign; }; //0 - left, 1 - center, 2 -right
	void SetReverseGradient()
	{
		COLORREF cTemp = clLeft;
		clLeft = clRight;
		clRight = cTemp;
	}

	DECLARE_WND_SUPERCLASS(0, GetWndClassName())

	BEGIN_MSG_MAP(CGradientStaticT)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)		
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		LRESULT lRet = DefWindowProc(uMsg, wParam, lParam);				
		return lRet;
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		WTL::CPaintDC dc(m_hWnd);
		CRect rect;
		GetClientRect(&rect);

		DrawGradRect( &dc, rect, clLeft, clRight );

		//let's set color defined by user
		::SetTextColor(dc,clText);

		HFONT hfontOld;
		WTL::CFontHandle* pFont = new WTL::CFontHandle( GetFont() );
		TCHAR m_sTEXT[1024];
		GetWindowText( m_sTEXT, 1024 );

		if(pFont)
			hfontOld = (HFONT)::SelectObject(dc.m_hDC, (HFONT)pFont->m_hFont);

		::SetBkMode(dc, TRANSPARENT);
		GetClientRect(&rect);

		if(m_iAlign == 1) // center
			::DrawText(dc, m_sTEXT, -1, &rect, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
		else if(m_iAlign == 0) // left
		{
			rect.left+=m_iLeftSpacing;
			::DrawText(dc, m_sTEXT, -1, &rect, DT_SINGLELINE|DT_VCENTER|DT_LEFT);
		}
		else //right
		{
			rect.right-=m_iLeftSpacing;
			::DrawText(dc, m_sTEXT, -1, &rect, DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
		}

		if(pFont)
		{
			::SelectObject(dc.m_hDC, hfontOld);
			delete pFont;
		}

		return 0;
	}
};

typedef CGradientStaticT CGradientStatic;

#endif //

