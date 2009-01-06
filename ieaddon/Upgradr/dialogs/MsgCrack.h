#ifndef _INT_MSGCRACK
#define _INT_MSGCRACK

#define MSG_TDM_CLICK_BUTTON(func) \
	if (uMsg == TDM_CLICK_BUTTON) \
	{ \
		SetMsgHandled(TRUE); \
		func((UINT)wParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_CLICK_RADIO_BUTTON(func) \
	if (uMsg == TDM_CLICK_RADIO_BUTTON) \
	{ \
		SetMsgHandled(TRUE); \
		func((UINT)wParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_CLICK_VERIFICATION(func) \
	if (uMsg == TDM_CLICK_VERIFICATION) \
	{ \
		SetMsgHandled(TRUE); \
		func((BOOL)wParam, (BOOL)lParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_ENABLE_BUTTON(func) \
	if (uMsg == TDM_ENABLE_BUTTON) \
	{ \
		SetMsgHandled(TRUE); \
		func((UINT)wParam, (BOOL)lParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_ENABLE_RADIO_BUTTON(func) \
	if (uMsg == TDM_ENABLE_RADIO_BUTTON) \
	{ \
		SetMsgHandled(TRUE); \
		func((UINT)wParam, (BOOL)lParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_UPDATE_ELEMENT_TEXT(func) \
	if (uMsg == TDM_UPDATE_ELEMENT_TEXT) \
	{ \
		SetMsgHandled(TRUE); \
		func((TASKDIALOG_ELEMENTS)wParam, (LPCWSTR) lParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_UPDATE_ICON(func) \
	if (uMsg == TDM_UPDATE_ICON) \
	{ \
		SetMsgHandled(TRUE); \
		func((TASKDIALOG_ICON_ELEMENTS)wParam, lParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_SET_PROGRESS_BAR_POS(func) \
	if (uMsg == TDM_SET_PROGRESS_BAR_POS) \
	{ \
		SetMsgHandled(TRUE); \
		func((UINT)wParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_SET_PROGRESS_BAR_RANGE(func) \
	if (uMsg == TDM_SET_PROGRESS_BAR_RANGE) \
	{ \
		SetMsgHandled(TRUE); \
		func((UINT)LOWORD(lParam), (UINT)HIWORD(lParam)); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_SET_PROGRESS_BAR_MARQUEE(func) \
	if (uMsg == TDM_SET_PROGRESS_BAR_MARQUEE) \
	{ \
		SetMsgHandled(TRUE); \
		func((BOOL)wParam, (UINT)lParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_SET_PROGRESS_BAR_STATE(func) \
	if (uMsg == TDM_SET_PROGRESS_BAR_STATE) \
	{ \
		SetMsgHandled(TRUE); \
		func((UINT)wParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MSG_TDM_NAVIGATE(func) \
	if (uMsg == TDM_NAVIGATE) \
	{ \
		SetMsgHandled(TRUE); \
		func(uMsg, lParam); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#endif // _INT_MSGCRACK
