#include "sample_enum_mediatypes.h"

#include <new>
#include "sample_pin.h"
#include "sample_utils.h"

SampleEnumMediaTypes::SampleEnumMediaTypes(SamplePin *pin_p)
{
	this->m_ref_count = 1;
	this->m_current_index = 0;

	this->m_pin = pin_p;
	if (this->m_pin)
	{
		this->m_pin->AddRef();
	}
}

SampleEnumMediaTypes::~SampleEnumMediaTypes()
{
	if (this->m_pin)
	{
		this->m_pin->Release();
	}
}

STDMETHODIMP SampleEnumMediaTypes::QueryInterface(REFIID riid, void **ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}

	if (riid == IID_IUnknown || riid == IID_IEnumMediaTypes)
	{
		AddRef();
		*ppv = this;

		return S_OK;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
}

STDMETHODIMP_(ULONG) SampleEnumMediaTypes::AddRef()
{
	return InterlockedIncrement(&m_ref_count);
}

STDMETHODIMP_(ULONG) SampleEnumMediaTypes::Release()
{
	if (!InterlockedDecrement(&m_ref_count))
	{
		delete this;
		return 0;
	}

	return m_ref_count;
}

STDMETHODIMP SampleEnumMediaTypes::Clone(IEnumMediaTypes **ppEnum)
{
	if (!ppEnum)
	{
		return E_POINTER;
	}

	*ppEnum = new (std::nothrow) SampleEnumMediaTypes(m_pin);
	if (!(*ppEnum))
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

STDMETHODIMP SampleEnumMediaTypes::Next(ULONG cMediaTypes, AM_MEDIA_TYPE **ppMediaTypes, ULONG *pcFetched)
{
	if (!ppMediaTypes || (cMediaTypes != 1 && pcFetched == NULL))
	{
		return E_POINTER;
	}

	ULONG nFetched = 0;

	if (m_current_index == 0 && cMediaTypes > 0)
	{
		AM_MEDIA_TYPE *ptr =
			(AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
		if (!ptr)
		{
			return E_OUTOFMEMORY;
		}

		ZeroMemory(ptr, sizeof(AM_MEDIA_TYPE));
		CopyMediaType(ptr, &(m_pin->GetMediaType()));

		*ppMediaTypes = ptr;
		nFetched = 1;
		m_current_index++;
	}

	if (pcFetched)
	{
		*pcFetched = nFetched;
	}

	return (nFetched == cMediaTypes) ? S_OK : S_FALSE;
}

STDMETHODIMP SampleEnumMediaTypes::Reset()
{
	m_current_index = 0;
	return S_OK;
}

STDMETHODIMP SampleEnumMediaTypes::Skip(ULONG cMediaTypes)
{
	m_current_index += cMediaTypes;

	return ((ULONG)m_current_index > 1) ? S_FALSE : S_OK;
}
