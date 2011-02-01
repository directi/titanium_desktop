/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "osx_tray_item.h"
#import "osx_tray_item_delegate.h"

using namespace ti;

@implementation OSXTrayItemDelegate
-(id)initWithTray:(OSXTrayItem*)inTrayItem
{
	trayItem = inTrayItem;
	self = [super init];
	return self;
}
-(void)dealloc
{
	[super dealloc];
}
-(void)invoke:(id)sender
{
	trayItem->InvokeCallback();
}
@end


