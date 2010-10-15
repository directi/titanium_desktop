/** * Appcelerator Titanium - licensed under the Apache Public License 2
 * SEE LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "../ui_module.h"
#include <sstream>
#include <cmath>

#define SetFlag(x,flag,b) ((b) ? x |= flag : x &= ~flag)
#define UnsetFlag(x,flag) (x &= ~flag)=
#define USERWINDOW_WINDOW_CLASS L"Win32UserWindow"
#define MEANING_OF_LIFE 42
#define MAX_FILE_DIALOG_STRING 1024

using namespace ti;

static void ParseMultipleSelectedFiles(OPENFILENAME* ofn,
	std::vector<std::wstring>& files);

static void* SetWindowUserData(HWND hwnd, void* userData)
{
	return reinterpret_cast<void*>(SetWindowLongPtr(hwnd, GWLP_USERDATA,
		reinterpret_cast<LONG_PTR> (userData)));
}

static void* GetWindowUserData(HWND hWnd)
{
	return reinterpret_cast<void*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

static void HandleHResultError(std::string message, HRESULT result, bool fatal=false)
{
	static Logger* logger = Logger::Get("UI.Win32UserWindow");

	message.append(": ");
	switch (result)
	{
		case REGDB_E_CLASSNOTREG:
			message.append("REGDB_E_CLASSNOTREG");
			break;
		case CLASS_E_NOAGGREGATION:
			message.append("CLASS_E_NOAGGREGATION");
			break;
		case E_NOINTERFACE:
			message.append("E_NOINTERFACE");
			break;
		case E_UNEXPECTED:
			message.append("E_UNEXPECTED");
			break;
		case E_OUTOFMEMORY:
			message.append("E_OUTOFMEMORY");
			break;
		case E_INVALIDARG:
			message.append("E_INVALIDARG");
			break;
		default:
			message.append("Unknown Error (");
			message.append(KList::IntToChars(result));
			message.append(")");
			break;
	}

	logger->Error(message);
	Win32UIBinding::ErrorDialog(message);
	if (fatal)
		exit(1);
	else
		throw ValueException::FromString(message);
}

static LRESULT CALLBACK UserWindowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32UserWindow* window = Win32UserWindow::FromWindow(hWnd);
	if (!window)
		return DefWindowProc(hWnd, message, wParam, lParam);

	LRESULT handled = 0;
	if (window->HasTransparentBackground())
	{
		if (message == WM_ERASEBKGND)
		{
			handled = 1;
		}
		else if (message == WM_TIMER && wParam == window->GetTimer())
		{
			window->UpdateBitmap();
			handled = 1;
		}
		else
		{
			IWebView* webView = window->GetWebView();
			if (webView)
			{
				// TODO: CARL Needs to fix this.
				//handled = webView->forwardingWindowProc(
				//	reinterpret_cast<OLE_HANDLE>(hWnd), message, wParam, lParam);

				// WebKit sometimes causes WM_PAINT messages to fire. We need to ensure
				// we call DefWindowProc in this case, otherwise Windows will assume
				// that it was not handled and continue to flood us with WM_PAINT messages.
				if (message == WM_PAINT)
				{
					// Calling UpdateBitmap here, assures smooth resizing.
					window->UpdateBitmap();
					handled = 0;
				}
			}
		}
	}

	switch (message)
	{
		case WM_CLOSE:
			if (!window->Close())
				handled = 1;
			break;

		case WM_GETMINMAXINFO:
			window->GetMinMaxInfo((MINMAXINFO*) lParam);
			handled = 1;
			break;

		case WM_SIZE:
			window->FireEvent(Event::RESIZED);
			if (wParam == SIZE_MAXIMIZED)
			{
				window->FireEvent(Event::MAXIMIZED);
				window->ResizeSubViews();
			}
			else if (wParam == SIZE_MINIMIZED)
			{
				window->FireEvent(Event::MINIMIZED);
			}
			else
			{
				window->ResizeSubViews();
			}
			handled = 1;
			break;

		case WM_SETFOCUS:
			// The focus event will be fired by the UIDelegate
			window->Focus(); // Focus the WebView and not the main window.
			handled = 1;
			break;

		case WM_MOVE:
			window->FireEvent(Event::MOVED);
			break;

		case WM_SHOWWINDOW:
			window->FireEvent(((BOOL)wParam) ? Event::SHOWN : Event::HIDDEN);
			break;

		case WM_MENUCOMMAND:
		{
			HMENU nativeMenu = (HMENU) lParam;
			UINT position = (UINT) wParam;
			UINT itemId = GetMenuItemID(nativeMenu, position);

			if (itemId == WEB_INSPECTOR_MENU_ITEM_ID)
			{
				handled = 1;
				window->ShowInspector(false);
			}
			else
			{
				handled = Win32MenuItem::HandleClickEvent(nativeMenu, position);
			}
		}
		break;
	}

	if (!handled)
		return DefWindowProc(hWnd, message, wParam, lParam);
	else
		return handled;
}

static void RegisterWindowClass()
{
	static bool classInitialized = false;
	if (!classInitialized)
	{
		HINSTANCE instanceHandle = GetModuleHandle(NULL);
		WNDCLASSEXW wcex;
		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = UserWindowWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 4;
		wcex.hInstance = instanceHandle;
		wcex.hIcon = 0;
		wcex.hIconSm = 0;
		wcex.hCursor = LoadCursor(instanceHandle, IDC_ARROW);
		wcex.hbrBackground = 0;
		wcex.lpszMenuName = L"";
		wcex.lpszClassName = USERWINDOW_WINDOW_CLASS;

		ATOM result = RegisterClassExW(&wcex);
		if (result == NULL)
		{
			Logger::Get("UI.Win32UserWindow")->Error("Error Registering Window Class: %d", GetLastError());
		}

		classInitialized = true;
	}
}

/*static*/
Win32UserWindow* Win32UserWindow::FromWindow(HWND hWnd)
{
	return reinterpret_cast<Win32UserWindow*>(GetWindowUserData(hWnd));
}

