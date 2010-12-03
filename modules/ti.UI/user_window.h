/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _USER_WINDOW_H_
#define _USER_WINDOW_H_

#include <string>
#include <vector>
#include <kroll/kroll.h>

#include "menu.h"
#include <kroll/javascript/javascript_module.h>

#include "app_config.h"
#include "window_config.h"

namespace ti
{
	typedef struct {
		double x;
		double y;
		double width;
		double height;
	} Bounds;

	class UserWindow;
	typedef AutoPtr<UserWindow> AutoUserWindow;

	class UserWindow : public KEventObject
	{
		public:
			// Platform-specific implementation.
			static UserWindow * createWindow(AutoPtr<WindowConfig> config, UserWindow *parent);

			virtual SharedString DisplayString(int levels=3);
			virtual ~UserWindow();
			void UpdateWindowForURL(std::string url);
			void RegisterJSContext(JSGlobalContextRef);
			void InsertAPI(KObjectRef frameGlobal);
			void PageLoaded(KObjectRef scope, std::string &url, JSGlobalContextRef context);
			inline KObjectRef GetDOMWindow() { return this->domWindow; }
			inline Host* GetHost() { return this->host; }
			inline bool IsToolWindow() {return this->config->IsToolWindow(); }
			inline void SetToolWindow(bool toolWindow) {this->config->SetToolWindow(toolWindow); }
			inline bool HasTransparentBackground() { return this->config->HasTransparentBackground(); }
			inline void SetTransparentBackground(bool transparentBackground) { this->config->SetTransparentBackground(transparentBackground); }
			inline std::string GetId() { return this->config->GetID(); }

