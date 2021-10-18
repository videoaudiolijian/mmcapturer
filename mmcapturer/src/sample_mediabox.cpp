#include "sample_mediabox.h"

#include <functional>

#include "common_logger.h"

DWORD WINAPI MediaBox::box_thread(LPVOID ptr)
{
	MediaBox *box = (MediaBox *)ptr;

	HRESULT hr = CoInitialize(NULL);
	if (hr == S_OK)
	{
		LOG_INFO("The COM library was initialized successfully on this thread.");

		box->loop();
		CoUninitialize();
	}
	else if (hr == S_FALSE)
	{
		LOG_WARNING("The COM library is already initialized on this thread.");

		box->loop();
		CoUninitialize();
	}
	else
	{
		LOG_ERROR("The COM library was initialized failed on this thread.");
	}
	return 0;
}

MediaBox::MediaBox()
	:m_video_pool(VIDEO_RAW_DATA_SIZE * 2), m_video_queue(VIDEO_RAW_DATA_SIZE * 2),
	m_audio_pool(AUDIO_RAW_DATA_SIZE * 2), m_audio_queue(AUDIO_RAW_DATA_SIZE * 2)
{
	m_has_error = FALSE;

	m_cmd_semaphore = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	if (!m_cmd_semaphore)
	{
		m_has_error = TRUE;

		LOG_ERROR("CreateSemaphore() failed.");
		return;
	}

	m_thread_handle = CreateThread(NULL, NULL, MediaBox::box_thread, this, NULL, NULL);
	if (!m_thread_handle)
	{
		m_has_error = TRUE;

		LOG_ERROR("The CreateThread() failed.");
		return;
	}

	for (int i = 0; i < VIDEO_RAW_DATA_SIZE; i++)
	{
		VideoRawData* raw = new (std::nothrow) VideoRawData();
		if (!raw)
		{
			LOG_ERROR("Out of memory.");
			continue;
		}
		m_raw_video_data_vec.push_back(raw);
		m_video_pool.push(raw);
	}

	for (int i = 0; i < AUDIO_RAW_DATA_SIZE; i++)
	{
		AudioRawData* raw = new (std::nothrow) AudioRawData();
		if (!raw)
		{
			LOG_ERROR("Out of memory.");
			continue;
		}
		m_raw_audio_data_vec.push_back(raw);
		m_audio_pool.push(raw);
	}

	LOG_INFO("The MediaBox was created successfully.");
}

MediaBox::~MediaBox()
{
	if (m_has_error)
	{
		return;
	}

	{
		CriticalScope scope(m_mutex);
		m_commands.resize(1);
		m_commands[0] = Command::Shutdown;
	}

	ReleaseSemaphore(m_cmd_semaphore, 1, NULL);
	WaitForSingleObject(m_thread_handle, INFINITE);

	std::vector<VideoRawData*>::const_iterator it = m_raw_video_data_vec.begin();
	for (; it < m_raw_video_data_vec.end(); it++)
	{
		VideoRawData* ptr = *it;
		delete ptr;
	}

	std::vector<AudioRawData*>::const_iterator it1 = m_raw_audio_data_vec.begin();
	for (; it1 < m_raw_audio_data_vec.end(); it1++)
	{
		AudioRawData* ptr = *it1;
		delete ptr;
	}
}

void MediaBox::set_video_config(const std::map<std::string, std::wstring>& configs)
{
	m_video_config_map.clear();
	for (auto it = configs.begin(); it != configs.end(); it++)
	{
		m_video_config_map.insert(std::make_pair(it->first, it->second));
	}
}

void MediaBox::set_audio_config(const std::map<std::string, std::wstring>& configs)
{
	m_audio_config_map.clear();
	for (auto it = configs.begin(); it != configs.end(); it++)
	{
		m_audio_config_map.insert(std::make_pair(it->first, it->second));
	}
}

void MediaBox::queue_command(Command cmd)
{
	if (m_has_error)
	{
		return;
	}

	CriticalScope scope(m_mutex);
	m_commands.push_back(cmd);
	ReleaseSemaphore(m_cmd_semaphore, 1, NULL);
}

