#include "sample_videodevice.h"

#include <vector>
#include "common_logger.h"

VideoDevice::VideoDevice()
{
	m_video_sample_filter = NULL;
	m_video_filter = NULL;
	m_update_media_type_by_sample = FALSE;

	ZeroMemory(&m_video_media_type, sizeof(AM_MEDIA_TYPE));
}

VideoDevice::~VideoDevice()
{
	stop();

	disconnect_filters();

	if (m_video_sample_filter)
	{
		m_video_sample_filter->Release();
	}

	if (m_video_filter)
	{
		m_video_filter->Release();
	}

	FreeMediaType(m_video_media_type);
}

HRESULT VideoDevice::set_video_config(VideoDeviceConfig& videoConfig)
{
	if (!m_initialized || m_active)
	{
		return E_FAIL;
	}

	FreeMediaType(m_video_media_type);
	ZeroMemory(&m_video_media_type, sizeof(AM_MEDIA_TYPE));

	if (m_video_sample_filter)
	{
		m_graph_builder->RemoveFilter(m_video_sample_filter);
		m_video_sample_filter->Release();
		m_video_sample_filter = NULL;
	}

	if (m_video_filter)
	{
		m_graph_builder->RemoveFilter(m_video_filter);
		m_video_filter->Release();
		m_video_filter = NULL;
	}

	if (videoConfig.namew.empty() && videoConfig.pathw.empty())
	{
		LOG_ERROR("The device name and path are all empty.");
		return E_FAIL;
	}

	IBaseFilter* filter = NULL;
	HRESULT hr;
	hr = GetDeviceFilter(CLSID_VideoInputDeviceCategory,
		videoConfig.namew.c_str(),
		videoConfig.pathw.c_str(), &filter);
	if (FAILED(hr))
	{
		LOG_ERROR("Get Device Filter failed");
		return E_FAIL;
	}

	m_current_video_config = videoConfig;

	hr = setup_video_filters(filter, m_current_video_config);
	if (FAILED(hr))
	{
		LOG_ERROR("Setup Video Filters failed");

		filter->Release();
		return E_FAIL;
	}

	videoConfig = m_current_video_config;

	filter->Release();
	return S_OK;
}

BOOL VideoDevice::get_video_config(VideoDeviceConfig& out)
{
	if (!m_video_sample_filter)
	{
		return FALSE;
	}
	out = m_current_video_config;
	return TRUE;
}

HRESULT VideoDevice::on_sample_data(IMediaSample* sample)
{
	if (!m_update_media_type_by_sample)
	{
		AM_MEDIA_TYPE* mt = NULL;

		HRESULT hr = sample->GetMediaType(&mt);
		if (hr == S_OK)
		{
			FreeMediaType(m_video_media_type);
			CopyMediaType(&m_video_media_type, mt);

			update_video_config();

			FreeMediaType(*mt);
			CoTaskMemFree(mt);

			m_update_media_type_by_sample = TRUE;
		}
	}

	long size = sample->GetActualDataLength();
	if (size == 0)
	{
		return E_FAIL;
	}

	BYTE *ptr;
	if (FAILED(sample->GetPointer(&ptr)))
	{
		return E_FAIL;
	}

	REFERENCE_TIME startTime, stopTime;
	bool hasTime = SUCCEEDED(sample->GetTime(&startTime, &stopTime));
	bool isEncodedData = (m_current_video_config.format == VideoFormat::MJPEG) ||
		(m_current_video_config.format == VideoFormat::H264);
	if (isEncodedData)
	{
		if (hasTime) 
		{
			m_current_video_config.callback(m_current_video_config, m_video_encode_data.bytes.data(),
				m_video_encode_data.bytes.size(), m_video_encode_data.lastStartTime,
				m_video_encode_data.lastStopTime);

			m_video_encode_data.bytes.resize(0);
			m_video_encode_data.lastStartTime = startTime;
			m_video_encode_data.lastStopTime = stopTime;
		}

		m_video_encode_data.bytes.insert(m_video_encode_data.bytes.end(), (unsigned char *)ptr,
			(unsigned char *)ptr + size);
	}
	else if (hasTime)
	{
		m_current_video_config.callback(m_current_video_config, ptr, size, startTime, stopTime);
	}

	return S_OK;
}

void VideoDevice::update_video_config()
{
	VIDEOINFOHEADER *vih = (VIDEOINFOHEADER *)m_video_media_type.pbFormat;
	const BITMAPINFOHEADER *bmih = GetBitmapInfoHeader(m_video_media_type);

	if (bmih)
	{
		m_current_video_config.cx = bmih->biWidth;
		m_current_video_config.cyabs = labs(bmih->biHeight);
		m_current_video_config.cyflip = bmih->biHeight < 0;
		m_current_video_config.frameInterval = vih->AvgTimePerFrame;

		bool same = m_current_video_config.internalFormat == m_current_video_config.format;
		GetMediaTypeVideoFormat(m_video_media_type, m_current_video_config.internalFormat);

		if (same)
		{
			m_current_video_config.format = m_current_video_config.internalFormat;
		}
	}
}

