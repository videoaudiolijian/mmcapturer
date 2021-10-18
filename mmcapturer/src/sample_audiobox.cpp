#include "sample_audiobox.h"

#include <new>
#include "common_logger.h"

AudioBox::AudioBox()
{
	m_audio = new (std::nothrow) AudioDevice();
}

AudioBox::~AudioBox()
{
	if (m_audio)
	{
		delete m_audio;
	}
}
HRESULT AudioBox::reset_graph()
{
	if (m_audio)
	{
		delete m_audio;
	}

	m_audio = new (std::nothrow) AudioDevice();
	if (m_audio)
	{
		return m_audio->create_graph();
	}
	else
	{
		return E_OUTOFMEMORY;
	}
}

HRESULT AudioBox::shutdown_graph()
{
	if (m_audio)
	{
		delete m_audio;
	}

	m_audio = new (std::nothrow) AudioDevice();

	return S_OK;
}

HRESULT AudioBox::set_audio_config(AudioDeviceConfig& audioConfig)
{
	if (!m_audio)
	{
		return E_FAIL;
	}

	return m_audio->set_audio_config(audioConfig);
}

HRESULT AudioBox::connect_filters()
{
	if (!m_audio)
	{
		return E_FAIL;
	}
	return m_audio->connect_filters();
}

HRESULT AudioBox::start()
{
	if (!m_audio)
	{
		return E_FAIL;
	}
	return m_audio->start();
}

HRESULT AudioBox::stop()
{
	if (!m_audio)
	{
		return E_FAIL;
	}
	return m_audio->stop();
}