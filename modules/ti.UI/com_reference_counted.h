/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _COM_REFERENCE_COUNTED_H_
#define _COM_REFERENCE_COUNTED_H_
#include <Unknwn.h>
/*
 * This is a COM-specific wrapper of ReferenceCounted. 
 */


class COMReferenceCounted : public ReferenceCounted, public IUnknown
{
public:
	// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = 0;
		if (IsEqualGUID(riid, IID_IUnknown))
		{
			*ppvObject = static_cast<IUnknown*>(this);
		}
		else
		{
			return E_NOINTERFACE;
		}
		return S_OK;
	}
	
	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		duplicate();
		return referenceCount();
	}
	
	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		release();
		return referenceCount();
	}
};
#endif