/*static*/
AutoPtr<Win32UserWindow> Win32UserWindow::FromWebView(IWebView* webView)
{
	std::vector<AutoUserWindow>& openWindows(UIBinding::GetInstance()->GetOpenWindows());
	for (size_t i = 0; i < openWindows.size(); i++)
	{
		AutoPtr<Win32UserWindow> userWindow(openWindows.at(i).cast<Win32UserWindow>());
		if (userWindow->webView == webView)
		{
			return userWindow;
		}
	}
	return 0;
}


DWORD Win32UserWindow::GetStyleFromConfig()
{
	DWORD style = WS_EX_APPWINDOW;
	if (config->IsToolWindow())
		style = WS_EX_TOOLWINDOW;

	if (config->GetTransparency() < 1.0 || this->HasTransparentBackground())
		style |= WS_EX_LAYERED;

	return style;
}

void Win32UserWindow::InitWindow()
{
	RegisterWindowClass();
	std::wstring titleW = ::UTF8ToWide(config->GetTitle());
	this->windowHandle = CreateWindowExW(GetStyleFromConfig(), USERWINDOW_WINDOW_CLASS,
		titleW.c_str(), 0, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		0, 0, GetModuleHandle(NULL), (LPVOID)this);

	SetWindowUserData(this->windowHandle, reinterpret_cast<void*>(this));

	if (!this->windowHandle)
	{
		throw ValueException::FromFormat("Error Creating Window: %s",
			Win32Utils::QuickFormatMessage(GetLastError()));
	}

	this->SetTransparency(config->GetTransparency());
}

static void setPrivatePrefs(IWebPreferences *prefs, bool debugModeEnabled, std::string appid) {
	IWebPreferencesPrivate* privatePrefs = NULL;
	HRESULT hr = prefs->QueryInterface(IID_IWebPreferencesPrivate, (void**) &privatePrefs);
	if (FAILED(hr) || !privatePrefs)
		HandleHResultError("Error getting IWebPreferencesPrivate", hr, true);

	privatePrefs->setDeveloperExtrasEnabled(debugModeEnabled);
	privatePrefs->setDatabasesEnabled(true);
	privatePrefs->setLocalStorageEnabled(true);
	privatePrefs->setOfflineWebApplicationCacheEnabled(true);
	privatePrefs->setAllowUniversalAccessFromFileURLs(true);
	
	_bstr_t dbPath = ::UTF8ToWide(FileUtils::GetApplicationDataDirectory(appid)).c_str();
	privatePrefs->setLocalStorageDatabasePath(dbPath.copy());
	privatePrefs->Release();
}

void Win32UserWindow::InitWebKit()
{
	HRESULT hr = WebKitCreateInstance(CLSID_WebView, 0, IID_IWebView,
		(void**) &(this->webView));
	if (FAILED(hr))
		HandleHResultError("Error creating WebKitWebView", hr, true);

	// TI-303 we need to add safari UA to our UA to resolve broken
	// sites that look at Safari and not WebKit for UA.
	_bstr_t ua("Version/4.0 Safari/528.16 "PRODUCT_NAME"/"PRODUCT_VERSION);
	webView->setApplicationNameForUserAgent(ua.copy());

	// place our user agent string in the global so we can later use it
	KObjectRef global = host->GetGlobalObject();
	if (global->Get("userAgent")->IsUndefined())
	{
		_bstr_t uaURL("http://titaniumapp.com");
		BSTR uaResp;
		webView->userAgentForURL(uaURL.copy(), &uaResp);
		std::string uaStr = _bstr_t(uaResp);
		global->Set("userAgent", Value::NewString(uaStr.c_str()));
	}

	frameLoadDelegate = new Win32WebKitFrameLoadDelegate(this);
	hr = webView->setFrameLoadDelegate(frameLoadDelegate);
	if (FAILED(hr))
		HandleHResultError("Error setting FrameLoadDelegate", hr, true);

	uiDelegate = new Win32WebKitUIDelegate(this);
	hr = webView->setUIDelegate(uiDelegate);
	if (FAILED(hr))
		HandleHResultError("Error setting UIDelegate", hr, true);

	policyDelegate = new Win32WebKitPolicyDelegate(this);
	hr = webView->setPolicyDelegate(policyDelegate);
	if (FAILED(hr))
		HandleHResultError("Error setting PolicyDelegate", hr, true);

	resourceLoadDelegate = new Win32WebKitResourceLoadDelegate(this);
	hr = webView->setResourceLoadDelegate(resourceLoadDelegate);
	if (FAILED(hr))
		HandleHResultError("Error setting ResourceLoadDelegate", hr, true);

	RECT clientRect;
	GetClientRect(windowHandle, &clientRect);

	OLE_HANDLE oleWindowHandle = NULL;
	if (this->HasTransparentBackground())
		oleWindowHandle = (OLE_HANDLE) this->windowHandle;

	IWebViewPrivate* webViewPrivate;
	hr = webView->QueryInterface(IID_IWebViewPrivate, (void**) &webViewPrivate);
	if (FAILED(hr))
		HandleHResultError("Error getting IWebViewPrivate", hr);

	if (!this->HasTransparentBackground())
	{
		hr = webView->setHostWindow((OLE_HANDLE) windowHandle);
		if (FAILED(hr))
			HandleHResultError("Error setting host window", hr, true);
	}

	webViewPrivate->setTransparent(this->HasTransparentBackground());

	std::string appid(AppConfig::Instance()->GetAppID());

	IWebPreferences *prefs = NULL;
	hr = WebKitCreateInstance(CLSID_WebPreferences, 0, IID_IWebPreferences,
		(void**) &prefs);
	if (FAILED(hr) || !prefs)
		HandleHResultError("Error getting IWebPreferences", hr, true);

	IWebPreferences *sharedPrefs = NULL;
	hr = prefs->standardPreferences(&sharedPrefs);
	if (FAILED(hr) || !sharedPrefs)
		HandleHResultError("Error getting SharedWebPreferences", hr, true);
	sharedPrefs->setAutosaves(FALSE);
	setPrivatePrefs(sharedPrefs, host->DebugModeEnabled(), appid);
	sharedPrefs->Release();

	_bstr_t pi(appid.c_str());
	prefs->initWithIdentifier(pi.copy(), &prefs);
	sharedPrefs->setAutosaves(FALSE);
	prefs->setCacheModel(WebCacheModelDocumentBrowser);
	prefs->setPlugInsEnabled(true);
	prefs->setJavaEnabled(true);
	prefs->setJavaScriptEnabled(true);
	prefs->setJavaScriptCanOpenWindowsAutomatically(true);
	prefs->setDOMPasteAllowed(true);
	prefs->setShouldPrintBackgrounds(true);

	// I think this is overkill but I'm just leaving it here for now.
	setPrivatePrefs(prefs, host->DebugModeEnabled(), appid);

	// allow app:// and ti:// to run with local permissions (cross-domain ajax,etc)
	_bstr_t appProto("app");
	webView->registerURLSchemeAsLocal(appProto.copy());

	_bstr_t tiProto("ti");
	webView->registerURLSchemeAsLocal(tiProto.copy());

	// TODO: CARL needs to fix this.
	//_bstr_t inspector_url("ti://runtime/WebKit.resources/inspector/inspector.html");
	//webInspector->setInspectorURL(inspector_url.copy());

	hr = webView->initWithFrame(clientRect, 0, 0); //TODO: CARL needs to fix this, oleWindowHandle);
	if (FAILED(hr))
		HandleHResultError("Could not intialize WebView with frame", hr, true);

	webView->setPreferences(prefs);
	prefs->Release();

	// Get the WebView's HWND
	hr = webViewPrivate->viewWindow((OLE_HANDLE*) &viewWindowHandle);
	if (FAILED(hr))
		HandleHResultError("Error getting WebView HWND", hr);

	// Get the WebView's WebInspector
	hr = webViewPrivate->inspector(&webInspector);
	if (FAILED(hr) || !webInspector)
		HandleHResultError("Error getting WebInspector HWND", hr);

	webViewPrivate->Release();

	hr = webView->mainFrame(&mainFrame);
	if (FAILED(hr) || !webInspector)
		HandleHResultError("Error getting WebView main frame", hr);
}

