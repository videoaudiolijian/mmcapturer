#ifndef _H_SAMPLE_FILTER_H_
#define _H_SAMPLE_FILTER_H_

#include <windows.h>
#include <dshow.h>
#include <strmif.h>

#include "sample_utils.h"
#include "sample_mediadevice.h"

class SamplePin;

class SampleFilter : public IBaseFilter, public IAMFilterMiscFlags
{
public:

	SampleFilter(MMDevice* device, const GUID& mediaMajorType, const GUID& mediaSubType);
	virtual ~SampleFilter();

	// IUnknown interface
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IPersist interface
	STDMETHODIMP GetClassID(CLSID *pClsID);

	// IMediaFilter interface
	//The GetState method retrieves the filters's state (running, stopped, or paused).
	STDMETHODIMP GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *State);
	//The SetSyncSource method sets the reference clock.
	STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
	//The GetSyncSource method retrieves the current reference clock.
	STDMETHODIMP GetSyncSource(IReferenceClock **pClock);
	//The Stop method stops the filter.
	STDMETHODIMP Stop();
	//The Pause method pauses the filter.
	STDMETHODIMP Pause();
	//The Run method runs the filter.
	STDMETHODIMP Run(REFERENCE_TIME tStart);

	// IBaseFilter interface
	//The EnumPins method enumerates the pins on this filter.
	STDMETHODIMP EnumPins(IEnumPins **ppEnum);
	//The FindPin method retrieves the pin with the specified identifier.
	STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);
	//The QueryFilterInfo method retrieves information about the filter.
	STDMETHODIMP QueryFilterInfo(FILTER_INFO *pInfo);
	//The JoinFilterGraph method notifies the filter that it has joined or left the filter graph.
	STDMETHODIMP JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName);
	//The QueryVendorInfo method retrieves a string containing vendor information.
	STDMETHODIMP QueryVendorInfo(LPWSTR *pVendorInfo);

	//IAMFilterMiscFlags interface
	STDMETHODIMP_(ULONG) GetMiscFlags();

	SamplePin *GetPin() const
	{
		return m_pin;
	}

	const FILTER_STATE& GetFilterState() const
	{
		return m_state;
	}

	inline void OnMediaSampleReceive(IMediaSample* sample)
	{
		m_device->on_sample_data(sample);
	}

private:
	volatile long m_ref_count;
	FILTER_STATE m_state;
	SamplePin* m_pin;
	IFilterGraph* m_graph;
	MMDevice* m_device;
};

#endif