			void _GetCurrentWindow(const kroll::ValueList&, kroll::KValueRef);
			void _GetDOMWindow(const kroll::ValueList&, kroll::KValueRef);
			void _InsertAPI(const kroll::ValueList&, kroll::KValueRef);
			void _Hide(const kroll::ValueList&, kroll::KValueRef);
			void _Show(const kroll::ValueList&, kroll::KValueRef);
			void _Minimize(const kroll::ValueList&, kroll::KValueRef);
			void _Maximize(const kroll::ValueList&, kroll::KValueRef);
			void _Unminimize(const kroll::ValueList&, kroll::KValueRef);
			void _Unmaximize(const kroll::ValueList&, kroll::KValueRef);
			void _IsMaximized(const kroll::ValueList&, kroll::KValueRef);
			void _IsMinimized(const kroll::ValueList&, kroll::KValueRef);
			void _Focus(const kroll::ValueList&, kroll::KValueRef);
			void _Unfocus(const kroll::ValueList&, kroll::KValueRef);
			void _IsUsingChrome(const kroll::ValueList&, kroll::KValueRef);
			void _SetUsingChrome(const kroll::ValueList&, kroll::KValueRef);
			void _IsToolWindow(const kroll::ValueList&, kroll::KValueRef);
			void _SetToolWindow(const kroll::ValueList&, kroll::KValueRef);
			void _HasTransparentBackground(const kroll::ValueList&, kroll::KValueRef);
			void _SetTransparentBackground(const kroll::ValueList&, kroll::KValueRef);
			void _IsUsingScrollbars(const kroll::ValueList&, kroll::KValueRef);
			void _IsFullscreen(const kroll::ValueList&, kroll::KValueRef);
			void _SetFullscreen(const kroll::ValueList&, kroll::KValueRef);
			void _GetId(const kroll::ValueList&, kroll::KValueRef);
			void _Open(const kroll::ValueList&, kroll::KValueRef);
			void _Close(const kroll::ValueList&, kroll::KValueRef);
			void _GetX(const kroll::ValueList&, kroll::KValueRef);
			double _GetX();
			void _SetX(const kroll::ValueList&, kroll::KValueRef);
			void _SetX(double x);
			void _GetY(const kroll::ValueList&, kroll::KValueRef);
			double _GetY();
			void _SetY(const kroll::ValueList&, kroll::KValueRef);
			void _SetY(double y);
			void _MoveTo(const kroll::ValueList&, kroll::KValueRef);
			void _GetWidth(const kroll::ValueList&, kroll::KValueRef);
			double _GetWidth();
			void _SetWidth(const kroll::ValueList&, kroll::KValueRef);
			void _SetWidth(double width);
			void _GetMaxWidth(const kroll::ValueList&, kroll::KValueRef);
			void _SetMaxWidth(const kroll::ValueList&, kroll::KValueRef);
			void _GetMinWidth(const kroll::ValueList&, kroll::KValueRef);
			void _SetMinWidth(const kroll::ValueList&, kroll::KValueRef);
			void _GetHeight(const kroll::ValueList&, kroll::KValueRef);
			double _GetHeight();
			void _SetHeight(const kroll::ValueList&, kroll::KValueRef);
			void _SetHeight(double height);
			void _GetMaxHeight(const kroll::ValueList&, kroll::KValueRef);
			void _SetMaxHeight(const kroll::ValueList&, kroll::KValueRef);
			void _GetMinHeight(const kroll::ValueList&, kroll::KValueRef);
			void _SetMinHeight(const kroll::ValueList&, kroll::KValueRef);
			void _GetBounds(const kroll::ValueList&, kroll::KValueRef);
			void _SetBounds(const kroll::ValueList&, kroll::KValueRef);
			void _GetTitle(const kroll::ValueList&, kroll::KValueRef);
			void _SetTitle(const kroll::ValueList&, kroll::KValueRef);
			void _GetURL(const kroll::ValueList&, kroll::KValueRef);
			void _SetURL(const kroll::ValueList&, kroll::KValueRef);
			void _IsResizable(const kroll::ValueList&, kroll::KValueRef);
			void _SetResizable(const kroll::ValueList&, kroll::KValueRef);
			void _IsMaximizable(const kroll::ValueList&, kroll::KValueRef);
			void _SetMaximizable(const kroll::ValueList&, kroll::KValueRef);
			void _IsMinimizable(const kroll::ValueList&, kroll::KValueRef);
			void _SetMinimizable(const kroll::ValueList&, kroll::KValueRef);
			void _IsCloseable(const kroll::ValueList&, kroll::KValueRef);
			void _SetCloseable(const kroll::ValueList&, kroll::KValueRef);
			void _IsVisible(const kroll::ValueList&, kroll::KValueRef);
			void _IsActive(const kroll::ValueList&, kroll::KValueRef);
			void _SetVisible(const kroll::ValueList&, kroll::KValueRef);
			void _GetTransparency(const kroll::ValueList&, kroll::KValueRef);
			void _SetTransparency(const kroll::ValueList&, kroll::KValueRef);
			void _GetMenu(const kroll::ValueList&, kroll::KValueRef);
			void _SetMenu(const kroll::ValueList&, kroll::KValueRef);
			void _GetContextMenu(const kroll::ValueList&, kroll::KValueRef);
			void _SetContextMenu(const kroll::ValueList&, kroll::KValueRef);
			void _GetIcon(const kroll::ValueList&, kroll::KValueRef);
			void _SetIcon(const kroll::ValueList&, kroll::KValueRef);
			void _GetParent(const kroll::ValueList&, kroll::KValueRef);
			void _GetChildren(const kroll::ValueList&, kroll::KValueRef);
			void _CreateWindow(const kroll::ValueList&, kroll::KValueRef);
			void _OpenFileChooserDialog(const ValueList& args, KValueRef result);
			void _OpenFolderChooserDialog(const ValueList& args, KValueRef result);
			void _OpenSaveAsDialog(const ValueList& args, KValueRef result);
			void _IsTopMost(const kroll::ValueList&, kroll::KValueRef);
			void _SetTopMost(const kroll::ValueList&, kroll::KValueRef);
			virtual void _ShowInspector(const ValueList& args, KValueRef result);
			void _Flash(const kroll::ValueList&, kroll::KValueRef);
			void _SetContents(const ValueList& args, KValueRef result);
			void SetContents(const std::string& content, const std::string& baseURL);
			bool HasTitaniumObject() { return hasTitaniumObject; }
			virtual void OpenFileChooserDialog(KMethodRef callback, bool multiple,
				std::string& title, std::string& path, std::string& defaultName,
				std::vector<std::string>& types, std::string& typesDescription) = 0;
			virtual void OpenFolderChooserDialog( KMethodRef callback,
				bool multiple, std::string& title, std::string& path,
				std::string& defaultName) = 0;
			virtual void OpenSaveAsDialog(KMethodRef callback, std::string& title,
				std::string& path, std::string& defaultName,
				std::vector<std::string>& types, std::string& typesDescription) = 0;