static void GetChromeSize(Bounds& chromeSize, DWORD windowStyle)
{
	RECT rect;
	rect.left = rect.top = 0;
	rect.bottom = rect.right = 100;

	// Don't take into account the menu size (last argument to AdjustWindowRect)
	// when getting the chrome size. This matches the behavior on Linux.
	AdjustWindowRect(&rect, windowStyle, 0);
	chromeSize.width = rect.right - rect.left - 100;
	chromeSize.height = rect.bottom - rect.top - 100;
}

Win32UserWindow::Win32UserWindow(AutoPtr<WindowConfig> config, AutoUserWindow& parent) :
	UserWindow(config, parent),
	frameLoadDelegate(0),
	uiDelegate(0),
	policyDelegate(0),
	resourceLoadDelegate(0),
	restoreStyles(0),
	windowHandle(0),
	viewWindowHandle(0),
	webkitBitmap(0),
	timer(0),
	webView(0),
	mainFrame(0),
	webInspector(0),
	requiresDisplay(true),
	menu(0),
	activeMenu(0),
	nativeMenu(0),
	contextMenu(0),
	iconPath("")
{
	logger = Logger::Get("UI.Win32UserWindow");
}

AutoUserWindow UserWindow::CreateWindow(AutoPtr<WindowConfig> config, AutoUserWindow parent)
{
	return new Win32UserWindow(config, parent);
}

Win32UserWindow::~Win32UserWindow()
{

	if (webView)
		webView->Release();

	if (mainFrame)
		mainFrame->Release();
}

typedef struct DrawChildWindowData_
{
	HWND parentWindow;
	HDC hdc;
} DrawChildWindowData;

static BOOL CALLBACK DrawChildWindow(HWND hWnd, LPARAM lParam)
{
	DrawChildWindowData* data = reinterpret_cast<DrawChildWindowData*>(lParam);
	HDC hdc = data->hdc;

	// Figure out what the child window offset is inside the parent window.
	RECT childWindowRect;
	GetWindowRect(hWnd, &childWindowRect);
	RECT windowRect;
	GetWindowRect(data->parentWindow, &windowRect);
	int xOffset = childWindowRect.left - windowRect.left;
	int yOffset = childWindowRect.top - windowRect.top;

	// Modify the world transform so that the plugin is positioned properly.
	XFORM originalTransform;
	GetWorldTransform(hdc, &originalTransform);
	XFORM transform = originalTransform;
	transform.eDx = xOffset;
	transform.eDy = yOffset;
	SetWorldTransform(hdc, &transform);

	// SelectClipRgn does not appear to honor our world transform
	// so offset it by the child window position inside the parent window.
	HRGN windowRegion = CreateRectRgn(0, 0, 0, 0);
	GetWindowRgn(hWnd, windowRegion);
	OffsetRgn(windowRegion, xOffset, yOffset);
	SelectClipRgn(hdc, windowRegion);

	// Tell the plugin to paint onto our HDC.
	SendMessage(hWnd, WM_PRINTCLIENT, reinterpret_cast<WPARAM>(hdc),
		PRF_CLIENT | PRF_CHILDREN | PRF_OWNED);

	SetWorldTransform(hdc, &originalTransform);
	return TRUE;
}

