/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "menu.h"
#include "clipboard.h"
#include "ui_binding.h"
#include "ui_module.h"
#include "notification.h"

#include <string>

namespace ti
{
	UIBinding::UIBinding(Host* host) :
		KAccessorObject("UI"),
		host(host)
	{
		this->Set("CENTERED", Value::NewInt(DEFAULT_POSITION));

		this->SetMethod("createNotification", &UIBinding::_CreateNotification);
		this->SetMethod("createMenu", &UIBinding::_CreateMenu);
		this->SetMethod("createMenuItem", &UIBinding::_CreateMenuItem);
		this->SetMethod("createCheckMenuItem", &UIBinding::_CreateCheckMenuItem);
		this->SetMethod("createSeparatorMenuItem", &UIBinding::_CreateSeparatorMenuItem);
		this->SetMethod("setMenu", &UIBinding::_SetMenu);
		this->SetMethod("getMenu", &UIBinding::_GetMenu);
		this->SetMethod("setContextMenu", &UIBinding::_SetContextMenu);
		this->SetMethod("getContextMenu", &UIBinding::_GetContextMenu);
		this->SetMethod("setIcon", &UIBinding::_SetIcon);
		this->SetMethod("addTray", &UIBinding::_AddTray);
		this->SetMethod("clearTray", &UIBinding::_ClearTray);

		this->SetMethod("bounceDockIcon", &UIBinding::_BounceDockIcon);
		this->SetMethod("setDockIcon", &UIBinding::_SetDockIcon);
		this->SetMethod("setDockMenu", &UIBinding::_SetDockMenu);
		this->SetMethod("setBadge", &UIBinding::_SetBadge);
		this->SetMethod("setBadgeImage", &UIBinding::_SetBadgeImage);
		this->SetMethod("getIdleTime", &UIBinding::_GetIdleTime);
		this->SetMethod("getOpenWindows", &UIBinding::_GetOpenWindows);
		this->SetMethod("getWindows", &UIBinding::_GetOpenWindows);
		this->SetMethod("getMainWindow", &UIBinding::_GetMainWindow);
		this->SetMethod("createWindow", &UIBinding::_CreateWindow);

		// Initialize notifications
		this->SetBool("nativeNotifications", Notification::InitializeImpl());

		this->SetObject("Clipboard", new Clipboard());
		Logger::AddLoggerCallback(&UIBinding::Log);
	}

	void UIBinding::CreateMainWindow(AutoPtr<WindowConfig> config)
	{
		this->mainWindow = UserWindow::createWindow(config, 0);
		this->mainWindow->Open();

		try
		{
			const std::string appIcon = host->GetApplication()->getImage();
			if (!appIcon.empty())
				this->_SetIcon(appIcon);
		}
		catch (ValueException& e)
		{
			const std::string ss = e.ToString();
			Logger::Get("UI")->Error("Could not set default icon: %s", ss.c_str());
		}
	}

	UserWindow *UIBinding::GetMainWindow()
	{
		return this->mainWindow;
	}

	void UIBinding::_CreateWindow(const ValueList& args, KValueRef result)
	{
		AutoPtr<WindowConfig> config(0);
		if (args.size() > 0 && args.at(0)->IsObject())
		{
			config = WindowConfig::FromProperties(args.GetObject(0));
		}
		else if (args.size() > 0 && args.at(0)->IsString())
		{
			std::string url(args.GetString(0));
			config = AppConfig::Instance()->GetWindowByURL(url);
			if (config.isNull())
			{
				config = WindowConfig::Default();
				config->SetURL(url);
			}
		}

		// If we still do not have a configuration, just use the default.
		if (config.isNull())
			config = WindowConfig::Default();

		result->SetObject(UserWindow::createWindow(config, 0));
	}

	void UIBinding::ErrorDialog(std::string msg)
	{
		std::cerr << msg << std::endl;
	}

	void UIBinding::Unload()
	{
		Logger::RemoveLoggerCallback(&UIBinding::Log);
	}

	UIBinding::~UIBinding()
	{
//		this->ClearTray();

		// Shutdown notifications
		Notification::ShutdownImpl();
	}

	Host* UIBinding::GetHost()
	{
		return host;
	}

	std::vector<AutoUserWindow>& UIBinding::GetOpenWindows()
	{
		return this->openWindows;
	}

	void UIBinding::AddToOpenWindows(AutoUserWindow window)
	{
		this->openWindows.push_back(window);
	}

	void UIBinding::RemoveFromOpenWindows(AutoUserWindow window)
	{
		static Logger* logger = Logger::Get("UI");
		std::vector<AutoUserWindow>::iterator w = openWindows.begin();
		while (w != openWindows.end())
		{
			if ((*w).get() == window.get())
			{
				w = this->openWindows.erase(w);
				return;
			}
			else
			{
				w++;
			}
		}
		logger->Warn("Tried to remove a non-existant window: %lx", (long int) window.get());
	}

