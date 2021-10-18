#include "sample_mediadevice.h"

#include "common_logger.h"

MMDevice::MMDevice()
{
	m_initialized = FALSE;
	m_active = FALSE;

	m_graph_builder = NULL;
	m_capture_graph_builder = NULL;
	m_media_control = NULL;
}

MMDevice::~MMDevice()
{
	if (m_media_control)
	{
		m_media_control->Release();
		m_media_control = NULL;
	}

	if (m_graph_builder)
	{
		m_graph_builder->Release();
		m_graph_builder = NULL;
	}

	if (m_capture_graph_builder)
	{
		m_capture_graph_builder->Release();
		m_capture_graph_builder = NULL;
	}
}

HRESULT MMDevice::disconnect_filters()
{
	IEnumFilters* filterEnum = NULL;
	HRESULT hr;

	if (!m_graph_builder)
	{
		return E_FAIL;
	}

	hr = m_graph_builder->EnumFilters(&filterEnum);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	IBaseFilter* filter = NULL;
	while (filterEnum->Next(1, &filter, NULL) == S_OK)
	{
		m_graph_builder->RemoveFilter(filter);
		filterEnum->Reset();

		filter->Release();
	}

	filterEnum->Release();
	return S_OK;
}

HRESULT MMDevice::create_graph()
{
	if (m_initialized)
	{
		return S_OK;
	}

	m_graph_builder = NULL;
	m_capture_graph_builder = NULL;
	m_media_control = NULL;

	HRESULT hr;
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IFilterGraph, (LPVOID*)&m_graph_builder);
	if (FAILED(hr))
	{
		goto errorExit;
	}

	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
		IID_ICaptureGraphBuilder2, (LPVOID*)&m_capture_graph_builder);
	if (FAILED(hr))
	{
		goto errorExit;
	}

	hr = m_capture_graph_builder->SetFiltergraph(m_graph_builder);
	if (FAILED(hr))
	{
		goto errorExit;
	}

	hr = m_graph_builder->QueryInterface(IID_IMediaControl, (LPVOID*)&m_media_control);
	if (FAILED(hr))
	{
		goto errorExit;
	}

	m_initialized = TRUE;
	return S_OK;

errorExit:
	if (m_graph_builder)
	{
		m_graph_builder->Release();
		m_graph_builder = NULL;
	}

	if (m_capture_graph_builder)
	{
		m_capture_graph_builder->Release();
		m_capture_graph_builder = NULL;
	}

	if (m_media_control)
	{
		m_media_control->Release();
		m_media_control = NULL;
	}

	return E_FAIL;
}

HRESULT MMDevice::get_pin_media_type(IPin *pin, AM_MEDIA_TYPE &mt, const GUID& formatType)
{
	if (!pin)
	{
		return E_POINTER;
	}

	IEnumMediaTypes* enumMediaTypes;
	if (SUCCEEDED(pin->EnumMediaTypes(&enumMediaTypes)))
	{
		AM_MEDIA_TYPE* mediaType = NULL;
		ULONG count = 0;

		while (enumMediaTypes->Next(1, &mediaType, &count) == S_OK)
		{
			if (mediaType->formattype == formatType)
			{
				FreeMediaType(mt);
				ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
				CopyMediaType(&mt, mediaType);

				FreeMediaType(*mediaType);
				CoTaskMemFree(mediaType);
				enumMediaTypes->Release();
				return S_OK;
			}
		}

		enumMediaTypes->Release();
	}

	return E_FAIL;
}

HRESULT MMDevice::connect_pins(const GUID &type, const GUID &category,
	IBaseFilter *filter, IBaseFilter *sample)
{
	if (!m_initialized || m_active)
	{
		return E_FAIL;
	}

	IPin* filterPin = NULL;
	IPin* samplePin = NULL;

	HRESULT hr = GetPinOfFilter(filter, type, category,
		PINDIR_OUTPUT, &filterPin);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	hr = GetPinOfFilterByDirection(sample, PINDIR_INPUT, &samplePin);
	if (FAILED(hr))
	{
		filterPin->Release();
		return E_FAIL;
	}

	hr = m_graph_builder->ConnectDirect(filterPin, samplePin, NULL);
	if (hr == S_OK)
	{
		filterPin->Release();
		samplePin->Release();
		return S_OK;
	}
	else
	{
		filterPin->Release();
		samplePin->Release();

		return E_FAIL;
	}
}

HRESULT MMDevice::start()
{
	HRESULT hr = m_media_control->Run();
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	m_active = TRUE;

	return S_OK;
}

HRESULT MMDevice::stop()
{
	if (m_active)
	{
		m_media_control->Stop();
		m_active = FALSE;
	}

	return S_OK;
}