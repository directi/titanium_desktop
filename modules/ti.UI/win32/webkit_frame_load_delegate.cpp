/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "webkit_frame_load_delegate.h"
#include "win32_user_window.h"

#include <comutil.h>
#include <kroll/javascript/k_kjs_object.h>

namespace ti
{

Win32WebKitFrameLoadDelegate::Win32WebKitFrameLoadDelegate(Win32UserWindow *window) :
	window(window),
	ref_count(1)
{
}

HRESULT STDMETHODCALLTYPE Win32WebKitFrameLoadDelegate::windowScriptObjectAvailable(
		/* [in] */ IWebView *webView,
		/* [in] */ JSContextRef context,
		/* [in] */ JSObjectRef windowScriptObject) 
{ 
	return E_NOTIMPL; 
}


HRESULT STDMETHODCALLTYPE Win32WebKitFrameLoadDelegate::didFinishLoadForFrame(
	IWebView *webView, IWebFrame *frame)
{
	JSGlobalContextRef context = frame->globalContext();
	JSObjectRef global_object = JSContextGetGlobalObject(context);
	KObjectRef frame_global = new KKJSObject(context, global_object);

	IWebDataSource *webDataSource;
	frame->dataSource(&webDataSource);
	IWebMutableURLRequest *urlRequest;
	webDataSource->request(&urlRequest);

	BSTR u;
	urlRequest->URL(&u);
	std::wstring wideURL(u);
	std::string url(::WideToUTF8(wideURL));

	window->FrameLoaded();
	window->PageLoaded(frame_global, url, context);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitFrameLoadDelegate::didClearWindowObject(
	IWebView *webView, JSContextRef context, JSObjectRef windowScriptObject,
	IWebFrame *frame)
{
	JSGlobalContextRef globalContext = frame->globalContext();
	KJSUtil::ProtectContext(globalContext);
	this->window->RegisterJSContext(globalContext);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32WebKitFrameLoadDelegate::didChangeLocationWithinPageForFrame(
	/* [in] */ IWebView *webView,
	/* [in] */ IWebFrame *frame)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE Win32WebKitFrameLoadDelegate::willCloseFrame(
	/* [in] */ IWebView *webView,
	/* [in] */ IWebFrame *frame)
{
	// Get rid of our references...
	KJSUtil::UnprotectContext(frame->globalContext(), true);
	return S_OK;
}


HRESULT STDMETHODCALLTYPE Win32WebKitFrameLoadDelegate::QueryInterface(
	REFIID riid, void **ppvObject)
{
	*ppvObject = 0;
	if (IsEqualGUID(riid, IID_IUnknown))
		*ppvObject = static_cast<IWebFrameLoadDelegate*>(this);
	else if (IsEqualGUID(riid, IID_IWebFrameLoadDelegate))
		*ppvObject = static_cast<IWebFrameLoadDelegate*>(this);
	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

ULONG STDMETHODCALLTYPE Win32WebKitFrameLoadDelegate::AddRef()
{
	return ++ref_count;
}

ULONG STDMETHODCALLTYPE Win32WebKitFrameLoadDelegate::Release()
{
	ULONG new_count = --ref_count;
	if (!new_count) delete(this);

	return new_count;
}

HRESULT STDMETHODCALLTYPE Win32WebKitFrameLoadDelegate::didReceiveTitle(
	/* [in] */ IWebView* webView,
	/* [in] */ BSTR title,
	/* [in] */ IWebFrame* frame)
{
	// Only change the title if the new title was received for the main frame.
	IWebFrame* mainFrame;
	HRESULT hr = webView->mainFrame(&mainFrame);
	if (FAILED(hr))
	{
		Logger::Get("FrameLoadDelegate")->Error("Could not fetch main "
			"frame in didReceiveTitle delegate method");
		return S_OK;
	}
	if (frame != mainFrame)
	{
		mainFrame->Release();
		return S_OK;
	}

	if (title)
	{
		std::string newTitle;
		newTitle.append(bstr_t(title));
		this->window->SetTitle(newTitle);
	}
	mainFrame->Release();
	return S_OK;
}

}