	void UIBinding::_GetOpenWindows(const ValueList& args, KValueRef result)
	{
		KListRef list = new StaticBoundList();
		std::vector<AutoUserWindow>::iterator w = openWindows.begin();
		while (w != openWindows.end()) {
			list->Append(Value::NewObject(*w++));
		}
		result->SetList(list);
	}

	void UIBinding::_GetMainWindow(const ValueList& args, KValueRef result)
	{
		result->SetObject(this->mainWindow);
	}

	void UIBinding::_CreateNotification(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createNotification", "?o");
		Notification *n = new Notification();

		if (args.GetValue(0)->IsObject())
			n->Configure(args.GetObject(0));

		result->SetObject(n);
	}

	void UIBinding::_CreateMenu(const ValueList& args, KValueRef result)
	{
		result->SetObject(__CreateMenu(args));
	}

	AutoMenu UIBinding::__CreateMenu(const ValueList& args)
	{
		// call into the native code to retrieve the menu
		return this->CreateMenu();
	}

	void UIBinding::_CreateMenuItem(const ValueList& args, KValueRef result)
	{
		result->SetObject(__CreateMenuItem(args));
	}

	AutoMenuItem UIBinding::__CreateMenuItem(const ValueList& args)
	{
		args.VerifyException("createMenuItem", "?s m|0 s|0");
		const std::string label = args.GetString(0);
		KValueRef eventListener = args.GetValue(1);
		const std::string iconURL = args.GetString(2);

		AutoMenuItem item = this->CreateMenuItem();
		if (!label.empty())
			item->SetLabel(label);
		if (!iconURL.empty())
			item->SetIcon(iconURL);
		if (!eventListener.isNull())
			item->AddEventListener(Event::CLICKED, eventListener);

		return item;
	}


	void UIBinding::_CreateCheckMenuItem(const ValueList& args, KValueRef result)
	{
		result->SetObject(__CreateCheckMenuItem(args));
	}

	AutoMenuItem UIBinding::__CreateCheckMenuItem(const ValueList& args)
	{
		args.VerifyException("createCheckMenuItem", "?s m|0");
		const std::string label = args.GetString(0);
		KValueRef eventListener = args.GetValue(1);

		AutoMenuItem item = this->CreateCheckMenuItem();
		if (!label.empty())
			item->SetLabel(label);
		if (!eventListener.isNull())
			item->AddEventListener(Event::CLICKED, eventListener);

		return item;
	}

	void UIBinding::_CreateSeparatorMenuItem(const ValueList& args, KValueRef result)
	{
		result->SetObject(__CreateSeparatorMenuItem(args));
	}

	AutoMenuItem UIBinding::__CreateSeparatorMenuItem(const ValueList& args)
	{
		return this->CreateSeparatorMenuItem();
	}

	void UIBinding::_SetMenu(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setMenu", "o|0");
		AutoMenu menu(args.GetObject(0, 0).cast<Menu>());

		this->SetMenu(menu); // platform-specific impl

		// Notify all windows that the app menu has changed.
		std::vector<AutoUserWindow>::iterator i = openWindows.begin();
		while (i != openWindows.end()) {
			(*i++)->AppMenuChanged();
		}
	}

	void UIBinding::_GetMenu(const ValueList& args, KValueRef result)
	{
		AutoMenu menu = this->GetMenu();
		if (menu)
		{
			result->SetObject(menu);
		}
		else
		{
			result->SetNull();
		}
	}

	void UIBinding::_SetContextMenu(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setContextMenu", "o|0");
		AutoMenu menu(args.GetObject(0, 0).cast<Menu>());
		this->SetContextMenu(menu);
	}

	void UIBinding::_GetContextMenu(const ValueList& args, KValueRef result)
	{
		AutoMenu menu = this->GetContextMenu();
		result->SetObject(menu);
	}

	void UIBinding::_SetIcon(const ValueList& args, KValueRef result)
	{
		args.VerifyException("setIcon", "s|0");

		std::string iconURL;
		if (args.size() > 0)
			iconURL = args.GetString(0);
		this->_SetIcon(iconURL);
	}

	void UIBinding::_SetIcon(const std::string &iconURL)
	{
		std::string iconPath;
		this->iconURL = iconURL;
		if (!iconURL.empty())
			iconPath = URLUtils::URLToPath(this->iconURL);

		this->SetIcon(iconPath); // platform-specific impl

		// Notify all windows that the app menu has changed.
		std::vector<AutoUserWindow>::iterator i = openWindows.begin();
		while (i != openWindows.end()) {
			(*i++)->AppIconChanged();
		}
	}

