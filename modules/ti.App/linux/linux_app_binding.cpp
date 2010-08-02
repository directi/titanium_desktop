/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include <kroll/kroll.h>
#include "../app_binding.h"

namespace ti
{

void AppBinding::Restart(const ValueList& args, KValueRef result)
{
	Host* host = Host::GetInstance();
	std::string cmdline(host->GetApplication()->arguments.at(0));

	// Remove all quotes.
	size_t i = cmdline.find('\"');
	while (i != std::string::npos)
	{
		cmdline.replace(i, 1, "");
		i = cmdline.find('\"');
	}

	std::string script = "\"" + cmdline + "\" &";
	if (system(script.c_str()) == -1)
		throw ValueException::FromString("Failed to start new process.");

	host->Exit(0);
}

void AppBinding::Exec(const ValueList& args, KValueRef result)
{
	if (args.size() > 0)
	{
		std::string command = args.at(0)->ToString();
		std::wstring wcommand(command.begin(), command.end());
		wchar_t *execargs[100];
		int i = 0;
		for(; i < args.size(); i++)
		{
			if(i == 99) // we limit commandline args to 100
			{
				break;
			}
			std::string param = args.at(i)->ToString();
			std::wstring wparam(param.begin(), param.end());
			execargs[i] = (wchar_t *)wparam.c_str();
		}
		execargs[i] = NULL;
		//::_wexecvp(wcommand.c_str(), execargs);
	}
}
}
