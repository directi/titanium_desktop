/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _NATIVE_WINDOW_H_
#define _NATIVE_WINDOW_H_

#include <kroll/kroll.h>

#import <Cocoa/Cocoa.h>
#import <WebKit/WebInspector.h>

using namespace ti;

@class WebViewDelegate;


@interface NativeWindow : NSWindow
{
	BOOL canReceiveFocus;
	WebView* webView;
	WebViewDelegate* delegate;
	BOOL requiresDisplay;
	OSXUserWindow* userWindow;
	WebInspector* inspector;
	BOOL fullscreen;
	BOOL focused;
	NSRect savedFrame;
}
- (void)setUserWindow:(OSXUserWindow*)inUserWindow;
- (void)setupDecorations:(AutoPtr<WindowConfig>)config;
- (void)setTransparency:(double)transparency;
- (void)setFullscreen:(BOOL)yn;
- (void)close;
- (void)finishClose;
- (void)open;
- (void)frameLoaded;
- (WebView*)webView;
- (NSString*)webViewMainWindowURL;
- (void) SetURL: (const std::string&) url;
- (void) SetContents: (const std::string&) content :(const std::string&) baseURL;
- (OSXUserWindow*)userWindow;
- (void)showInspector:(BOOL)console;
@end

#endif
