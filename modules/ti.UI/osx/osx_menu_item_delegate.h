/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <kroll/kroll.h>

#include "osx_menu_item.h"

@interface OSXMenuItemDelegate : NSObject
{
	ti::OSXMenuItem *menuItem;
}
-(id)initWithMenuItem:(ti::OSXMenuItem*)item;
-(void)invoke:(id)sender;
-(ti::OSXMenuItem*)getMenuItem;
@end