void Win32UserWindow::UpdateBitmap()
{
	if (!this->HasTransparentBackground())
		return;

	// FIXME: Sometimes the WebView will initialize and not do an initial
	// paint. We need to force the initial paint in WebKit instead of forcing
	// it here.
	if (!this->webkitBitmap)
	{
		SendMessage(windowHandle, WM_PAINT, 0, 0);
		return;
	}

	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, webkitBitmap);

	// Force child windows (plugins) to draw onto our backing store.
	// The graphics mode of the HDC must be GM_ADVANCED so it will be capable
	// of selecting world transforms (for positioning plugin draws).
	SetGraphicsMode(hdcMem, GM_ADVANCED);
	DrawChildWindowData data = {windowHandle, hdcMem};
	EnumChildWindows(windowHandle, DrawChildWindow, reinterpret_cast<LPARAM>(&data));

	BLENDFUNCTION blendFunction;
	blendFunction.BlendOp = AC_SRC_OVER;
	blendFunction.BlendFlags = 0;
	// TODO: Find another method to apply all-over transparency.
	// blendFunction.SourceConstantAlpha = floor(config->GetTransparency() * 255);
	blendFunction.SourceConstantAlpha = 0xFF;
	blendFunction.AlphaFormat = AC_SRC_ALPHA;

	POINT bitmapOrigin = {0, 0};

	Bounds bounds = GetBounds();
	POINT windowOrigin = {bounds.x, bounds.y};
	SIZE windowSize = {bounds.width, bounds.height};
	UpdateLayeredWindow(windowHandle, hdcScreen,
		&windowOrigin, &windowSize,
		hdcMem, &bitmapOrigin,
		0, &blendFunction, ULW_ALPHA);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdcScreen);
}

void Win32UserWindow::ResizeSubViews()
{
	if (this->HasTransparentBackground() || !viewWindowHandle)
		return;

	RECT rcClient;
	GetClientRect(windowHandle, &rcClient);
	MoveWindow(viewWindowHandle, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, TRUE);
}

HWND Win32UserWindow::GetWindowHandle()
{
	return this->windowHandle;
}

void Win32UserWindow::Hide()
{
	ShowWindow(windowHandle, SW_HIDE);
}

void Win32UserWindow::Show()
{
	if (requiresDisplay)
	{
		this->requiresDisplay = false;

		// Stuff to do when we want a show to happen.
		this->SetupState();
		this->SetTopMost(config->IsTopMost() && config->IsVisible());
		this->ResizeSubViews();

		// Ensure we have valid restore values
		restoreBounds = GetBounds();
		restoreStyles = GetWindowLong(windowHandle, GWL_STYLE);

		ShowWindow(windowHandle, SW_SHOW);
		UpdateWindow(this->windowHandle);
		
		if (this->windowHandle != viewWindowHandle)
			UpdateWindow(viewWindowHandle);
		SetFocus(viewWindowHandle);

		// TODO: Start the transparent-background update timer here,
		// because UpdateBitmap sometimes needs to force the WebView
		// to paint by sending a WM_PAINT message. Once that bug is
		// fixed in WebKit, we can move this back to InitWindow.
		if (this->HasTransparentBackground())
			this->timer = ::SetTimer(this->windowHandle,
				MEANING_OF_LIFE, 1000/40, 0);
	}
	else
	{
		ShowWindow(windowHandle, SW_SHOW);
	}
}

void Win32UserWindow::Minimize()
{
	ShowWindow(windowHandle, SW_MINIMIZE);
}

void Win32UserWindow::Unminimize()
{
	ShowWindow(windowHandle, SW_RESTORE);
}

bool Win32UserWindow::IsMinimized()
{
	if (!requiresDisplay)
		return IsIconic(windowHandle) != 0;
	else
		return config->IsMinimized();
}

void Win32UserWindow::Maximize()
{
	ShowWindow(windowHandle, SW_MAXIMIZE);
}

void Win32UserWindow::Unmaximize()
{
	ShowWindow(windowHandle, SW_RESTORE);
}

bool Win32UserWindow::IsMaximized()
{
	if (!requiresDisplay)
		return IsZoomed(windowHandle) != 0;
	else
		return config->IsMaximized();
}

void Win32UserWindow::Focus()
{
	if (viewWindowHandle)
		SetFocus(viewWindowHandle);
}

void Win32UserWindow::Unfocus()
{
	// SetFocus sends a WM_KILLFOCUS message to the window that has focus.
	// By sending NULL, we basically turn off keystrokes to window that had focus.
	HWND focusedWindow = GetFocus();
	if (focusedWindow == windowHandle || ::IsChild(windowHandle, focusedWindow))
	{
		SetFocus(NULL);
	}
}

void Win32UserWindow::Open()
{
	this->InitWindow();
	this->SetupDecorations();
	this->InitWebKit();
	this->SetupIcon();

	UserWindow::Open();
	this->SetupFrame();

	FireEvent(Event::OPENED);
}

bool Win32UserWindow::Close()
{
	// Hold a reference here so we can still get the value of
	// this->timer and this->active even after calling ::Closed
	// which will remove us from the open window list and decrement
	// the reference count.
	AutoUserWindow keep(this, true);

	if (!this->active)
		return false;

	UserWindow::Close();

	// If the window is still active at this point, it
	// indicates an event listener has cancelled this close event.
	if (!this->active)
	{
		this->RemoveOldMenu();
		UserWindow::Closed();

		if (this->timer)
			::KillTimer(this->windowHandle, this->timer);

		DestroyWindow(windowHandle);
	}

	return !this->active;
}

double Win32UserWindow::GetX()
{
	return GetBounds().x;
}

