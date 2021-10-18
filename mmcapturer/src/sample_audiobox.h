#ifndef _H_MULTIMEDIA_AUDIO_BOX_H_
#define _H_MULTIMEDIA_AUDIO_BOX_H_

#include <windows.h>
#include "sample_utils.h"
#include "sample_audiodevice.h"

class AudioBox
{
public:
	AudioBox();
	virtual ~AudioBox();

	HRESULT reset_graph();
	HRESULT shutdown_graph();
	HRESULT set_audio_config(AudioDeviceConfig& audioConfig);
	HRESULT connect_filters();
	HRESULT start();
	HRESULT stop();

private:
	AudioDevice* m_audio;
};

#endif
