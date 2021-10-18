#ifndef _H_SAMPLE_PIN_H_
#define _H_SAMPLE_PIN_H_

#include <windows.h>
#include <dshow.h>
#include <strmif.h>

#include "sample_utils.h"

class SampleFilter;

class SamplePin : public IPin, public IMemInputPin
{
public:
	SamplePin(SampleFilter *filter, const GUID& mediaMajorType, const GUID& mediaSubType);
	virtual ~SamplePin();

	// IUnknown interface
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IPin interface
	//The Connect method connects the pin to another pin.
	STDMETHODIMP Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
	//The ReceiveConnection method accepts a connection from another pin
	STDMETHODIMP ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
	//The Disconnect method breaks the current pin connection.
	STDMETHODIMP Disconnect();
	//The ConnectedTo method retrieves a pointer to the connected pin, if any.
	STDMETHODIMP ConnectedTo(IPin **pPin);
	//The ConnectionMediaType method retrieves the media type for the current pin connection, if any.
	STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE *pmt);
	//The QueryPinInfo method retrieves information about the pin
	STDMETHODIMP QueryPinInfo(PIN_INFO *pInfo);
	//The QueryDirection method gets the direction of the pin
	STDMETHODIMP QueryDirection(PIN_DIRECTION *pPinDir);
	//The QueryId method retrieves an identifier for the pin.
	STDMETHODIMP QueryId(LPWSTR *Id);
	//The QueryAccept method determines whether the pin accepts a specified media type.
	STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE *pmt);
	//The EnumMediaTypes method enumerates the pin's preferred media types.
	STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **ppEnum);
	//The QueryInternalConnections method retrieves the pins that are connected internally to this pin (within the filter)
	STDMETHODIMP QueryInternalConnections(IPin **apPin, ULONG *nPin);
	//The EndOfStream method notifies the pin that no additional data is expected, 
	//until a new run command is issued to the filter
	STDMETHODIMP EndOfStream();
	//The BeginFlush method begins a flush operation
	STDMETHODIMP BeginFlush();
	//The EndFlush method ends a flush operation.
	STDMETHODIMP EndFlush();
	//The NewSegment method notifies the pin that media samples received after this call are grouped as a segment,
	//with a common start time, stop time, and rate.
	STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop,
		double dRate);

	// IMemInputPin interface
	//The GetAllocator method retrieves the memory allocator proposed by this pin. 
	//After the allocator has been selected, this method returns a pointer to the selected allocator
	STDMETHODIMP GetAllocator(IMemAllocator **ppAllocator);
	//The NotifyAllocator method specifies an allocator for the connection.
	STDMETHODIMP NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly);
	//The GetAllocatorRequirements method retrieves the allocator properties requested by the input pin.
	STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);
	//The Receive method receives the next media sample in the stream.
	STDMETHODIMP Receive(IMediaSample *pSample);
	//The ReceiveMultiple method receives multiple samples in the stream.
	STDMETHODIMP ReceiveMultiple(IMediaSample **pSamples, long nSamples, long *nSamplesProcessed);
	//The ReceiveCanBlock method determines whether calls to the IMemInputPin::Receive method might block.
	STDMETHODIMP ReceiveCanBlock();

	const AM_MEDIA_TYPE& GetMediaType() const
	{
		return this->m_media_type;
	}

	BOOL IsMediaTypeValid(const AM_MEDIA_TYPE *pmt) const;

private:
	volatile long m_ref_count;
	SampleFilter* m_filter;
	GUID m_media_major_type;
	GUID m_media_sub_type;
	IPin* m_peer_pin;
	AM_MEDIA_TYPE m_media_type;
	volatile BOOL m_flushing;
};


#endif