void Win32UserWindow::SetX(double x)
{
	this->SetupFrame();
}

double Win32UserWindow::GetY()
{
	return GetBounds().y;
}

void Win32UserWindow::SetY(double y)
{
	this->SetupFrame();
}

void Win32UserWindow::MoveTo(double x, double y)
{
	this->SetupFrame();
}

double Win32UserWindow::GetWidth()
{
	return GetBounds().width;
}

void Win32UserWindow::SetWidth(double width)
{
	this->SetupFrame();
}

double Win32UserWindow::GetHeight()
{
	return GetBounds().height;
}

void Win32UserWindow::SetHeight(double height)
{
	this->SetupFrame();
}

double Win32UserWindow::GetMaxWidth()
{
	return this->config->GetMaxWidth();
}

void Win32UserWindow::SetMaxWidth(double width)
{
	this->SetupFrame();
}

double Win32UserWindow::GetMinWidth()
{
	return this->config->GetMinWidth();
}

void Win32UserWindow::SetMinWidth(double width)
{
	this->SetupFrame();
}

double Win32UserWindow::GetMaxHeight()
{
	return this->config->GetMaxHeight();
}

void Win32UserWindow::SetMaxHeight(double height)
{
	this->SetupFrame();
}

double Win32UserWindow::GetMinHeight()
{
	return this->config->GetMinHeight();
}

void Win32UserWindow::SetMinHeight(double height)
{
	this->SetupFrame();
}

Bounds Win32UserWindow::GetBoundsImpl()
{
	// We need to use window rectangle to get the position, since we
	// position independent of chrome and the client rectange for the size
	// since the size is adjusted for chrome.
	RECT clientRect, windowRect;
	GetClientRect(windowHandle, &clientRect);
	GetWindowRect(windowHandle, &windowRect);

	Bounds bounds =
	{
		windowRect.left, windowRect.top,
		clientRect.right - clientRect.left, clientRect.bottom - clientRect.top
	};

	if (!activeMenu.isNull())
		bounds.height += GetSystemMetrics(SM_CYMENU);

	return bounds;
}

void Win32UserWindow::SetupFrame()
{
	Bounds bounds;
	bounds.x = this->config->GetX();
	bounds.y = this->config->GetY();
	bounds.width = this->config->GetWidth();
	bounds.height = this->config->GetHeight();
	this->SetBounds(bounds);
}

void Win32UserWindow::SetBoundsImpl(Bounds bounds)
{
	HWND desktop = GetDesktopWindow();
	RECT desktopRect, boundsRect;

	GetWindowRect(desktop, &desktopRect);
	if (bounds.x == DEFAULT_POSITION)
	{
		bounds.x = (desktopRect.right - bounds.width) / 2;
		this->config->SetX(bounds.x);
	}
	if (bounds.y == DEFAULT_POSITION)
	{
		bounds.y = (desktopRect.bottom - bounds.height) / 2;
		this->config->SetY(bounds.y);
	}

	UINT flags = SWP_SHOWWINDOW | SWP_NOZORDER;
	if (!this->config->IsVisible())
	{
		flags = SWP_HIDEWINDOW;
	}

	boundsRect.left = bounds.x;
	boundsRect.right = bounds.x + bounds.width;
	boundsRect.top = bounds.y;
	boundsRect.bottom = bounds.y + bounds.height;

	if (this->config->IsUsingChrome())
	{
		bounds.width += this->chromeSize.width;
		bounds.height += this->chromeSize.height;
	}

	MoveWindow(windowHandle, bounds.x, bounds.y, bounds.width, bounds.height, TRUE);
}

void Win32UserWindow::SetTitleImpl(const std::string& title)
{
	std::wstring titleW = ::UTF8ToWide(title);
	SetWindowTextW(windowHandle, titleW.c_str());
}

void Win32UserWindow::SetURL(std::string& url_)
{
	std::string url = URLUtils::NormalizeURL(url_);
	Win32UIBinding::SetProxyForURL(url);

	IWebMutableURLRequest* request = 0;
	std::wstring method = L"GET" ;

	HRESULT hr = WebKitCreateInstance(CLSID_WebMutableURLRequest, 0, 
		IID_IWebMutableURLRequest, (void**) &request);
	if (FAILED(hr))
		HandleHResultError("Error creating WebMutableURLRequest", hr, true);

	std::wstring wurl = ::UTF8ToWide(url);
	hr = request->initWithURL(SysAllocString(wurl.c_str()), 
		WebURLRequestUseProtocolCachePolicy, 60);
	if (FAILED(hr))
	{
		request->Release();
		std::string error("Error initialiazing WebMutableURLRequest for ");
		error.append(url);
		HandleHResultError(error, hr, true);
	}

	hr = request->setHTTPMethod(SysAllocString(method.c_str()));
	if (FAILED(hr))
	{
		request->Release();
		std::string error("Error setting HTTP method for ");
		error.append(url);
		HandleHResultError(error, hr, true);
	}

	hr = mainFrame->loadRequest(request);
	if (FAILED(hr))
	{
		request->Release();
		std::string error("Error starting load request for ");
		error.append(url);
		HandleHResultError(error, hr, true);
	}
}

void Win32UserWindow::SetResizableImpl(bool resizable)
{
	this->SetupDecorations();
}

void Win32UserWindow::SetMaximizable(bool maximizable)
{
	this->SetupDecorations();
}

void Win32UserWindow::SetMinimizable(bool minimizable)
{
	this->SetupDecorations();
}

void Win32UserWindow::SetCloseable(bool closeable)
{
	this->SetupDecorations();
}

bool Win32UserWindow::IsVisible()
{
	return IsWindowVisible(windowHandle);
}

