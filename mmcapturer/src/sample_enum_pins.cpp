#include "sample_enum_pins.h"

#include <new>
#include "sample_filter.h"

SampleEnumPins::SampleEnumPins(SampleFilter *filter_p, int current_index_p)
{
	this->m_ref_count = 1;
	this->m_filter = filter_p;
	if (this->m_filter)
	{
		m_filter->AddRef();
	}

	this->m_current_index = current_index_p;
}

SampleEnumPins::~SampleEnumPins()
{
	if (this->m_filter)
	{
		m_filter->Release();
	}
}

STDMETHODIMP SampleEnumPins::QueryInterface(REFIID riid, void **ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}

	if (riid == IID_IUnknown || riid == IID_IEnumPins)
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

STDMETHODIMP_(ULONG) SampleEnumPins::AddRef()
{
	return (ULONG)InterlockedIncrement(&m_ref_count);
}

STDMETHODIMP_(ULONG) SampleEnumPins::Release()
{
	if (!InterlockedDecrement(&m_ref_count))
	{
		delete this;
		return 0;
	}

	return (ULONG)m_ref_count;
}


STDMETHODIMP SampleEnumPins::Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched)
{
	if (!ppPins || (cPins != 1 && pcFetched == NULL))
	{
		return E_POINTER;
	}

	UINT nFetched = 0;

	if (m_current_index == 0 && cPins > 0)
	{
		IPin *pPin = (IPin*)m_filter->GetPin();

		*ppPins = pPin;
		if (pPin)
		{
			pPin->AddRef();
			nFetched = 1;

			m_current_index++;
		}
		else
		{
			nFetched = 0;
		}
	}

	if (pcFetched)
	{
		*pcFetched = nFetched;
	}

	return (nFetched == cPins) ? S_OK : S_FALSE;
}

STDMETHODIMP SampleEnumPins::Skip(ULONG cPins)
{
	m_current_index += cPins;
	if (m_current_index > 1)
	{
		return S_FALSE;
	}

	return S_OK;
}

STDMETHODIMP SampleEnumPins::Reset()
{
	m_current_index = 0;
	return S_OK;
}

STDMETHODIMP SampleEnumPins::Clone(IEnumPins **ppEnum)
{
	*ppEnum = new (std::nothrow) SampleEnumPins(m_filter, m_current_index);
	return (*ppEnum == NULL) ? E_OUTOFMEMORY : S_OK;
}
