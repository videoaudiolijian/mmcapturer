#include "sample_audiodevice.h"

#include <vector>
#include <mmreg.h>
#include "common_logger.h"

AudioDevice::AudioDevice()
{
	m_audio_sample_filter = NULL;
	m_audio_filter = NULL;
	m_update_media_type_by_sample = FALSE;

	ZeroMemory(&m_audio_media_type, sizeof(AM_MEDIA_TYPE));
}

AudioDevice::~AudioDevice()
{
	stop();

	disconnect_filters();

	if (m_audio_sample_filter)
	{
		m_audio_sample_filter->Release();
	}

	if (m_audio_filter)
	{
		m_audio_filter->Release();
	}

	FreeMediaType(m_audio_media_type);
}

HRESULT AudioDevice::set_audio_config(AudioDeviceConfig& audioConfig)
{
	if (!m_initialized || m_active)
	{
		return E_FAIL;
	}

	FreeMediaType(m_audio_media_type);
	ZeroMemory(&m_audio_media_type, sizeof(AM_MEDIA_TYPE));

	if (m_audio_sample_filter)
	{
		m_graph_builder->RemoveFilter(m_audio_sample_filter);
		m_audio_sample_filter->Release();
		m_audio_sample_filter = NULL;
	}

	if (m_audio_filter)
	{
		m_graph_builder->RemoveFilter(m_audio_filter);
		m_audio_filter->Release();
		m_audio_filter = NULL;
	}

	if (audioConfig.namew.empty() && audioConfig.pathw.empty())
	{
		LOG_ERROR("The device name and path are all empty.");
		return E_FAIL;
	}

	IBaseFilter* filter = NULL;
	HRESULT hr;
	hr = GetDeviceFilter(CLSID_AudioInputDeviceCategory,
		audioConfig.namew.c_str(),
		audioConfig.pathw.c_str(), &filter);
	if (FAILED(hr))
	{
		LOG_ERROR("Get Device Filter failed");
		return E_FAIL;
	}

	m_current_audio_config = audioConfig;

	hr = setup_audio_filters(filter, m_current_audio_config);
	if (FAILED(hr))
	{
		LOG_ERROR("Setup Audio Filters failed");

		filter->Release();
		return E_FAIL;
	}

	audioConfig = m_current_audio_config;

	filter->Release();
	return S_OK;
}

BOOL AudioDevice::get_audio_config(AudioDeviceConfig& out)
{
	if (!m_audio_sample_filter)
	{
		return FALSE;
	}
	out = m_current_audio_config;
	return TRUE;
}

void AudioDevice::update_audio_config()
{
	WAVEFORMATEX *wfex =
		reinterpret_cast<WAVEFORMATEX *>(m_audio_media_type.pbFormat);

	m_current_audio_config.sampleRate = wfex->nSamplesPerSec;
	m_current_audio_config.channels = wfex->nChannels;

	if (wfex->wFormatTag == WAVE_FORMAT_RAW_AAC1)
	{
		m_current_audio_config.format = AudioFormat::AAC;
	}
	else if (wfex->wFormatTag == WAVE_FORMAT_DVM)
	{
		m_current_audio_config.format = AudioFormat::AC3;
	}
	else if (wfex->wFormatTag == WAVE_FORMAT_MPEG)
	{
		m_current_audio_config.format = AudioFormat::MPGA;
	}
	else if (wfex->wBitsPerSample == 16)
	{
		m_current_audio_config.format = AudioFormat::Wave16bit;
	}
	else if (wfex->wBitsPerSample == 32)
	{
		m_current_audio_config.format = AudioFormat::WaveFloat;
	}
	else
	{
		m_current_audio_config.format = AudioFormat::Unknown;
	}
}

