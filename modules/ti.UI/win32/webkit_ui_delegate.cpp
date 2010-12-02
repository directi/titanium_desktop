/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "webkit_ui_delegate.h"
#include "win32_user_window.h"
#include "popup_dialog.h"
#include "win32_ui_binding.h"

#include <tchar.h>
#include <commdlg.h>
#include <comutil.h>

#define MARGIN 20

namespace ti
{

Win32WebKitUIDelegate::Win32WebKitUIDelegate(Win32UserWindow *window_) :
	window(window_),
	nativeContextMenu(0),
	logger(Logger::Get("UI.Win32WebKitUIDelegate")),
	referenceCount(1)
{
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::QueryInterface(
	REFIID riid, void** ppvObject)
{
	*ppvObject = 0;
	if (IsEqualGUID(riid, IID_IUnknown))
		*ppvObject = static_cast<IWebUIDelegate*>(this);
	else if (IsEqualGUID(riid, IID_IWebUIDelegate))
		*ppvObject = static_cast<IWebUIDelegate*>(this);
	else if (IsEqualGUID(riid, IID_IWebUIDelegate2))
		*ppvObject = static_cast<IWebUIDelegate2*>(this);
	else if (IsEqualGUID(riid, IID_IWebUIDelegatePrivate))
		*ppvObject = static_cast<IWebUIDelegatePrivate*>(this);
	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

ULONG STDMETHODCALLTYPE Win32WebKitUIDelegate::AddRef()
{
	return ++referenceCount;
}

ULONG STDMETHODCALLTYPE Win32WebKitUIDelegate::Release()
{
	ULONG new_count = --referenceCount;
	if (!new_count)
		delete(this);
	return new_count;
}

int PropertyBagGetIntProperty(IPropertyBag *bag, const wchar_t *property)
{
	VARIANT value;
	::VariantInit(&value);
	if (bag->Read(property, &value, NULL) == S_OK)
	{
		if (::VariantChangeType(&value, &value, VARIANT_LOCALBOOL, VT_I4) == S_OK)
		{
			return value.intVal;
		}
	}
	return -1;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::createWebViewWithRequest(
	/* [in] */ IWebView *sender,
	/* [in] */ IWebURLRequest *request,
	/* [in] */ IPropertyBag *features,
	/* [retval][out] */ IWebView **newWebView)
{
	AutoPtr<WindowConfig> config(WindowConfig::Default());
	if(request) 
	{
		BSTR burl;
		request->URL(&burl);
		std::string url = _bstr_t(burl);

		if (url.size() > 0)
		{
			config->SetURL(url);
		}
	}

	if (features)
	{
		int fullscreen = PropertyBagGetIntProperty(features, L"fullscreen");
		if (fullscreen != -1)
			config->SetFullscreen(fullscreen == 1);

		int x = PropertyBagGetIntProperty(features, L"x");
		if (x != -1)
			config->SetX(x);

		int y = PropertyBagGetIntProperty(features, L"y");
		if (y != -1)
			config->SetY(y);

		int width = PropertyBagGetIntProperty(features, L"width");
		if (width != -1)
			config->SetWidth(width);

		int height = PropertyBagGetIntProperty(features, L"height");
		if (height != -1)
			config->SetHeight(height);
	}

	AutoUserWindow window(UserWindow::createWindow(config,
		this->window->GetAutoPtr().cast<UserWindow>()));
	window->Open();

	// Win32UserWindow::GetWebView returns a borrowed reference
	// but this delegate should return a new reference, so bump
	// the reference count before returning.
	*newWebView = window.cast<Win32UserWindow>()->GetWebView();
	(*newWebView)->AddRef();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::webViewClose(
	/* [in] */ IWebView *sender)
{
	window->Close();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::webViewFocus(
	/* [in] */ IWebView *sender)
{
	window->Focus();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::webViewUnfocus(
	/* [in] */ IWebView *sender)
{
	window->Unfocus();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::setStatusText(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR text)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::setFrame(
	/* [in] */ IWebView *sender,
	/* [in] */ RECT *frame)
{
	Bounds b =
	{
		frame->left,
		frame->top,
		frame->right - frame->left,
		frame->bottom - frame->top,
	};
	window->SetBounds(b);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::webViewFrame(
	/* [in] */ IWebView *sender,
	/* [retval][out] */ RECT *frame)
{
	Bounds b = window->GetBounds();
	frame->left = b.x;
	frame->top = b.y;
	frame->right = b.x + b.width;
	frame->bottom = b.y + b.height;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::runJavaScriptAlertPanelWithMessage(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR message)
{
	HWND handle = window->GetWindowHandle();
	std::wstring title(::UTF8ToWide(window->GetTitle()));
	std::wstring msg;
	if (message)
		msg.append(bstr_t(message));

	//Win32PopupDialog popupDialog(handle);
	//popupDialog.SetTitle(title);
	//popupDialog.SetMessage(msg);
	//int r = popupDialog.Show();

	MessageBox(0, msg.c_str(), title.c_str(), 0);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::runJavaScriptConfirmPanelWithMessage(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR message,
	/* [retval][out] */ BOOL *result)
{
	HWND handle = window->GetWindowHandle();
	std::wstring title(::UTF8ToWide(window->GetTitle()));
	std::wstring msg;
	if (message)
		msg.append(bstr_t(message));

	int r = MessageBox(0, msg.c_str(), title.c_str(), MB_ICONINFORMATION | MB_OKCANCEL);
	*result = (r == IDOK);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::runJavaScriptTextInputPanelWithPrompt(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR message,
	/* [in] */ BSTR defaultText,
	/* [retval][out] */ BSTR *result)
{
	HWND handle = window->GetWindowHandle();
	std::string title(window->GetTitle());
	std::string msg = _bstr_t(message);
	std::string def;

	if (defaultText)
	{
		def.append(_bstr_t(defaultText));
	}

	Win32PopupDialog popupDialog(handle);
	popupDialog.SetTitle(title);
	popupDialog.SetMessage(msg);
	popupDialog.SetShowInputText(true);
	popupDialog.SetInputText(def);
	popupDialog.SetShowCancelButton(true);
	int r = popupDialog.Show();

	if (r == IDOK)
	{
		_bstr_t bstr1(popupDialog.GetInputText().c_str());
		*result = bstr1.copy();
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::runBeforeUnloadConfirmPanelWithMessage(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR message,
	/* [in] */ IWebFrame *initiatedByFrame,
	/* [retval][out] */ BOOL *result)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::hasCustomMenuImplementation(
	/* [retval][out] */ BOOL *hasCustomMenus)
{
	*hasCustomMenus = TRUE;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::trackCustomPopupMenu(
	/* [in] */ IWebView *sender,
	/* [in] */ OLE_HANDLE inMenu,
	/* [in] */ LPPOINT point)
{
	AutoPtr<Win32Menu> menu = this->window->GetContextMenu().cast<Win32Menu>();

	// No window menu, try to use the application menu.
	if (menu.isNull())
	{
		Win32UIBinding * b = static_cast<Win32UIBinding *>(UIBinding::GetInstance());
		menu = b->GetContextMenu().cast<Win32Menu>();
	}

	if (this->nativeContextMenu) {
		DestroyMenu(this->nativeContextMenu);
		this->nativeContextMenu = 0;
	}

	Host* host = Host::GetInstance();
	if (!menu.isNull())
	{
		this->nativeContextMenu = menu->CreateNative(false);
	}
	else if (host->DebugModeEnabled())
	{
		this->nativeContextMenu = CreatePopupMenu();
		Win32Menu::ApplyNotifyByPositionStyleToNativeMenu(this->nativeContextMenu);
	}

	if (this->nativeContextMenu)
	{
		if (host->DebugModeEnabled())
		{
			AppendMenu(this->nativeContextMenu, MF_SEPARATOR, 1, L"Separator");
			AppendMenu(this->nativeContextMenu,
				MF_STRING, WEB_INSPECTOR_MENU_ITEM_ID, L"Show Inspector");
		}

		TrackPopupMenu(this->nativeContextMenu, TPM_BOTTOMALIGN,
			point->x, point->y, 0, this->window->GetWindowHandle(), NULL);
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::registerUndoWithTarget(
	/* [in] */ IWebUndoTarget *target,
	/* [in] */ BSTR actionName,
	/* [in] */ IUnknown *actionArg)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::removeAllActionsWithTarget(
	/* [in] */ IWebUndoTarget *target)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::setActionTitle(
	/* [in] */ BSTR actionTitle)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::undo()
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::redo()
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::canUndo(
	/* [retval][out] */ BOOL *result)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::canRedo(
	/* [retval][out] */ BOOL *result)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::webViewAddMessageToConsole(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR message,
	/* [in] */ int lineNumber,
	/* [in] */ BSTR url,
	/* [in] */ BOOL isError)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::webViewReceivedFocus(
	/* [in] */ IWebView *sender)
{
	window->FireEvent(Event::FOCUSED);
	window->Focus(); // Focus the WebView and not the main window.
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::webViewLostFocus(
	/* [in] */ IWebView *sender,
	/* [in] */ OLE_HANDLE loseFocusTo)
{
	window->FireEvent(Event::UNFOCUSED);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::doDragDrop(
	/* [in] */ IWebView *sender,
	/* [in] */ IDataObject *dataObject,
	/* [in] */ IDropSource *dropSource,
	/* [in] */ DWORD okEffect,
	/* [retval][out] */ DWORD *performedEffect)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::webViewGetDlgCode(
	/* [in] */ IWebView *sender,
	/* [in] */ UINT keyCode,
	/* [retval][out] */ LONG_PTR *code)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::webViewPainted(
	/* [in] */ IWebView *sender)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::exceededDatabaseQuota(
	/* [in] */ IWebView *sender,
	/* [in] */ IWebFrame *frame,
	/* [in] */ IWebSecurityOrigin *origin,
	/* [in] */ BSTR databaseIdentifier)
{
	origin->setQuota(100 * 1024 * 1024); // 100MB
	return S_OK;
}

static BOOL CALLBACK AbortProc(HDC hdc, int error)
{
	// TODO: It might be night to show a dialog allowing users
	// to cancel print jobs.
	MSG msg;
	while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}

	return TRUE;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::printFrame(
	/* [in] */ IWebView *webView,
	/* [in] */ IWebFrame *frame)
{
	// This code is originally based on the PrintView function in
	// the WinLauncher code of WebKit. We should periodically check
	// there to ensure that this doesn't become obsolete.

	// Open a printing dialog to fetch the HDC of the desired printer.
	PRINTDLG dialog;
	ZeroMemory(&dialog, sizeof(PRINTDLG));
	dialog.lStructSize = sizeof(PRINTDLG);
	dialog.Flags = PD_PRINTSETUP | PD_RETURNDC;
	BOOL dialogResult = ::PrintDlg(&dialog);

	if (!dialogResult) // Error or cancel.
	{
		DWORD reason = CommDlgExtendedError();
		if (!reason) // User cancelled.
			return S_OK;

		logger->Error("Could not print page, dialog error code: %i",
			reason);
		return E_FAIL;
	}

	HDC hdc = dialog.hDC;
	if (!hdc)
	{
		logger->Error("Could not fetch printer HDC.");
		return E_FAIL;
	}

	if (::SetAbortProc(hdc, AbortProc) == SP_ERROR)
	{
		logger->Error("Could not set printer AbortProc.");
		return E_FAIL;
	}
	
	IWebFrame* mainFrame = 0;
	if (FAILED(webView->mainFrame(&mainFrame)))
	{
		return E_POINTER;
	}

	IWebFramePrivate* framePrivate = 0;
	if (FAILED(mainFrame->QueryInterface(&framePrivate)))
	{
		mainFrame->Release();
		return E_POINTER;
	}

	framePrivate->setInPrintingMode(TRUE, hdc);
	UINT pageCount = 0;
	framePrivate->getPrintedPageCount(hdc, &pageCount);

	DOCINFO docInfo;
	ZeroMemory(&docInfo, sizeof(DOCINFO));
	docInfo.cbSize = sizeof(DOCINFO);
	docInfo.lpszDocName = _T("Titanium Document");
	::StartDoc(hdc, &docInfo);

	void* graphicsContext = 0;
	for (size_t page = 1; page <= pageCount; page++)
	{
		::StartPage(hdc);
		framePrivate->spoolPages(hdc, page, page, graphicsContext);
		::EndPage(hdc);
	}

	framePrivate->setInPrintingMode(FALSE, hdc);

	::EndDoc(hdc);
	::DeleteDC(hdc);

	if (mainFrame)
		mainFrame->Release();
	if (framePrivate)
		framePrivate->Release();

	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitUIDelegate::webViewPrintingMarginRect(
	/* [in] */ IWebView *webView,
	/* [retval][out] */ RECT *rect)
{

	if (!webView || !rect)
		return E_POINTER;

	IWebFrame* mainFrame = 0;
	if (FAILED(webView->mainFrame(&mainFrame)))
		return E_FAIL;

	IWebFramePrivate* framePrivate = 0;
	if (FAILED(mainFrame->QueryInterface(&framePrivate))) {
		mainFrame->Release();
		return E_FAIL;
	}

	framePrivate->frameBounds(rect);
	rect->left += MARGIN;
	rect->top += MARGIN;
	rect->right -= MARGIN;
	rect->bottom -= MARGIN;

	framePrivate->Release();
	mainFrame->Release();

	return S_OK;
}

}
