#include "sample_pin.h"

#include <new>

#include "sample_enum_mediatypes.h"
#include "sample_filter.h"
#include "sample_utils.h"

BOOL SamplePin::IsMediaTypeValid(const AM_MEDIA_TYPE *pmt) const
{
	if (pmt->pbFormat)
	{
		if (pmt->subtype != m_media_sub_type ||
			pmt->majortype != m_media_major_type)
		{
			return FALSE;
		}

		if (m_media_major_type == MEDIATYPE_Video)
		{
			const BITMAPINFOHEADER *bih = GetBitmapInfoHeader(*pmt);
			if (!bih || bih->biHeight == 0 || bih->biWidth == 0)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

SamplePin::SamplePin(SampleFilter *filter_p, const GUID& mediaMajorType_p, const GUID& mediaSubType_p)
{
	m_ref_count = 1;
	m_filter = filter_p;

	m_media_major_type = mediaMajorType_p;
	m_media_sub_type = mediaSubType_p;

	m_peer_pin = NULL;
	m_flushing = FALSE;

	ZeroMemory(&m_media_type, sizeof(AM_MEDIA_TYPE));
}

SamplePin::~SamplePin()
{
	if (m_peer_pin)
	{
		m_peer_pin->Release();
	}
}

STDMETHODIMP SamplePin::QueryInterface(REFIID riid, void **ppv)
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
	else if (riid == IID_IPin)
	{
		AddRef();
		*ppv = (IPin *)this;
	}
	else if (riid == IID_IMemInputPin)
	{
		AddRef();
		*ppv = (IMemInputPin *)this;
	}
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	return S_OK;
}

STDMETHODIMP_(ULONG) SamplePin::AddRef()
{
	return (ULONG)InterlockedIncrement(&m_ref_count);
}

STDMETHODIMP_(ULONG) SamplePin::Release()
{
	long refs = InterlockedDecrement(&m_ref_count);
	if (!refs)
	{
		delete this;
		return 0;
	}

	return (ULONG)refs;
}

STDMETHODIMP SamplePin::BeginFlush()
{
	m_flushing = TRUE;
	return S_OK;
}

STDMETHODIMP SamplePin::EndFlush()
{
	m_flushing = FALSE;
	return S_OK;
}

STDMETHODIMP SamplePin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
	if (State_Running == m_filter->GetFilterState())
	{
		return VFW_E_NOT_STOPPED;
	}

	if (m_peer_pin)
	{
		return VFW_E_ALREADY_CONNECTED;
	}

	if (!pmt)
	{
		return S_OK;
	}

	if (pmt->majortype != GUID_NULL &&
		pmt->majortype != m_media_major_type)
	{
		return S_FALSE;
	}

	if (pmt->majortype == m_media_major_type &&
		!IsMediaTypeValid(pmt))
	{
		return S_FALSE;
	}

	return S_OK;
}

STDMETHODIMP SamplePin::Disconnect()
{
	if (!m_peer_pin)
	{
		return S_FALSE;
	}

	if (State_Running == m_filter->GetFilterState())
	{
		return VFW_E_NOT_STOPPED;
	}

	m_peer_pin->Release();
	m_peer_pin = NULL;
	return S_OK;
}

STDMETHODIMP SamplePin::ConnectedTo(IPin **pPin)
{
	if (!pPin)
	{
		return E_POINTER;
	}

	if (!m_peer_pin)
	{
		*pPin = NULL;
		return VFW_E_NOT_CONNECTED;
	}

	IPin *pin = m_peer_pin;
	pin->AddRef();
	*pPin = pin;

	return S_OK;
}

STDMETHODIMP SamplePin::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
	if (!m_peer_pin)
	{
		return VFW_E_NOT_CONNECTED;
	}

	return CopyMediaType(pmt, &m_media_type);
}

STDMETHODIMP SamplePin::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
	if (State_Stopped != m_filter->GetFilterState())
	{
		return VFW_E_NOT_STOPPED;
	}

	if (!pConnector || !pmt)
	{
		return E_POINTER;
	}

	if (m_peer_pin)
	{
		return VFW_E_ALREADY_CONNECTED;
	}

	if (QueryAccept(pmt) != S_OK)
	{
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	m_peer_pin = pConnector;
	m_peer_pin->AddRef();

	FreeMediaType(m_media_type);
	ZeroMemory(&m_media_type, sizeof(AM_MEDIA_TYPE));
	CopyMediaType(&m_media_type, pmt);

	return S_OK;
}