void MediaBox::loop()
{
	while (true)
	{
		DWORD ret = WaitForSingleObject(m_cmd_semaphore, INFINITE);
		if (ret != WAIT_OBJECT_0)
		{
			break;
		}

		Command cmd = Command::None;
		{
			CriticalScope scope(m_mutex);
			if (m_commands.size())
			{
				cmd = m_commands.front();
				m_commands.erase(m_commands.begin());
			}
		}

		switch (cmd)
		{
		case Command::ActivateAll:
			activate_all();
			break;

		case Command::ActivateVideo:
			activate_video();
			break;

		case Command::ActivateAudio:
			activate_audio();
			break;

		case Command::DeactivateAll:
			deactivate_all();
			break;

		case Command::DeactivateVideo:
			deactivate_video();
			break;

		case Command::DeactivateAudio:
			deactivate_audio();
			break;

		case Command::Shutdown:
			m_video.shutdown_graph();
			m_audio.shutdown_graph();
			return;

		case Command::None:
			break;
		}
	}
}

void MediaBox::set_video_sample_callback(VideoSampleCallback videoCallback)
{
	this->m_video_sample_callback = videoCallback;
}

void MediaBox::set_audio_sample_callback(AudioSampleCallback audioCallback)
{
	this->m_audio_sample_callback = audioCallback;
}

void MediaBox::on_video_data(const VideoDeviceConfig &config, unsigned char *data,
	size_t size, REFERENCE_TIME startTime, REFERENCE_TIME stopTime)
{
	if (size == 0)
	{
		return;
	}

	if (size > RAW_VIDEO_DATA_SIZE)
	{
		LOG_ERROR("The video data is too big");
		return;
	}

	VideoRawData* raw;
	if (m_video_pool.pop(raw))
	{
		raw->format = config.format;
		raw->startTime = startTime;
		raw->stopTime = stopTime;
		raw->used = size;
		raw->width = config.cx;
		raw->height = config.cyabs;
		memcpy(raw->data, data, size);

		if (m_video_queue.push(raw))
		{
			m_video_sample_callback();
		}
		else
		{
			m_video_pool.push(raw);
		}
	}
	else
	{
		LOG_ERROR("The video pool is empty");
	}
}

void MediaBox::on_audio_data(const AudioDeviceConfig &config, unsigned char *data,
	size_t size, REFERENCE_TIME startTime, REFERENCE_TIME stopTime)
{
	if (size == 0)
	{
		return;
	}

	if (size > RAW_AUDIO_DATA_SIZE)
	{
		LOG_ERROR("The audio data is too big");
		return;
	}

	AudioRawData* raw;
	if (m_audio_pool.pop(raw))
	{
		raw->format = config.format;
		raw->startTime = startTime;
		raw->stopTime = stopTime;
		raw->used = size;
		raw->sampleRate = config.sampleRate;
		raw->channels = config.channels;
		memcpy(raw->data, data, size);

		if (m_audio_queue.push(raw))
		{
			m_audio_sample_callback();
		}
		else
		{
			m_audio_pool.push(raw);
		}
	}
	else
	{
		LOG_ERROR("The audio pool is empty");
	}
}

VideoRawData* MediaBox::begin_raw_video_data()
{
	VideoRawData* raw;
	if (m_video_queue.pop(raw))
	{
		return raw;
	}
	else 
	{
		return nullptr;
	}
}

void MediaBox::end_raw_video_data(VideoRawData* raw)
{
	if (!m_video_pool.push(raw))
	{
		LOG_ERROR("the video pool is full. fatal error");
	}
}

AudioRawData* MediaBox::begin_raw_audio_data()
{
	AudioRawData* raw;
	if (m_audio_queue.pop(raw))
	{
		return raw;
	}
	else
	{
		return nullptr;
	}
}

void MediaBox::end_raw_audio_data(AudioRawData* raw)
{
	if (!m_audio_pool.push(raw))
	{
		LOG_ERROR("the audio pool is full. fatal error");
	}
}