void Win32UserWindow::SetTransparency(double transparency)
{
	if (!HasTransparentBackground())
	{
		SetWindowLong(windowHandle, GWL_EXSTYLE, GetStyleFromConfig());
		SetLayeredWindowAttributes(windowHandle, 0,
			(BYTE) floor(config->GetTransparency() * 255), LWA_ALPHA);
	}
}

void Win32UserWindow::SetFullscreen(bool fullscreen)
{
	if (fullscreen)
	{
		restoreBounds = GetBounds();
		restoreStyles = GetWindowLong(windowHandle, GWL_STYLE);

		HMONITOR hmon = MonitorFromWindow(
			this->windowHandle, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo(hmon, &mi))
		{
			SetWindowLong(windowHandle, GWL_STYLE, 0);
			SetWindowPos(windowHandle, NULL, 0, 0, 
				mi.rcMonitor.right - mi.rcMonitor.left,
				mi.rcMonitor.bottom - mi.rcMonitor.top,
				SWP_SHOWWINDOW);
		}

		FireEvent(Event::FULLSCREENED);
	}
	else
	{
		SetWindowLong(windowHandle, GWL_STYLE, restoreStyles);
		SetBounds(restoreBounds);
		FireEvent(Event::UNFULLSCREENED);
	}
}

void Win32UserWindow::SetMenu(AutoMenu menu)
{
	this->menu = menu.cast<Win32Menu>();
	this->SetupMenu();
}

AutoMenu Win32UserWindow::GetMenu()
{
	return this->menu;
}

void Win32UserWindow::SetContextMenu(AutoMenu menu)
{
	this->contextMenu = menu.cast<Win32Menu>();
}

AutoMenu Win32UserWindow::GetContextMenu()
{
	return this->contextMenu;
}

void Win32UserWindow::SetIcon(std::string& iconPath)
{
	this->iconPath = iconPath;
	this->SetupIcon();
}

static HICON GetDefaultIcon()
{
	// Set initial window icon to icon associated with exe file.
	static HICON defaultIcon = 0;

	if (!defaultIcon)
	{
		static HINSTANCE instanceHandle = GetModuleHandle(NULL);
		wchar_t exePath[MAX_PATH];
		GetModuleFileNameW(instanceHandle, exePath, MAX_PATH);
		defaultIcon = ExtractIconW(instanceHandle, exePath, 0);
	}

	return defaultIcon;
}

void Win32UserWindow::SetupIcon()
{
	std::string iconPath(this->iconPath);
	if (iconPath.empty())
		iconPath = static_cast<Win32UIBinding*>(UIBinding::GetInstance())->GetIcon();

	HICON smallIcon = 0;
	HICON largeIcon = 0;
	if (!iconPath.empty())
	{
		int smallIconSizeX = GetSystemMetrics(SM_CXSMICON);
		int smallIconSizeY = GetSystemMetrics(SM_CYSMICON);
		int largeIconSizeX = GetSystemMetrics(SM_CXICON);
		int largeIconSizeY = GetSystemMetrics(SM_CYICON);

		largeIcon = Win32UIBinding::LoadImageAsIcon(iconPath, largeIconSizeX, largeIconSizeY);
		smallIcon = Win32UIBinding::LoadImageAsIcon(iconPath, smallIconSizeX, smallIconSizeY);
	}

	if (!largeIcon)
		largeIcon = GetDefaultIcon();
	if (!smallIcon)
		smallIcon = GetDefaultIcon();
	if (largeIcon)
		SendMessageA(windowHandle, (UINT) WM_SETICON, ICON_BIG, (LPARAM) largeIcon);
	if (smallIcon)
		SendMessageA(windowHandle, (UINT) WM_SETICON, ICON_SMALL, (LPARAM) smallIcon);
}

std::string& Win32UserWindow::GetIcon()
{
	return iconPath;
}

void Win32UserWindow::SetUsingChrome(bool chrome)
{
	this->SetupDecorations();
}

void Win32UserWindow::SetupDecorations()
{
	long windowStyle = GetWindowLong(this->windowHandle, GWL_STYLE);

	SetFlag(windowStyle, WS_OVERLAPPED, config->IsUsingChrome());
	SetFlag(windowStyle, WS_SIZEBOX,
		config->IsUsingChrome() && config->IsResizable());
	SetFlag(windowStyle, WS_OVERLAPPEDWINDOW,
		config->IsUsingChrome() && config->IsResizable());
	SetFlag(windowStyle, WS_SYSMENU,
		config->IsUsingChrome() && config->IsCloseable());
	SetFlag(windowStyle, WS_CAPTION, config->IsUsingChrome());
	SetFlag(windowStyle, WS_BORDER, config->IsUsingChrome());
	SetFlag(windowStyle, WS_MAXIMIZEBOX, config->IsMaximizable());
	SetFlag(windowStyle, WS_MINIMIZEBOX, config->IsMinimizable());

	SetWindowLong(this->windowHandle, GWL_STYLE, windowStyle);
	GetChromeSize(this->chromeSize, windowStyle);

	// If the window is visible and the first frame load has completed
	// then we need to hide and show the window so that the decorations
	// refresh.
	if (!requiresDisplay && config->IsVisible())
	{
		Hide();
		Show();
	}
}

void Win32UserWindow::SetupState()
{
	if (config->IsFullscreen())
	{
		this->SetFullscreen(true);
	}
	else if (config->IsMaximized())
	{
		this->Maximize();
	}
	else if (config->IsMinimized())
	{
		this->Minimize();
	}
}

void Win32UserWindow::AppMenuChanged()
{
	if (this->menu.isNull())
	{
		this->SetupMenu();
	}
}

void Win32UserWindow::AppIconChanged()
{
	this->SetupIcon();
}

