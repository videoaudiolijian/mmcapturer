#ifndef _H_SAMPLE_ENUM_MEDIA_TYPES_H_
#define _H_SAMPLE_ENUM_MEDIA_TYPES_H_

#include <dshow.h>
#include <strmif.h>

class SamplePin;

class SampleEnumMediaTypes : public IEnumMediaTypes
{
public:
	SampleEnumMediaTypes(SamplePin *pin);
	virtual ~SampleEnumMediaTypes();

	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	//The Clone method makes a copy of the enumerator. 
	//The returned object starts with the same enumeration state as the original.
	STDMETHODIMP Clone(IEnumMediaTypes **ppEnum);

	//The Next method retrieves a specified number of media types.
	STDMETHODIMP Next(ULONG cMediaTypes, AM_MEDIA_TYPE **ppMediaTypes, ULONG *pcFetched);

	//The Reset method resets the enumeration sequence to the beginning.
	STDMETHODIMP Reset();

	//The Skip method skips over a specified number of media types.
	STDMETHODIMP Skip(ULONG cMediaTypes);
private:
	volatile long m_ref_count;
	SamplePin * m_pin;

	int m_current_index;
};

#endif