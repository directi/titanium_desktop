/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include <sstream>

#include <kroll/utils/zip_utils.h>
#include <kroll/utils/unzip/unzip.h>
#include <kroll/utils/win32/win32_utils.h>


namespace UTILS_NS
{
	namespace FileUtils
	{
		bool Unzip(const std::string& source, const std::string& destination, 
			UnzipCallback callback, void *data)
		{
			bool success = true;
			std::wstring wideSource(UTILS_NS::UTF8ToWide(source));
			std::wstring wideDestination(UTILS_NS::UTF8ToWide(destination));

			HZIP handle = OpenZip(wideSource.c_str(), 0);
			SetUnzipBaseDir(handle, wideDestination.c_str());

			ZIPENTRY zipEntry; ZeroMemory(&zipEntry, sizeof(ZIPENTRY));

			GetZipItem(handle, -1, &zipEntry);
			int numItems = zipEntry.index;
			if (callback)
			{ 
				std::ostringstream message;
				message << "Starting extraction of " << numItems 
					<< " items from " << source << "to " << destination;
				std::string messageString(message.str());
				callback((char*) messageString.c_str(), 0, numItems, data);
			}

			for (int zi = 0; zi < numItems; zi++) 
			{ 
				ZeroMemory(&zipEntry, sizeof(ZIPENTRY));
				GetZipItem(handle, zi, &zipEntry);

				if (callback)
				{
					std::string name(WideToUTF8(zipEntry.name));
					std::string message("Extracting ");
					message.append(name);
					message.append("...");
					bool result = callback((char*) message.c_str(), zi, numItems, data);
					if (!result)
					{
						success = false;
						break;
					}
				}

				UnzipItem(handle, zi, zipEntry.name);
			}
			CloseZip(handle);
			return success;
		}
	}
}

