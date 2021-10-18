#ifndef _H_MULTIMEDIA_BOX_H_
#define _H_MULTIMEDIA_BOX_H_

#include <vector>
#include <string>
#include <map>
#include <functional>

#include <windows.h>

#include "sample_utils.h"
#include "sample_videobox.h"
#include "sample_audiobox.h"
#include "sample_media_rawdata.h"
#include "codec_utils.h"

#include "common_lockfree_queue.h"

enum class Command
{
	None,
	ActivateAll,
	ActivateVideo,
	ActivateAudio,
	DeactivateAll,
	DeactivateVideo,
	DeactivateAudio,
	Shutdown
};

class CriticalSection
{
public:
	CriticalSection()
	{
		InitializeCriticalSection(&m_mutex);
	}

	~CriticalSection()
	{
		DeleteCriticalSection(&m_mutex);
	}

	CRITICAL_SECTION * Get()
	{
		return &m_mutex;
	}

private:
	CRITICAL_SECTION m_mutex;
};

class CriticalScope
{
public:
	CriticalScope(CriticalSection &mutex_p) : m_mutex(mutex_p)
	{
		EnterCriticalSection(m_mutex.Get());
	}

	~CriticalScope()
	{
		LeaveCriticalSection(m_mutex.Get());
	}

private:
	CriticalSection &m_mutex;
};

constexpr int VIDEO_RAW_DATA_SIZE = 10;
constexpr int AUDIO_RAW_DATA_SIZE = 10;

typedef std::function<void()> VideoSampleCallback;
typedef std::function<void()> AudioSampleCallback;

class MediaBox
{
public:
	MediaBox();
	virtual ~MediaBox();

	void queue_command(Command cmd);
	void set_video_config(const std::map<std::string, std::wstring>& configs);
	void set_audio_config(const std::map<std::string, std::wstring>& configs);

	VideoRawData* begin_raw_video_data();
	void end_raw_video_data(VideoRawData* raw);

	AudioRawData* begin_raw_audio_data();
	void end_raw_audio_data(AudioRawData* raw);

	void set_video_sample_callback(VideoSampleCallback videoCallback);
	void set_audio_sample_callback(AudioSampleCallback audioCallback);

protected:
	HRESULT activate_all();
	HRESULT activate_video();
	HRESULT activate_audio();

	HRESULT deactivate_all();
	HRESULT deactivate_video();
	HRESULT deactivate_audio();

	void loop();
	inline void on_video_data(const VideoDeviceConfig &config, unsigned char *data,
		size_t size, REFERENCE_TIME startTime, REFERENCE_TIME stopTime);
	inline void on_audio_data(const AudioDeviceConfig &config, unsigned char *data,
		size_t size, REFERENCE_TIME startTime, REFERENCE_TIME stopTime);

	static DWORD WINAPI box_thread(LPVOID ptr);
private:
	BOOL m_has_error;

	//video
	VideoBox m_video;
	std::map<std::string, std::wstring> m_video_config_map;
	std::vector<VideoRawData*> m_raw_video_data_vec;
	LockFreeQueue<VideoRawData*> m_video_pool;
	LockFreeQueue<VideoRawData*> m_video_queue;

	VideoSampleCallback m_video_sample_callback;

	//audio
	AudioBox m_audio;
	std::map<std::string, std::wstring> m_audio_config_map;
	std::vector<AudioRawData*> m_raw_audio_data_vec;
	LockFreeQueue<AudioRawData*> m_audio_pool;
	LockFreeQueue<AudioRawData*> m_audio_queue;

	AudioSampleCallback m_audio_sample_callback;
	
	HANDLE m_thread_handle;
	CriticalSection m_mutex;
	std::vector<Command> m_commands;
	HANDLE m_cmd_semaphore;
};

#endif