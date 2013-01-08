#pragma once
#include <wrl.h>
#include <d2d1_1.h>
#include <d2d1effects.h>
#include <d2d1_1helper.h>
#include <dwrite_1.h>
#include <wincodec.h>
#include <shcore.h>

using namespace Microsoft::WRL;

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DX API errors.
			throw Platform::Exception::CreateException(hr);
		}
	}

	template<class Interface>
	inline void SafeRelease(
		Interface **ppInterfaceToRelease
		)
	{
		if (*ppInterfaceToRelease != NULL)
		{
			(*ppInterfaceToRelease)->Release();

			(*ppInterfaceToRelease) = NULL;
		}
	}
}

