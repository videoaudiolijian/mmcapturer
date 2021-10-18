#include "sample_videobox.h"

#include <new>
#include "common_logger.h"

VideoBox::VideoBox()
{
	m_video = new (std::nothrow) VideoDevice();
}

VideoBox::~VideoBox()
{
	if (m_video)
	{
		delete m_video;
	}
}
HRESULT VideoBox::reset_graph()
{
	if (m_video)
	{
		delete m_video;
	}

	m_video = new (std::nothrow) VideoDevice();
	if (m_video)
	{
		return m_video->create_graph();
	}
	else
	{
		return E_OUTOFMEMORY;
	}

}

HRESULT VideoBox::shutdown_graph()
{
	if (m_video)
	{
		delete m_video;
	}

	m_video = new (std::nothrow) VideoDevice();

	return S_OK;
}

HRESULT VideoBox::set_video_config(VideoDeviceConfig& videoConfig)
{
	if (!m_video)
	{
		return E_FAIL;
	}

	return m_video->set_video_config(videoConfig);
}

HRESULT VideoBox::connect_filters()
{
	if (!m_video)
	{
		return E_FAIL;
	}
	return m_video->connect_filters();
}

HRESULT VideoBox::start()
{
	if (!m_video)
	{
		return E_FAIL;
	}
	return m_video->start();
}

HRESULT VideoBox::stop()
{
	if (!m_video)
	{
		return E_FAIL;
	}
	return m_video->stop();
}