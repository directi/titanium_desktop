/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "diffiehellman_module.h"
#include "diffiehellman_binding.h"

using namespace kroll;
using namespace ti;

namespace ti
{
	KROLL_MODULE(DiffieHellmanModule, STRING(MODULE_NAME), STRING(MODULE_VERSION));
	
	void DiffieHellmanModule::Initialize()
	{

            FILE *file;
            file = fopen("/Users/shashanksingh/libDHfile.txt", "a+"); /*apend file (add text to a file or create a file if it does not exist.*/
            fprintf(file, "%s", "This is just an example :)"); /*writes*/
            fclose(file); /*done!*/
            
            /**
             * @tiapi Creates an instance of a DiffieHellmanBinding object and
             * @tiapi returns it. The client application can use it for
             * @tiapi generation of keys required in a session.
             * @tiresult[DiffieHellmanBinding] The new DiffieHellmanBinding object.
             */
            this->SetMethod("createDH", &DiffieHellmanModule::CreateDH);

            KObjectRef autoThis(this, true);
            host->GetGlobalObject()->SetObject("diffiehellman", autoThis);
            
	}

	void DiffieHellmanModule::Stop()
	{
            host->GetGlobalObject()->SetUndefined("diffiehellman");
	}
        
        void DiffieHellmanModule::CreateDH(const ValueList& args, KValueRef result)
        {
            result->SetObject(new DiffieHellmanBinding());
        }
	
}