	void UIBinding::_AddTray(const ValueList& args, KValueRef result)
	{
		args.VerifyException("createTrayIcon", "s,?m");
		TrayItem * item = this->AddTray(args.GetString(0), args.GetValue(1));
		this->trayItems.push_back(item);
		result->SetObject(item);
	}

	void UIBinding::_ClearTray(const ValueList& args, KValueRef result)
	{
		this->ClearTray();
	}

	void UIBinding::ClearTray()
	{
		std::vector<TrayItem *>::iterator i = this->trayItems.begin();
		while (i != this->trayItems.end())
		{
			(*i++)->Remove();
		}
		this->trayItems.clear();
	}

	void UIBinding::UnregisterTrayItem(TrayItem* item)
	{
		std::vector<TrayItem *>::iterator i = this->trayItems.begin();
		while (i != this->trayItems.end())
		{
			if (*i == item)
			{
				i = this->trayItems.erase(i);
			}
			else
			{
				i++;
			}
		}
	}

	void UIBinding::_BounceDockIcon(const ValueList& args, KValueRef result)
	{
		this->BounceDockIcon();
	}

	void UIBinding::_SetDockIcon(const ValueList& args, KValueRef result)
	{
		std::string iconPath;
		if (args.size() > 0) {
			std::string in = args.GetString(0);
			iconPath = URLUtils::URLToPath(in);
		}
		this->SetDockIcon(iconPath);
	}

	void UIBinding::_SetDockMenu(const ValueList& args, KValueRef result)
	{
		AutoMenu menu(args.GetObject(0, 0).cast<Menu>());
		this->SetDockMenu(menu);
	}

	void UIBinding::_SetBadge(const ValueList& args, KValueRef result)
	{
		std::string badgeText;
		if (args.size() > 0) {
			badgeText = args.GetString(0);
		}

		this->SetBadge(badgeText);
	}

	void UIBinding::_SetBadgeImage(const ValueList& args, KValueRef result)
	{
		std::string iconPath;
		if (args.size() > 0) {
			std::string in = args.GetString(0);
			iconPath = URLUtils::URLToPath(in);
		}

		this->SetBadgeImage(iconPath);
	}

	void UIBinding::_GetIdleTime(
		const ValueList& args,
		KValueRef result)
	{
		result->SetDouble(this->GetIdleTime());
	}

    void UIBinding::Log(Logger::Level level, const std::string& message)
	{
		std::string methodName("debug_orig");
		switch(level) 
		{
		case Logger::LFATAL:
		case Logger::LCRITICAL:
		case Logger::LERROR:
			methodName = "error_orig";
			break;
		case Logger::LWARN:
			methodName = "warn_orig";
			break;
		case Logger::LNOTICE:
		case Logger::LINFO:
		case Logger::LDEBUG:
		case Logger::LTRACE:
			break;
		}

		std::string script("window.console.");
		script.append(methodName);
		script.append("('");
		std::string escapedMessage(message);
		static const std::string delimiters("'\\");
		size_t pos = 0;
		while(true)
		{
			pos = escapedMessage.find_first_of(delimiters, pos);
			if(pos == string::npos) break;
			escapedMessage.insert(pos, "\\");
			pos += 2;
		} 

		script.append(escapedMessage);
		script.append("')");

		ValueList args = ValueList(Value::NewString(script));
		if(IsMainThread())
		{
			PrivateLog(args);
		}
		else
		{
			KMethodRef function = new KFunctionPtrMethod(&UIBinding::PrivateLog);
			RunOnMainThread(function, args);
		}
	}

    KValueRef UIBinding::PrivateLog(const ValueList& args)
	{
		ASSERT_MAIN_THREAD
		std::string script = args.at(0)->ToString();
		std::vector<AutoUserWindow>& openWindows = UIModule::GetBinding()->GetOpenWindows();
		for (size_t i = 0; i < openWindows.size(); i++)
		{
			if(! openWindows[i]->HasTitaniumObject()) continue;
			KObjectRef domWindow = openWindows[i]->GetDOMWindow();
			AutoPtr<KKJSObject> kobj = domWindow.cast<KKJSObject>();
			if(kobj.isNull()) continue;

			try 
			{
				KJSUtil::Evaluate(kobj->GetContext(), script.c_str(), NULL);
			} 
			catch (ValueException& exception)
			{
				fprintf(stderr, "Error logging: JSException: %s\n", exception.ToString().c_str()); 
			}
			catch (std::exception &e)
			{
				fprintf(stderr, "Error logging: std::exception: %s\n", e.what());
			}
			catch(...) 
			{
				// Ignore for now atleast.
				fprintf(stderr, "Yikes, lost a log message\n");
			}
		}
		return Value::Undefined;
	}
}