			// TODO: make these methods non-virtual
			virtual void Hide() = 0;
			virtual void Show() = 0;
			virtual void Minimize() = 0;
			virtual void Maximize() = 0;
			virtual void Unminimize() = 0;
			virtual void Unmaximize() = 0;
			virtual bool IsMaximized() = 0;
			virtual bool IsMinimized() = 0;
			virtual void Focus() = 0;
			virtual void Unfocus() = 0;
			virtual bool IsUsingChrome() = 0;
			virtual bool IsUsingScrollbars() = 0;
			virtual bool IsFullscreen() = 0;
			virtual void Open();
			virtual bool Close();
			void Closed();
			virtual double GetX() = 0;
			virtual void SetX(double x) = 0;
			virtual double GetY() = 0;
			virtual void SetY(double y) = 0;
			virtual void MoveTo(double x, double y) = 0;

			virtual double GetWidth() = 0;
			virtual void SetWidth(double width) = 0;
			virtual double GetMaxWidth() = 0;
			virtual void SetMaxWidth(double width) = 0;
			virtual double GetMinWidth() = 0;
			virtual void SetMinWidth(double width) = 0;
			virtual double GetHeight() = 0;
			virtual void SetHeight(double height) = 0;
			virtual double GetMaxHeight() = 0;
			virtual void SetMaxHeight(double height) = 0;
			virtual double GetMinHeight() = 0;
			virtual void SetMinHeight(double height) = 0;
			virtual Bounds GetBounds();
			virtual Bounds GetBoundsImpl() = 0;
			void SetBounds(Bounds bounds);
			virtual void SetBoundsImpl(Bounds bounds) = 0;
			virtual std::string GetTitle() = 0;
			virtual void SetTitle(const std::string& title);
			virtual void SetTitleImpl(const std::string& title) = 0;
			virtual std::string GetURL() = 0;
			virtual void SetURL(std::string &url) = 0;
			virtual bool IsResizable() = 0;
			virtual void SetResizable(bool resizable);
			virtual void SetResizableImpl(bool resizable) = 0;
			virtual bool IsMaximizable() = 0;
			virtual void SetMaximizable(bool maximizable) = 0;
			virtual bool IsMinimizable() = 0;
			virtual void SetMinimizable(bool minimizable) = 0;
			virtual bool IsCloseable() = 0;
			virtual void SetCloseable(bool closeable) = 0;
			virtual bool IsVisible() = 0;
			virtual double GetTransparency() = 0;
			virtual void SetTransparency(double transparency) = 0;
			virtual void SetFullscreen(bool fullscreen) = 0;
			virtual void SetUsingChrome(bool chrome) = 0;
			virtual void SetMenu(AutoMenu menu) = 0;
			virtual AutoMenu GetMenu() = 0;
			virtual void SetContextMenu(AutoMenu menu) = 0;
			virtual AutoMenu GetContextMenu() = 0;
			virtual void SetIcon(std::string& iconPath) = 0;
			virtual std::string& GetIcon() = 0;
			virtual bool IsTopMost() = 0;
			virtual void SetTopMost(bool topmost) = 0;
			virtual void ShowInspector(bool console=false) = 0;
			virtual void Flash(int timesToFlash) = 0;
			virtual void AppIconChanged() {};
			virtual void AppMenuChanged() {};
			virtual void SetContentsImpl(const std::string& content,  const std::string& baseURL) = 0;

		protected:
			Logger* logger;
			KObjectRef domWindow;
			Host* host;
			AutoPtr<WindowConfig> config;
			AutoUserWindow parent;
			std::vector<AutoUserWindow> children;
			bool active;
			bool initialized;
			std::string iconURL;
			bool hasTitaniumObject;

			UserWindow(AutoPtr<WindowConfig> config, UserWindow *parent);
			virtual AutoUserWindow GetParent();
			virtual void AddChild(AutoUserWindow);
			virtual void RemoveChild(AutoUserWindow);
			void ReadChooserDialogObject(KObjectRef o, bool& multiple,
				std::string& title, std::string& path, std::string& defaultName,
				std::vector<std::string>& types, std::string& typesDescription);
			static void LoadUIJavaScript(JSGlobalContextRef context);

		private:
			DISALLOW_EVIL_CONSTRUCTORS(UserWindow);
	};
}
#endif
