#ifndef _H_MULTIMEDIA_DEVICE_H_
#define _H_MULTIMEDIA_DEVICE_H_

#include <windows.h>
#include <dshow.h>
#include <strmif.h>

#include "sample_pin.h"
#include "sample_utils.h"

struct EncodeData
{
	REFERENCE_TIME lastStartTime;
	REFERENCE_TIME lastStopTime;
	std::vector<unsigned char> bytes;

	EncodeData()
	{
		lastStartTime = 0;
		lastStopTime = 0;
	}
};

class MMDevice
{
public:
	MMDevice();
	virtual ~MMDevice();

	HRESULT create_graph();
	HRESULT start();
	HRESULT stop();

	virtual HRESULT connect_filters() = 0;
	virtual HRESULT on_sample_data(IMediaSample* sample) = 0;

protected:
	HRESULT get_pin_media_type(IPin *pin, AM_MEDIA_TYPE &mt, const GUID& formatType);
	HRESULT connect_pins(const GUID &type, const GUID &category,
		IBaseFilter *filter, IBaseFilter *sample);
	HRESULT disconnect_filters();

protected:
	BOOL m_initialized;
	BOOL m_active;
	IGraphBuilder* m_graph_builder;
	ICaptureGraphBuilder2* m_capture_graph_builder;
	IMediaControl* m_media_control;
};

#endif