HRESULT AudioDevice::on_sample_data(IMediaSample* sample)
{
	if (!m_update_media_type_by_sample)
	{
		AM_MEDIA_TYPE* mt = NULL;

		HRESULT hr = sample->GetMediaType(&mt);
		if (hr == S_OK)
		{
			FreeMediaType(m_audio_media_type);
			CopyMediaType(&m_audio_media_type, mt);

			update_audio_config();

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
	bool isEncodedData = (m_current_audio_config.format == AudioFormat::AAC) ||
		(m_current_audio_config.format == AudioFormat::AC3) ||
		(m_current_audio_config.format == AudioFormat::MPGA);
	if (isEncodedData)
	{
		if (hasTime)
		{
			m_current_audio_config.callback(m_current_audio_config, m_audio_encode_data.bytes.data(),
				m_audio_encode_data.bytes.size(), m_audio_encode_data.lastStartTime,
				m_audio_encode_data.lastStopTime);

			m_audio_encode_data.bytes.resize(0);
			m_audio_encode_data.lastStartTime = startTime;
			m_audio_encode_data.lastStopTime = stopTime;
		}

		m_audio_encode_data.bytes.insert(m_audio_encode_data.bytes.end(), (unsigned char *)ptr,
			(unsigned char *)ptr + size);
	}
	else if (hasTime)
	{
		m_current_audio_config.callback(m_current_audio_config, ptr, size, startTime, stopTime);
	}

	return S_OK;
}

HRESULT AudioDevice::setup_audio_filters(IBaseFilter* filter, AudioDeviceConfig& audioConfig)
{
	IPin* pin = NULL;
	IAMStreamConfig* streamConfig = NULL;
	HRESULT hr;

	hr = GetPinOfFilter(filter, MEDIATYPE_Audio, PIN_CATEGORY_CAPTURE, PINDIR_OUTPUT, &pin);
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

	if (audioConfig.useDefaultAudioConfig)
	{
		AM_MEDIA_TYPE* defaultMediaType = NULL;
		hr = streamConfig->GetFormat(&defaultMediaType);
		if (FAILED(hr))
		{
			get_pin_media_type(pin, m_audio_media_type, FORMAT_WaveFormatEx);
		}
		else
		{
			WAVEFORMATEX *wfex = (WAVEFORMATEX *)defaultMediaType->pbFormat;
			if (wfex->wBitsPerSample == 24)
			{
				audioConfig.sampleRate = wfex->nSamplesPerSec;
				audioConfig.channels = wfex->nChannels;
				audioConfig.format = AudioFormat::Wave16bit;
				audioConfig.useDefaultAudioConfig = false;
			}
			else
			{
				FreeMediaType(m_audio_media_type);
				CopyMediaType(&m_audio_media_type, defaultMediaType);

				FreeMediaType(*defaultMediaType);
				CoTaskMemFree(defaultMediaType);
			}
		}
	}

	if (!audioConfig.useDefaultAudioConfig)
	{
		//get the closes audio media type
		hr = GetClosestAudioMediaType(streamConfig, audioConfig, m_audio_media_type);
		if (FAILED(hr))
		{
			pin->Release();
			streamConfig->Release();
			return E_FAIL;
		}
	}

	hr = streamConfig->SetFormat(&m_audio_media_type);
	if (FAILED(hr) && hr != E_NOTIMPL)
	{
		pin->Release();
		streamConfig->Release();
		return E_FAIL;
	}

	update_audio_config();

	m_audio_sample_filter = new SampleFilter(this, m_audio_media_type.majortype, m_audio_media_type.subtype);
	m_audio_filter = filter;
	m_audio_filter->AddRef();

	m_graph_builder->AddFilter(m_audio_sample_filter, L"Audio Sample Filter");
	m_graph_builder->AddFilter(m_audio_filter, L"Audio Filter");

	pin->Release();
	streamConfig->Release();
	return S_OK;
}

HRESULT AudioDevice::connect_filters()
{
	if (!m_initialized || m_active)
	{
		return E_FAIL;
	}

	if (!m_audio_filter || !m_audio_sample_filter)
	{
		return E_FAIL;
	}

	HRESULT hr = connect_pins(MEDIATYPE_Audio, PIN_CATEGORY_CAPTURE,
		m_audio_filter, m_audio_sample_filter);
	if (FAILED(hr))
	{
		hr = m_capture_graph_builder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
			m_audio_filter, NULL, m_audio_sample_filter);
		if (FAILED(hr))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}