HRESULT VideoDevice::setup_video_filters(IBaseFilter* filter, VideoDeviceConfig& videoConfig)
{
	IPin* pin = NULL;
	IAMStreamConfig* streamConfig = NULL;
	HRESULT hr;

	hr = GetPinOfFilter(filter, MEDIATYPE_Video, PIN_CATEGORY_CAPTURE, PINDIR_OUTPUT, &pin);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	pin->QueryInterface(IID_IAMStreamConfig, (void **)&streamConfig);
	if (!streamConfig)
	{
		pin->Release();
		return E_FAIL;
	}

	if (videoConfig.useDefaultVideoConfig)
	{
		AM_MEDIA_TYPE* defaultMediaType = NULL;
		hr = streamConfig->GetFormat(&defaultMediaType);
		if (FAILED(hr))
		{
			get_pin_media_type(pin, m_video_media_type, FORMAT_VideoInfo);
		}
		else
		{
			FreeMediaType(m_video_media_type);
			CopyMediaType(&m_video_media_type, defaultMediaType);

			FreeMediaType(*defaultMediaType);
			CoTaskMemFree(defaultMediaType);
		}

		update_video_config();
		videoConfig.format = videoConfig.internalFormat = VideoFormat::Any;
	}

	//get the closes video media type
	hr = GetClosestVideoMediaType(streamConfig, videoConfig, m_video_media_type);
	if (FAILED(hr))
	{
		pin->Release();
		streamConfig->Release();
		return E_FAIL;
	}

	hr = streamConfig->SetFormat(&m_video_media_type);
	if (FAILED(hr) && hr != E_NOTIMPL)
	{
		pin->Release();
		streamConfig->Release();
		return E_FAIL;
	}

	update_video_config();

	GUID subType;
	if (videoConfig.format == VideoFormat::XRGB32)
	{
		subType = MEDIASUBTYPE_RGB32;
	}
	else if (videoConfig.format == VideoFormat::ARGB32)
	{
		subType = MEDIASUBTYPE_ARGB32;
	}
	else if (videoConfig.format == VideoFormat::RGB24)
	{
		subType = MEDIASUBTYPE_RGB24;
	}
	else if (videoConfig.format == VideoFormat::RGB555)
	{
		subType = MEDIASUBTYPE_RGB555;
	}
	else if (videoConfig.format == VideoFormat::RGB565)
	{
		subType = MEDIASUBTYPE_RGB565;
	}
	else if (videoConfig.format == VideoFormat::IYUV)
	{
		subType = MEDIASUBTYPE_IYUV;
	}
	else if (videoConfig.format == VideoFormat::NV12)
	{
		subType = MEDIASUBTYPE_NV12;
	}
	else if (videoConfig.format == VideoFormat::YV12)
	{
		subType = MEDIASUBTYPE_YV12;
	}
	else if (videoConfig.format == VideoFormat::AYUV)
	{
		subType = MEDIASUBTYPE_AYUV;
	}
	else if (videoConfig.format == VideoFormat::YVYU)
	{
		subType = MEDIASUBTYPE_YVYU;
	}
	else if (videoConfig.format == VideoFormat::YUY2)
	{
		subType = MEDIASUBTYPE_YUY2;
	}
	else if (videoConfig.format == VideoFormat::UYVY)
	{
		subType = MEDIASUBTYPE_UYVY;
	}
	else
	{
		subType = m_video_media_type.subtype;
	}

	m_video_sample_filter = new SampleFilter(this, m_video_media_type.majortype, subType);
	m_video_filter = filter;
	m_video_filter->AddRef();

	m_graph_builder->AddFilter(m_video_sample_filter, L"Video Sample Filter");
	m_graph_builder->AddFilter(m_video_filter, L"Video Filter");

	pin->Release();
	streamConfig->Release();
	return S_OK;
}

HRESULT VideoDevice::connect_filters()
{
	if (!m_initialized || m_active)
	{
		return E_FAIL;
	}

	if (!m_video_filter || !m_video_sample_filter)
	{
		return E_FAIL;
	}

	HRESULT hr = connect_pins(MEDIATYPE_Video, PIN_CATEGORY_CAPTURE,
		m_video_filter, m_video_sample_filter);
	if (FAILED(hr))
	{
		hr = m_capture_graph_builder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, 
			m_video_filter, NULL, m_video_sample_filter);
		if (FAILED(hr))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}