HRESULT MediaBox::activate_all()
{
	//start video
	HRESULT hr = activate_video();
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	hr = activate_audio();
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT MediaBox::activate_video()
{
	//start video
	std::wstring name = m_video_config_map["name"];
	std::wstring path = m_video_config_map["path"];

	HRESULT hr = m_video.reset_graph();
	if (FAILED(hr))
	{
		LOG_ERROR("Video Device Reset Graph failed.");
		return E_FAIL;
	}

	VideoDeviceConfig videoConfig;
	videoConfig.useDefaultVideoConfig = true;
	videoConfig.namew = name;
	videoConfig.pathw = path;
	videoConfig.callback = std::bind(&MediaBox::on_video_data, this,
		std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3, std::placeholders::_4,
		std::placeholders::_5);

	hr = m_video.set_video_config(videoConfig);
	if (FAILED(hr))
	{
		LOG_ERROR("Video Device Set Video Config failed.");
		return E_FAIL;
	}

	hr = m_video.connect_filters();
	if (FAILED(hr))
	{
		LOG_ERROR("Video Device Connect Filters failed.");
		return E_FAIL;
	}

	hr = m_video.start();
	if (FAILED(hr))
	{
		LOG_ERROR("Video Device Start failed.");
		return E_FAIL;
	}

	LOG_INFO("Media Box Activate Video successfully.");
	return S_OK;
}

HRESULT MediaBox::activate_audio()
{
	//start audio
	std::wstring aname = m_audio_config_map["name"];
	std::wstring apath = m_audio_config_map["path"];

	HRESULT hr = m_audio.reset_graph();
	if (FAILED(hr))
	{
		LOG_ERROR("Audio Device Reset Graph failed.");
		return E_FAIL;
	}

	AudioDeviceConfig audioConfig;
	audioConfig.useDefaultAudioConfig = true;
	audioConfig.namew = aname;
	audioConfig.pathw = apath;
	audioConfig.callback = std::bind(&MediaBox::on_audio_data, this,
		std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3, std::placeholders::_4,
		std::placeholders::_5);

	hr = m_audio.set_audio_config(audioConfig);
	if (FAILED(hr))
	{
		LOG_ERROR("Audio Device Set Audio Config failed.");
		return E_FAIL;
	}

	LOG_DEBUG("audio channels:%d, format:%d, sampleRate:%d", audioConfig.channels, audioConfig.format, audioConfig.sampleRate);

	hr = m_audio.connect_filters();
	if (FAILED(hr))
	{
		LOG_ERROR("Audio Device Connect Filters failed.");
		return E_FAIL;
	}

	hr = m_audio.start();
	if (FAILED(hr))
	{
		LOG_ERROR("Audio Device Start failed.");
		return E_FAIL;
	}

	LOG_INFO("Media Box Activate Audio Successfully.");
	return S_OK;
}

HRESULT MediaBox::deactivate_all()
{
	m_video.reset_graph();
	{
		VideoRawData* raw;
		while (m_video_queue.pop(raw))
		{
			if (!m_video_pool.push(raw))
			{
				LOG_ERROR("Error occurs, memory leaks");
			}
		}
	}

	m_audio.reset_graph();
	{
		AudioRawData* raw;
		while (m_audio_queue.pop(raw))
		{
			if (!m_audio_pool.push(raw))
			{
				LOG_ERROR("Error occurs, memory leaks");
			}
		}
	}
	
	return S_OK;
}

HRESULT MediaBox::deactivate_video()
{
	m_video.reset_graph();
	{
		VideoRawData* raw;
		while (m_video_queue.pop(raw))
		{
			if (!m_video_pool.push(raw))
			{
				LOG_ERROR("Error occurs, memory leaks");
			}
		}
	}

	return S_OK;
}

HRESULT MediaBox::deactivate_audio()
{
	m_audio.reset_graph();
	{
		AudioRawData* raw;
		while (m_audio_queue.pop(raw))
		{
			if (!m_audio_pool.push(raw))
			{
				LOG_ERROR("Error occurs, memory leaks");
			}
		}
	}

	return S_OK;
}