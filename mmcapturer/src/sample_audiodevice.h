#ifndef _H_MULTIMEDIA_AUDIO_DEVICE_H_
#define _H_MULTIMEDIA_AUDIO_DEVICE_H_

#include "sample_mediadevice.h"
#include "sample_filter.h"
#include "common_logger.h"

class AudioDevice : public MMDevice
{
public:
	AudioDevice();
	virtual ~AudioDevice();

	HRESULT set_audio_config(AudioDeviceConfig& audioConfig);
	BOOL get_audio_config(AudioDeviceConfig& out);

	virtual HRESULT connect_filters();
	virtual HRESULT on_sample_data(IMediaSample* sample);

protected:
	void update_audio_config();
	HRESULT setup_audio_filters(IBaseFilter* filter, AudioDeviceConfig& audioConfig);

protected:
	SampleFilter* m_audio_sample_filter;
	IBaseFilter* m_audio_filter;
	AudioDeviceConfig m_current_audio_config;
	AM_MEDIA_TYPE m_audio_media_type;
	BOOL m_update_media_type_by_sample;

	EncodeData m_audio_encode_data;
};

#endif