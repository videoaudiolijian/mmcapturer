#ifndef _H_MULTIMEDIA_VIDEO_BOX_H_
#define _H_MULTIMEDIA_VIDEO_BOX_H_

#include <windows.h>
#include "sample_utils.h"
#include "sample_videodevice.h"

class VideoBox
{
public:
	VideoBox();
	virtual ~VideoBox();

	HRESULT reset_graph();
	HRESULT shutdown_graph();
	HRESULT set_video_config(VideoDeviceConfig& videoConfig);
	HRESULT connect_filters();
	HRESULT start();
	HRESULT stop();

private:
	VideoDevice* m_video;
};

#endif
