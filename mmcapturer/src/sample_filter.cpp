#include "sample_filter.h"

#include <new>

#include "sample_enum_pins.h"
#include "sample_pin.h"
#include "sample_mediadevice.h"

SampleFilter::SampleFilter(MMDevice* device_p, const GUID& mediaMajorType, const GUID& mediaSubType)
{
	m_ref_count = 1;
	m_state = State_Stopped;
	m_graph = NULL;
	m_device = device_p;

	m_pin = new (std::nothrow) SamplePin(this, mediaMajorType, mediaSubType);
}

SampleFilter::~SampleFilter()
{
	if (m_pin)
	{
		m_pin->Release();
	}

	if (m_graph)
	{
		m_graph->Release();
	}
}

STDMETHODIMP SampleFilter::QueryInterface(REFIID riid, void **ppv)
{
	if (!ppv)
	{
		return E_POINTER;
	}

	if (riid == IID_IUnknown)
	{
		AddRef();
		*ppv = this;
	}
	else if (riid == IID_IPersist)
	{
		AddRef();
		*ppv = (IPersist *)this;
	}
	else if (riid == IID_IMediaFilter)
	{
		AddRef();
		*ppv = (IMediaFilter *)this;
	}
	else if (riid == IID_IBaseFilter)
	{
		AddRef();
		*ppv = (IBaseFilter *)this;
	}
	else if (riid == IID_IAMFilterMiscFlags)
	{
		AddRef();
		*ppv = (IAMFilterMiscFlags *)this;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	return S_OK;
}

STDMETHODIMP_(ULONG) SampleFilter::AddRef()
{
	return InterlockedIncrement(&m_ref_count);
}

STDMETHODIMP_(ULONG) SampleFilter::Release()
{
	if (!InterlockedDecrement(&m_ref_count))
	{
		delete this;
		return 0;
	}

	return m_ref_count;
}

STDMETHODIMP SampleFilter::GetClassID(CLSID *pClsID)
{
	return E_FAIL;
}

STDMETHODIMP SampleFilter::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *State)
{
	if (!State)
	{
		return E_POINTER;
	}

	*State = m_state;

	return S_OK;
}

STDMETHODIMP SampleFilter::SetSyncSource(IReferenceClock *pClock)
{
	return S_OK;
}

STDMETHODIMP SampleFilter::GetSyncSource(IReferenceClock **pClock)
{
	if (!pClock)
	{
		return E_POINTER;
	}

	return S_OK;
}

STDMETHODIMP SampleFilter::Stop()
{
	m_state = State_Stopped;
	return S_OK;
}

STDMETHODIMP SampleFilter::Pause()
{
	m_state = State_Paused;
	return S_OK;
}

STDMETHODIMP SampleFilter::Run(REFERENCE_TIME tStart)
{
	m_state = State_Running;

	return S_OK;
}

STDMETHODIMP SampleFilter::EnumPins(IEnumPins **ppEnum)
{
	if (!ppEnum)
	{
		return E_POINTER;
	}

	*ppEnum = new (std::nothrow) SampleEnumPins(this);
	return (*ppEnum == NULL) ? E_OUTOFMEMORY : S_OK;
}

STDMETHODIMP SampleFilter::FindPin(LPCWSTR Id, IPin **ppPin)
{
	if (!ppPin)
	{
		return E_POINTER;
	}

	*ppPin = m_pin;
	if (*ppPin)
	{
		(*ppPin)->AddRef();
	}
	return S_OK;
}

STDMETHODIMP SampleFilter::QueryFilterInfo(FILTER_INFO *pInfo)
{
	if (!pInfo)
	{
		return E_POINTER;
	}

	StringCbCopyW(pInfo->achName, sizeof(pInfo->achName), L"Sample Filter");

	pInfo->pGraph = m_graph;
	if (m_graph)
	{
		IFilterGraph *graph_ptr = m_graph;
		graph_ptr->AddRef();
	}

	return S_OK;
}

STDMETHODIMP SampleFilter::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
	// never hold a reference count on the Filter Graph Manager. 
	// Doing so creates a circular reference count
	m_graph = pGraph;

	return S_OK;
}

STDMETHODIMP SampleFilter::QueryVendorInfo(LPWSTR *pVendorInfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP_(ULONG) SampleFilter::GetMiscFlags()
{
	return AM_FILTER_MISC_FLAGS_IS_RENDERER;
}