STDMETHODIMP SamplePin::QueryPinInfo(PIN_INFO *pInfo)
{
	if (!pInfo)
	{
		return E_POINTER;
	}

	pInfo->pFilter = m_filter;
	if (m_filter)
	{
		IBaseFilter *ptr = m_filter;
		ptr->AddRef();
	}

	memset(pInfo->achName, 0, sizeof(pInfo->achName));
	if (m_media_type.majortype == MEDIATYPE_Video)
	{
		memcpy(pInfo->achName, L"Sample Video Pin", sizeof(L"Sample Video Pin"));
	}
	else
	{
		memcpy(pInfo->achName, L"Sample Audio Pin", sizeof(L"Sample Audio Pin"));
	}

	pInfo->dir = PINDIR_INPUT;

	return S_OK;
}

STDMETHODIMP SamplePin::QueryDirection(PIN_DIRECTION *pPinDir)
{
	if (!pPinDir)
	{
		return E_POINTER;
	}

	*pPinDir = PINDIR_INPUT;
	return S_OK;
}

STDMETHODIMP SamplePin::QueryId(LPWSTR *Id)
{
	if (!Id)
	{
		return E_POINTER;
	}

	wchar_t *str = (wchar_t *)CoTaskMemAlloc(sizeof(L"Sample Input Pin") + 1);
	if (!str)
	{
		return E_OUTOFMEMORY;
	}

	memset(str, 0, sizeof(L"Sample Input Pin") + 1);
	//memcpy(str, L"Sample Input Pin", sizeof(L"Sample Input Pin"));
	StringCbCopyW(str, sizeof(L"Sample Input Pin") + 1, L"Sample Input Pin");
	*Id = str;

	return S_OK;
}

STDMETHODIMP SamplePin::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
	if (pmt->majortype != m_media_major_type)
	{
		return S_FALSE;
	}

	if (!IsMediaTypeValid(pmt))
	{
		return S_FALSE;
	}

	if (m_peer_pin)
	{
		FreeMediaType(m_media_type);
		ZeroMemory(&m_media_type, sizeof(AM_MEDIA_TYPE));
		CopyMediaType(&m_media_type, pmt);
	}

	return S_OK;
}

STDMETHODIMP SamplePin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
	if (!ppEnum)
	{
		return E_POINTER;
	}

	*ppEnum = new (std::nothrow) SampleEnumMediaTypes(this);
	if (!(*ppEnum))
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

STDMETHODIMP SamplePin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
	return E_NOTIMPL;
}

STDMETHODIMP SamplePin::EndOfStream()
{
	return S_OK;
}

STDMETHODIMP SamplePin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop,
	double dRate)
{
	return S_OK;
}

STDMETHODIMP SamplePin::GetAllocator(IMemAllocator **ppAllocator)
{
	return VFW_E_NO_ALLOCATOR;
}

STDMETHODIMP SamplePin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
	return S_OK;
}

STDMETHODIMP SamplePin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
	return E_NOTIMPL;
}

STDMETHODIMP SamplePin::Receive(IMediaSample *pSample)
{
	if (m_flushing)
	{
		return S_FALSE;
	}

	if (pSample)
	{
		m_filter->OnMediaSampleReceive(pSample);
	}

	return S_OK;
}

STDMETHODIMP SamplePin::ReceiveMultiple(IMediaSample **pSamples, long nSamples,
	long *nSamplesProcessed)
{
	if (m_flushing)
	{
		return S_FALSE;
	}

	for (long i = 0; i < nSamples; i++)
	{
		if (pSamples[i])
		{
			m_filter->OnMediaSampleReceive(pSamples[i]);
		}
	}

	*nSamplesProcessed = nSamples;

	return S_OK;
}

STDMETHODIMP SamplePin::ReceiveCanBlock()
{
	return S_FALSE;
}
