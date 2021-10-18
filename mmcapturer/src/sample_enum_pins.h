#ifndef _H_SAMPLE_ENUM_PINS_H_
#define _H_SAMPLE_ENUM_PINS_H_

#include <dshow.h>
#include <strmif.h>

class SampleFilter;

class SampleEnumPins : public IEnumPins
{
public:
	SampleEnumPins(SampleFilter *filter, int current_index = 0);
	virtual ~SampleEnumPins();

	// IUnknown interface
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IEnumPins interface
	// The Next method retrieves a specified number of pins in the enumeration sequence.
	STDMETHODIMP Next(ULONG cPins, IPin **ppPins, ULONG *pcFetched);
	//The Skip method skips over a specified number of pins.
	STDMETHODIMP Skip(ULONG cPins);
	//The Reset method resets the enumeration sequence to the beginning.
	STDMETHODIMP Reset();
	//The Clone method makes a copy of the enumerator with the same enumeration state.
	STDMETHODIMP Clone(IEnumPins **ppEnum);

private:
	volatile long m_ref_count;
	SampleFilter* m_filter;
	int m_current_index;
};
#endif