void Win32UserWindow::RemoveOldMenu()
{
	if (!this->activeMenu.isNull() && this->nativeMenu) {
		this->activeMenu->DestroyNative(this->nativeMenu);
	}

	if (this->windowHandle != NULL && this->nativeMenu) {
		::SetMenu(this->windowHandle, NULL);
	}

	this->activeMenu = 0;
	this->nativeMenu = 0;

}

void Win32UserWindow::SetupMenu()
{
	AutoPtr<Win32Menu> menu = this->menu;

	// No window menu, try to use the application menu.
	if (menu.isNull())
	{
		Win32UIBinding* b = static_cast<Win32UIBinding*>(UIBinding::GetInstance());
		menu = b->GetMenu().cast<Win32Menu>();
	}

	// Only do this if the menu is actually changing.
	if (menu.get() != this->activeMenu.get()) {

		if (!menu.isNull() && this->windowHandle) {
			this->RemoveOldMenu();

			HMENU newNativeMenu = menu->CreateNativeTopLevel(true);
			::SetMenu(this->windowHandle, newNativeMenu);
			this->nativeMenu = newNativeMenu;
		}
		this->activeMenu = menu;
	}
}

// Called by FrameLoadDelegate to let the window know it's loaded.
void Win32UserWindow::FrameLoaded()
{
	if (this->requiresDisplay && this->config->IsVisible())
	{
		this->Show();
	}
}

bool Win32UserWindow::IsTopMost()
{
	return this->config->IsTopMost();
}

void Win32UserWindow::SetTopMost(bool topmost)
{
	if (topmost)
	{
		SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE);
	}
	else
	{
		SetWindowPos(windowHandle, HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE);
	}
}

void Win32UserWindow::ShowInspector(bool console)
{
	if (this->webInspector)
	{
		if (console)
		{
			this->webInspector->showConsole();
		}
		else
		{
			this->webInspector->show();
		}
	}
}

void Win32UserWindow::Flash(int timesToFlash)
{
	FLASHWINFO fwi;
	ZeroMemory(&fwi, sizeof(FLASHWINFO));
	fwi.cbSize = sizeof(FLASHWINFO);
	fwi.hwnd = windowHandle;
	fwi.dwFlags = FLASHW_ALL;
	fwi.uCount = timesToFlash;
	fwi.dwTimeout = 0;
	FlashWindowEx(&fwi);
}

void Win32UserWindow::OpenFileChooserDialog(KMethodRef callback, bool multiple,
	std::string& title, std::string& path, std::string& defaultName,
	std::vector<std::string>& types, std::string& typesDescription)
{

	KListRef results = this->SelectFile(
		false, multiple, title, path, defaultName, types, typesDescription);
	callback->Call(ValueList(Value::NewList(results)));
}

void Win32UserWindow::OpenFolderChooserDialog(KMethodRef callback, bool multiple,
	std::string& title, std::string& path, std::string& defaultName)
{
	KListRef results = SelectDirectory(multiple, title, path, defaultName);
	callback->Call(ValueList(Value::NewList(results)));
}

void Win32UserWindow::OpenSaveAsDialog(KMethodRef callback, std::string& title,
	std::string& path, std::string& defaultName,
	std::vector<std::string>& types, std::string& typesDescription)
{
	KListRef results = SelectFile(true, false, title, path, defaultName,
		 types, typesDescription);
	callback->Call(ValueList(Value::NewList(results)));
}

KListRef Win32UserWindow::SelectFile(bool saveDialog, bool multiple, std::string& title,
	std::string& path, std::string& defaultName, std::vector<std::string>& types,
	std::string& typesDescription)
{
	std::wstring filter;
	std::wstring typesDescriptionW = ::UTF8ToWide(typesDescription);
	if (types.size() > 0)
	{
		//"All\0*.*\0Test\0*.TXT\0";
		if (typesDescription.size() == 0)
		{
			// Reasonable default?
			typesDescriptionW = L"Selected Files";
		}
		filter.append(typesDescriptionW);
		filter.push_back(L'\0');
		
		for (int i = 0; i < types.size(); i++)
		{
			std::string type = types.at(i);
			std::wstring typeW = ::UTF8ToWide(type);
			//multiple filters: "*.TXT;*.DOC;*.BAK"
			size_t found = type.find("*.");
			if (found != 0)
			{
				filter.append(L"*.");
			}
			filter.append(typeW);
			filter.append(L";");
		}
		filter.push_back(L'\0');
	}

	OPENFILENAME ofn;

	std::wstring pathW = ::UTF8ToWide(path);
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->windowHandle;

	// Windows may not null-terminate the string it puts here, so we zero it.
	wchar_t filenameW[MAX_FILE_DIALOG_STRING];
	ZeroMemory(&filenameW, MAX_FILE_DIALOG_STRING * sizeof(wchar_t));
	wcscpy(filenameW, ::UTF8ToWide(defaultName).c_str());
	ofn.lpstrFile = filenameW;

	ofn.nMaxFile = MAX_FILE_DIALOG_STRING;
	ofn.lpstrFilter = (LPWSTR) (filter.size() == 0 ? 0 : filter.c_str());
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = 0;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = (LPWSTR) (pathW.length() == 0 ? 0 : pathW.c_str());
	ofn.Flags = OFN_EXPLORER;

	std::wstring titleW;
	if (!title.empty())
	{
		titleW = ::UTF8ToWide(title);
		ofn.lpstrTitle = titleW.c_str();
	}

	if (!saveDialog)
	{
		ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	}

	if (multiple)
	{
		ofn.Flags |= OFN_ALLOWMULTISELECT;
	}

	
	BOOL result;
	if (saveDialog)
	{
		result = ::GetSaveFileName(&ofn);
	}
	else
	{
		result = ::GetOpenFileName(&ofn);
	}

	// A zero-return value here indicates either an error or that the user
	// cancelled the action (CommDlgExtendedError returns 0). We should
	// return a helpful exception if it's an error.
	if (!result)
	{
		DWORD code = CommDlgExtendedError();
		if (code == 0)
			return new StaticBoundList();

		throw ValueException::FromFormat(
			"File dialog action failed with error code: %i", code);
	}

	// From:  http://msdn.microsoft.com/en-us/library/ms646839(VS.85).aspx
	// If multiple files have been selected there will be two '\0' characters
	// at the end of this array of characters, so if we enabled multiple file
	// selected, just check for that second '\0'.
	KListRef results = new StaticBoundList();
	if (multiple && ofn.lpstrFile[ofn.nFileOffset - 1] == L'\0')
	{
		std::vector<std::wstring> files;
		ParseMultipleSelectedFiles(&ofn, files);
		for (size_t i = 0; i < files.size(); i++)
		{
			results->Append(Value::NewString(
				::WideToUTF8(files[i])));
		}
	}
	else
	{
		results->Append(Value::NewString(::WideToUTF8(
			ofn.lpstrFile)));
	}

	return results;
}

