#ifndef _H_MULTIMEDIA_VIDEO_DEVICE_H_
#define _H_MULTIMEDIA_VIDEO_DEVICE_H_

#include "sample_mediadevice.h"
#include "sample_filter.h"
#include "common_logger.h"

class VideoDevice : public MMDevice
{
public:
	VideoDevice();
	virtual ~VideoDevice();

	HRESULT set_video_config(VideoDeviceConfig& videoConfig);
	BOOL get_video_config(VideoDeviceConfig& out);

	virtual HRESULT connect_filters();
	virtual HRESULT on_sample_data(IMediaSample* sample);

protected:
	void update_video_config();
	HRESULT setup_video_filters(IBaseFilter* filter, VideoDeviceConfig& videoConfig);

protected:
	SampleFilter* m_video_sample_filter;
	IBaseFilter* m_video_filter;
	VideoDeviceConfig m_current_video_config;
	AM_MEDIA_TYPE m_video_media_type;
	BOOL m_update_media_type_by_sample;

	EncodeData m_video_encode_data;
};

#endif