KListRef Win32UserWindow::SelectDirectory(bool multiple, std::string& title,
	std::string& path, std::string& defaultName)
{
	KListRef results = new StaticBoundList();

	BROWSEINFO bi = { 0 };
	std::wstring titleW = ::UTF8ToWide(title);
	bi.lpszTitle = titleW.c_str();
	bi.hwndOwner = this->windowHandle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		wchar_t in_path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, in_path))
		{
			std::wstring inPathW = in_path;
			std::string inPath = ::WideToUTF8(inPathW);
			results->Append(Value::NewString(inPath));
		}

		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}
	}
	return results;
}

void Win32UserWindow::GetMinMaxInfo(MINMAXINFO* minMaxInfo)
{
	if (!minMaxInfo)
		return;


	// If we are in fullscreen mode, minimum and maximum constraints
	// should not apply, so just send the defaults back.
	if (this->IsFullscreen())
	{
		minMaxInfo->ptMaxTrackSize.x = minMaxInfo->ptMaxTrackSize.y = INT_MAX;
		minMaxInfo->ptMinTrackSize.x = minMaxInfo->ptMinTrackSize.y = 0;
		return;
	}

	int maxWidth = (int) GetMaxWidth();
	int minWidth = (int) GetMinWidth();
	int maxHeight = (int) GetMaxHeight();
	int minHeight = (int) GetMinHeight();

	minMaxInfo->ptMaxTrackSize.x = maxWidth == -1 ? INT_MAX : maxWidth;
	minMaxInfo->ptMinTrackSize.x = minWidth == -1 ? 0 : minWidth;
	minMaxInfo->ptMaxTrackSize.y = maxHeight == -1 ? INT_MAX : maxHeight;
	minMaxInfo->ptMinTrackSize.y = minHeight == -1 ? 0 : minHeight;

	// We want the maximum and minimum size of the window to apply to only
	// the client area of the window. The chrome size might change radically,
	// but this will allow developers to set min/max sizes independent of that.
	if (this->config->IsUsingChrome())
	{
		if (minMaxInfo->ptMaxTrackSize.x != INT_MAX)
			minMaxInfo->ptMaxTrackSize.x += this->chromeSize.width;
		if (minMaxInfo->ptMaxTrackSize.y != INT_MAX)
			minMaxInfo->ptMaxTrackSize.y += this->chromeSize.height;

		minMaxInfo->ptMinTrackSize.x += this->chromeSize.width;
		minMaxInfo->ptMinTrackSize.y += this->chromeSize.height;
	}
}

static void ParseMultipleSelectedFiles(OPENFILENAME* ofn,
	std::vector<std::wstring>& files)
{
	// From: http://msdn.microsoft.com/en-us/library/ms646839(VS.85).aspx
	// If the OFN_ALLOWMULTISELECT flag is set and the user selects
	// multiple files, the buffer contains the current directory followed
	// by the file names of the selected files. For Explorer-style dialog
	// boxes, the directory and file name strings are NULL separated, with
	// an extra NULL character after the last file name. 
	std::wstring containingDirectory(ofn->lpstrFile);

	size_t offset = ofn->nFileOffset;
	std::wstring filename(&ofn->lpstrFile[offset]);
	files.push_back(filename);

	// The offset of the start of the next string should be the
	// size of the previous filename plus one. When this becomes
	// NULL we are finished reading the filenames.
	offset += filename.size() + 1;
	while (ofn->lpstrFile[offset] != L'\0')
	{
		filename = &ofn->lpstrFile[offset];
		files.push_back(filename);
		offset += filename.size() + 1;
	}

	// Now we need to construct the full path to these files.
	for (size_t i = 0; i< files.size(); i++)
	{
		files[i] = FileUtils::Join(containingDirectory.c_str(),
			files[i].c_str(), 0);
	}
}

void Win32UserWindow::RedrawMenu()
{
	if (this->windowHandle)
		DrawMenuBar(this->windowHandle);
}

/*static*/
void Win32UserWindow::RedrawAllMenus()
{
	// Notify all windows that the app menu has changed.
	std::vector<AutoUserWindow>& windows = UIBinding::GetInstance()->GetOpenWindows();
	std::vector<AutoUserWindow>::iterator i = windows.begin();
	while (i != windows.end())
	{
		AutoPtr<Win32UserWindow> wuw = (*i++).cast<Win32UserWindow>();
		if (!wuw.isNull())
			wuw->RedrawMenu();
	}
}

void Win32UserWindow::SetContentsImpl(const std::string& content, const std::string& baseURL)
{
	if (!this->mainFrame)
		return;

	_bstr_t bContent(::UTF8ToWide(content).c_str());
	_bstr_t bBaseURL(::UTF8ToWide(baseURL).c_str());
	mainFrame->loadHTMLString(bContent, bBaseURL);
}
