#include "sample_utils.h"

#include <new>
#include <vector>

#include "common_logger.h"

static BOOL GetFourCCVFormat(DWORD fourCC, VideoFormat &format)
{
	switch (fourCC)
	{
	case MAKEFOURCC('R', 'G', 'B', '1'):
		format = VideoFormat::RGB24;
	case MAKEFOURCC('R', 'G', 'B', '2'):
		format = VideoFormat::XRGB32;
		break;
	case MAKEFOURCC('R', 'G', 'B', '4'):
		format = VideoFormat::XRGB32;
		break;
	case MAKEFOURCC('A', 'R', 'G', 'B'):
		format = VideoFormat::ARGB32;
		break;
	case MAKEFOURCC('I', '4', '2', '0'):
	case MAKEFOURCC('I', 'Y', 'U', 'V'):
		format = VideoFormat::IYUV;
		break;
	case MAKEFOURCC('Y', 'V', '1', '2'):
		format = VideoFormat::YV12;
		break;
	case MAKEFOURCC('N', 'V', '1', '2'):
		format = VideoFormat::NV12;
		break;
	case MAKEFOURCC('A', 'Y', 'U', 'V'):
		format = VideoFormat::AYUV;
		break;
	case MAKEFOURCC('Y', 'V', 'Y', 'U'):
		format = VideoFormat::YVYU;
		break;
	case MAKEFOURCC('Y', 'U', 'Y', '2'):
		format = VideoFormat::YUY2;
		break;
	case MAKEFOURCC('U', 'Y', 'V', 'Y'):
		format = VideoFormat::UYVY;
		break;
	case MAKEFOURCC('H', '2', '6', '4'):
		format = VideoFormat::H264;
		break;
	case MAKEFOURCC('M', 'J', 'P', 'G'):
		format = VideoFormat::MJPEG;
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

WORD VideoFormatBits(VideoFormat format)
{
	switch (format)
	{
	case VideoFormat::ARGB32:
	case VideoFormat::XRGB32:
		return 32;

	case VideoFormat::RGB24:
		return 24;

	case VideoFormat::RGB555:
	case VideoFormat::RGB565:
		return 16;

	case VideoFormat::IYUV:
	case VideoFormat::NV12:
	case VideoFormat::YV12:
		return 12;

	case VideoFormat::AYUV:
		return 24;

	case VideoFormat::YVYU:
	case VideoFormat::YUY2:
	case VideoFormat::UYVY:
		return 16;

	default:
		return 0;
	}
}

BOOL GetMediaTypeVideoFormat(const AM_MEDIA_TYPE &mt, VideoFormat &format)
{
	if (mt.majortype != MEDIATYPE_Video)
	{
		return FALSE;
	}

	const BITMAPINFOHEADER *bmih = GetBitmapInfoHeader(mt);

	format = VideoFormat::Unknown;

	if (mt.subtype == MEDIASUBTYPE_RGB24)
	{
		format = VideoFormat::RGB24;
	}
	else if (mt.subtype == MEDIASUBTYPE_RGB32)
	{
		format = VideoFormat::XRGB32;
	}
	else if (mt.subtype == MEDIASUBTYPE_ARGB32)
	{
		format = VideoFormat::ARGB32;
	}
	else if (mt.subtype == MEDIASUBTYPE_RGB555)
	{
		format = VideoFormat::RGB555;
	}
	else if (mt.subtype == MEDIASUBTYPE_RGB565)
	{
		format = VideoFormat::RGB565;
	}
	else if (mt.subtype == MEDIASUBTYPE_IYUV)
	{
		format = VideoFormat::IYUV;
	}
	else if (mt.subtype == MEDIASUBTYPE_YV12)
	{
		format = VideoFormat::YV12;
	}
	else if (mt.subtype == MEDIASUBTYPE_NV12)
	{
		format = VideoFormat::NV12;
	}
	else if (mt.subtype == MEDIASUBTYPE_AYUV)
	{
		format = VideoFormat::AYUV;
	}
	else if (mt.subtype == MEDIASUBTYPE_YVYU)
	{
		format = VideoFormat::YVYU;
	}
	else if (mt.subtype == MEDIASUBTYPE_YUY2)
	{
		format = VideoFormat::YUY2;
	}
	else if (mt.subtype == MEDIASUBTYPE_UYVY)
	{
		format = VideoFormat::UYVY;
	}
	else if (mt.subtype == MEDIASUBTYPE_H264)
	{
		format = VideoFormat::H264;
	}
	else if (mt.subtype == MEDIASUBTYPE_MJPG)
	{
		format = VideoFormat::MJPEG;
	}
	else
	{
		return bmih ? GetFourCCVFormat(bmih->biCompression, format) : FALSE;
	}

	return TRUE;
}

DWORD VideoFormatToFourCC(VideoFormat format)
{
	switch (format)
	{
	case VideoFormat::RGB24:
		return MAKEFOURCC('R', 'G', 'B', '1');
	case VideoFormat::ARGB32:
		return MAKEFOURCC('A', 'R', 'G', 'B');
	case VideoFormat::XRGB32:
		return MAKEFOURCC('R', 'G', 'B', '4');

	case VideoFormat::IYUV:
		return MAKEFOURCC('I', 'Y', 'U', 'V');
	case VideoFormat::NV12:
		return MAKEFOURCC('N', 'V', '1', '2');
	case VideoFormat::YV12:
		return MAKEFOURCC('Y', 'V', '1', '2');

	case VideoFormat::AYUV:
		return MAKEFOURCC('A', 'Y', 'U', 'V');
	case VideoFormat::YVYU:
		return MAKEFOURCC('Y', 'V', 'Y', 'U');
	case VideoFormat::YUY2:
		return MAKEFOURCC('Y', 'U', 'Y', '2');
	case VideoFormat::UYVY:
		return MAKEFOURCC('U', 'Y', 'V', 'Y');

	case VideoFormat::MJPEG:
		return MAKEFOURCC('M', 'J', 'P', 'G');
	case VideoFormat::H264:
		return MAKEFOURCC('H', '2', '6', '4');

	default:
		return 0;
	}
}

GUID VideoFormatToSubType(VideoFormat format)
{
	switch (format)
	{
	case VideoFormat::RGB555:
		return MEDIASUBTYPE_RGB555;
	case VideoFormat::RGB565:
		return MEDIASUBTYPE_RGB565;
	case VideoFormat::ARGB32:
		return MEDIASUBTYPE_ARGB32;
	case VideoFormat::XRGB32:
		return MEDIASUBTYPE_RGB32;
	case VideoFormat::RGB24:
		return MEDIASUBTYPE_RGB24;

	case VideoFormat::IYUV:
		return MEDIASUBTYPE_IYUV;
	case VideoFormat::NV12:
		return MEDIASUBTYPE_NV12;
	case VideoFormat::YV12:
		return MEDIASUBTYPE_YV12;

	case VideoFormat::AYUV:
		return MEDIASUBTYPE_AYUV;
	case VideoFormat::YVYU:
		return MEDIASUBTYPE_YVYU;
	case VideoFormat::YUY2:
		return MEDIASUBTYPE_YUY2;
	case VideoFormat::UYVY:
		return MEDIASUBTYPE_UYVY;

	case VideoFormat::MJPEG:
		return MEDIASUBTYPE_MJPG;
	case VideoFormat::H264:
		return MEDIASUBTYPE_H264;

	default:
		return GUID_NULL;
	}
}

WORD VideoFormatPlanes(VideoFormat format)
{
	switch (format)
	{
	case VideoFormat::ARGB32:
	case VideoFormat::XRGB32:
	case VideoFormat::RGB555:
	case VideoFormat::RGB565:
	case VideoFormat::RGB24:
		return 1;

	case VideoFormat::IYUV:
		return 3;
	case VideoFormat::NV12:
		return 2;
	case VideoFormat::YV12:
		return 3;

	case VideoFormat::AYUV:
	case VideoFormat::YVYU:
	case VideoFormat::YUY2:
	case VideoFormat::UYVY:
		return 1;

	default:
		return 0;
	}
}

BITMAPINFOHEADER *GetBitmapInfoHeader(const AM_MEDIA_TYPE &mt)
{
	if (mt.formattype == FORMAT_VideoInfo)
	{
		VIDEOINFOHEADER *vih = reinterpret_cast<VIDEOINFOHEADER *>(mt.pbFormat);
		return &vih->bmiHeader;
	}
	else if (mt.formattype == FORMAT_VideoInfo2)
	{
		VIDEOINFOHEADER2 *vih = reinterpret_cast<VIDEOINFOHEADER2 *>(mt.pbFormat);
		return &vih->bmiHeader;
	}

	return NULL;
}

HRESULT CopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource)
{
	if (!pmtSource || !pmtTarget)
	{
		return S_FALSE;
	}

	*pmtTarget = *pmtSource;

	if (pmtSource->cbFormat && pmtSource->pbFormat)
	{
		pmtTarget->pbFormat = (BYTE*)CoTaskMemAlloc(pmtSource->cbFormat);

		if (pmtTarget->pbFormat == NULL)
		{
			pmtTarget->cbFormat = 0;
			return E_OUTOFMEMORY;
		}
		else
		{
			memcpy(pmtTarget->pbFormat, pmtSource->pbFormat, pmtTarget->cbFormat);
		}
	}

	if (pmtTarget->pUnk != NULL)
	{
		pmtTarget->pUnk->AddRef();
	}

	return S_OK;
}

void FreeMediaType(AM_MEDIA_TYPE &mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((LPVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}

	if (mt.pUnk)
	{
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

AM_MEDIA_TYPE CreateMediaType(VideoFormat format, int cx, int cy,
	long long interval)
{
	AM_MEDIA_TYPE mt;
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));

	WORD bits = VideoFormatBits(format);
	DWORD size = cx * cy * bits / 8;
	DWORD rate = size * 10000000UL / (DWORD)interval * 8UL;

	mt.pbFormat = (PBYTE)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER));
	if (mt.pbFormat)
	{
		mt.cbFormat = sizeof(VIDEOINFOHEADER);
		ZeroMemory(mt.pbFormat, sizeof(VIDEOINFOHEADER));

		VIDEOINFOHEADER *vih = (VIDEOINFOHEADER*)mt.pbFormat;
		vih->bmiHeader.biSize = sizeof(vih->bmiHeader);
		vih->bmiHeader.biWidth = cx;
		vih->bmiHeader.biHeight = cy;
		vih->bmiHeader.biPlanes = VideoFormatPlanes(format);
		vih->bmiHeader.biBitCount = bits;
		vih->bmiHeader.biSizeImage = size;
		vih->bmiHeader.biCompression = VideoFormatToFourCC(format);
		vih->rcSource.right = cx;
		vih->rcSource.bottom = cy;
		vih->rcTarget = vih->rcSource;
		vih->dwBitRate = (DWORD)rate;
		vih->AvgTimePerFrame = interval;
	}

	mt.majortype = MEDIATYPE_Video;
	mt.subtype = VideoFormatToSubType(format);
	mt.formattype = FORMAT_VideoInfo;
	mt.bFixedSizeSamples = TRUE;
	mt.lSampleSize = size;

	return mt;
}

static BOOL IsPinConfigMajorType(IPin *pin, const GUID &type)
{
	HRESULT hr;
	IAMStreamConfig* config;
	int count;
	int size;

	hr = pin->QueryInterface(IID_IAMStreamConfig, (void **)&config);
	if (FAILED(hr))
	{
		return FALSE;
	}

	hr = config->GetNumberOfCapabilities(&count, &size);
	if (FAILED(hr))
	{
		config->Release();
		return FALSE;
	}

	std::vector<BYTE> caps;
	caps.resize(size);

	for (int i = 0; i < count; i++)
	{
		AM_MEDIA_TYPE* mt = NULL;
		if (SUCCEEDED(config->GetStreamCaps(i, &mt, caps.data())))
		{
			if (mt->majortype == type)
			{
				FreeMediaType(*mt);
				CoTaskMemFree(mt);
				mt = NULL;
				config->Release();

				return TRUE;
			}

			FreeMediaType(*mt);
			CoTaskMemFree(mt);
		}
	}

	config->Release();
	return FALSE;
}

static BOOL IsPinMajorType(IPin *pin, const GUID &type)
{
	HRESULT hr;
	AM_MEDIA_TYPE* mediaType = NULL;
	IEnumMediaTypes* mediaEnum = NULL;

	if (IsPinConfigMajorType(pin, type))
	{
		return TRUE;
	}

	if (FAILED(pin->EnumMediaTypes(&mediaEnum)))
	{
		return FALSE;
	}

	ULONG fetched;
	hr = mediaEnum->Next(1, &mediaType, &fetched);
	if (FAILED(hr) || !mediaType)
	{
		mediaEnum->Release();
		return FALSE;
	}
	mediaEnum->Release();

	BOOL ret = (mediaType->majortype == type);

	FreeMediaType(*mediaType);
	CoTaskMemFree(mediaType);

	return ret;
}

static BOOL IsPinDirection(IPin *pin, const PIN_DIRECTION& dir)
{
	if (!pin)
	{
		return FALSE;
	}

	PIN_DIRECTION pinDir;
	return SUCCEEDED(pin->QueryDirection(&pinDir)) && pinDir == dir;
}

static HRESULT GetPinCategory(IPin *pin, GUID &category)
{
	if (!pin)
	{
		return E_POINTER;
	}

	IKsPropertySet* propertySet;
	DWORD size;

	pin->QueryInterface(IID_IKsPropertySet, (void **)&propertySet);
	if (propertySet == NULL)
	{
		return E_NOINTERFACE;
	}

	HRESULT hr = propertySet->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
		NULL, 0, &category, sizeof(GUID), &size);

	propertySet->Release();

	return hr;
}

static BOOL IsPinCategory(IPin *pin, const GUID &category)
{
	if (!pin)
	{
		return FALSE;
	}

	GUID pinCategory;
	HRESULT hr = GetPinCategory(pin, pinCategory);
	if (FAILED(hr))
	{
		return (hr == E_NOINTERFACE);
	}

	return category == pinCategory;
}

static BOOL MatchPin(IPin *pin, const GUID &type, const GUID &category, const PIN_DIRECTION &dir)
{
	if (!IsPinMajorType(pin, type))
	{
		return FALSE;
	}

	if (!IsPinDirection(pin, dir))
	{
		return FALSE;
	}

	if (!IsPinCategory(pin, category))
	{
		return FALSE;
	}

	return TRUE;
}

HRESULT GetPinOfFilter(IBaseFilter *filter, const GUID &type, const GUID &category,
	const PIN_DIRECTION& dir, IPin **pin)
{
	IPin* currentPin = NULL;
	IEnumPins* enumPins;
	ULONG count;

	if (!filter || !pin)
	{
		return E_POINTER;
	}

	if (FAILED(filter->EnumPins(&enumPins)))
	{
		return E_FAIL;
	}

	while (S_OK == (enumPins->Next(1, &currentPin, &count)))
	{
		if (MatchPin(currentPin, type, category, dir))
		{
			*pin = currentPin;
			(*pin)->AddRef();

			currentPin->Release();
			enumPins->Release();
			return S_OK;
		}

		currentPin->Release();
		currentPin = NULL;
	}

	enumPins->Release();

	return E_FAIL;
}

HRESULT GetPinOfFilterByDirection(IBaseFilter *filter, const PIN_DIRECTION& dir, IPin **pin)
{
	IPin* currentPin;
	IEnumPins* enumPins;
	ULONG count;

	if (!filter || !pin)
	{
		return E_POINTER;
	}

	if (FAILED(filter->EnumPins(&enumPins)))
	{
		return E_FAIL;
	}

	while (S_OK == (enumPins->Next(1, &currentPin, &count)))
	{
		if (IsPinDirection(currentPin, dir))
		{
			*pin = currentPin;
			(*pin)->AddRef();

			currentPin->Release();
			enumPins->Release();
			return S_OK;
		}

		currentPin->Release();
	}

	enumPins->Release();

	return E_FAIL;
}

static BOOL EnumVideoCaps(IPin* pin, std::vector<VideoInfo>& videoInfos)
{
	HRESULT hr;
	IAMStreamConfig* streamConfig;
	int count, size;

	hr = pin->QueryInterface(IID_IAMStreamConfig, (void **)&streamConfig);
	if (FAILED(hr))
	{
		return FALSE;
	}

	hr = streamConfig->GetNumberOfCapabilities(&count, &size);
	if (FAILED(hr))
	{
		streamConfig->Release();
		return FALSE;
	}

	std::vector<BYTE> caps;
	caps.resize(size);

	for (int i = 0; i < count; i++)
	{
		AM_MEDIA_TYPE* mt = NULL;
		hr = streamConfig->GetStreamCaps(i, &mt, caps.data());
		if (SUCCEEDED(hr))
		{
			VideoInfo info;

			if (mt->formattype == FORMAT_VideoInfo)
			{
				if (GetVideoInfo(*mt, caps.data(), info))
				{
					videoInfos.push_back(info);
				}
			}

			FreeMediaType(*mt);
			CoTaskMemFree(mt);
		}
	}

	streamConfig->Release();
	return TRUE;
}

static BOOL EnumVideoDevice(IMoniker* moniker, std::vector<VideoDeviceInfo> &devices)
{
	IPropertyBag* propertyBag = NULL;
	IBaseFilter* filter = NULL;
	HRESULT hr;

	hr = moniker->BindToStorage(0, 0, IID_IPropertyBag,
		(void **)&propertyBag);
	if (FAILED(hr))
	{
		return FALSE;
	}

	VARIANT deviceName;
	VARIANT devicePath;
	deviceName.vt = VT_BSTR;
	devicePath.vt = VT_BSTR;
	deviceName.bstrVal = NULL;
	devicePath.bstrVal = NULL;

	hr = propertyBag->Read(L"FriendlyName", &deviceName, NULL);
	if (FAILED(hr))
	{
		propertyBag->Release();
		return FALSE;
	}

	propertyBag->Read(L"DevicePath", &devicePath, NULL);

	hr = moniker->BindToObject(NULL, 0, IID_IBaseFilter, (void **)&filter);
	if (SUCCEEDED(hr))
	{
		IPin* pin = NULL;
		hr = GetPinOfFilter(filter, MEDIATYPE_Video,
			PIN_CATEGORY_CAPTURE, PINDIR_OUTPUT, &pin);
		if (SUCCEEDED(hr))
		{
			VideoDeviceInfo info;
			if (EnumVideoCaps(pin, info.infos))
			{
				info.namew = deviceName.bstrVal;
				if (devicePath.bstrVal)
				{
					info.pathw = devicePath.bstrVal;
				}

				devices.push_back(info);
			}
			pin->Release();
		}
		filter->Release();
	}

	propertyBag->Release();
	return TRUE;
}

HRESULT EnumAllVideoDevices(std::vector<VideoDeviceInfo> &devices)
{
	devices.clear();

	ICreateDevEnum* deviceEnum = NULL;
	IEnumMoniker* enumMoniker = NULL;
	IMoniker* moniker = NULL;
	HRESULT hr;
	DWORD count = 0;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&deviceEnum);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	hr = deviceEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enumMoniker, 0);
	if (FAILED(hr))
	{
		deviceEnum->Release();
		return E_FAIL;
	}

	if (hr == S_OK)
	{
		while (enumMoniker->Next(1, &moniker, &count) == S_OK)
		{
			(void)EnumVideoDevice(moniker, devices);
			moniker->Release();
		}
	}

	enumMoniker->Release();
	deviceEnum->Release();
	return S_OK;
}

static BOOL EnumAudioCaps(IPin* pin, std::vector<AudioInfo>& audioInfos)
{
	HRESULT hr;
	IAMStreamConfig* streamConfig;
	int count, size;

	hr = pin->QueryInterface(IID_IAMStreamConfig, (void **)&streamConfig);
	if (FAILED(hr))
	{
		return FALSE;
	}

	hr = streamConfig->GetNumberOfCapabilities(&count, &size);
	if (FAILED(hr))
	{
		streamConfig->Release();
		return FALSE;
	}

	std::vector<BYTE> caps;
	caps.resize(size);

	for (int i = 0; i < count; i++)
	{
		AM_MEDIA_TYPE* mt = NULL;
		hr = streamConfig->GetStreamCaps(i, &mt, caps.data());
		if (SUCCEEDED(hr))
		{
			AudioInfo info;

			if (mt->formattype == FORMAT_WaveFormatEx)
			{
				if (GetAudioInfo(*mt, caps.data(), info))
				{
					audioInfos.push_back(info);
				}
			}

			FreeMediaType(*mt);
			CoTaskMemFree(mt);
		}
	}

	streamConfig->Release();
	return TRUE;
}


static BOOL EnumAudioDevice(IMoniker* moniker, std::vector<AudioDeviceInfo> &devices)
{
	IPropertyBag* propertyBag = NULL;
	IBaseFilter* filter = NULL;
	HRESULT hr;

	hr = moniker->BindToStorage(0, 0, IID_IPropertyBag,
		(void **)&propertyBag);
	if (FAILED(hr))
	{
		return FALSE;
	}

	VARIANT deviceName;
	VARIANT devicePath;
	deviceName.vt = VT_BSTR;
	devicePath.vt = VT_BSTR;
	deviceName.bstrVal = NULL;
	devicePath.bstrVal = NULL;

	hr = propertyBag->Read(L"FriendlyName", &deviceName, NULL);
	if (FAILED(hr))
	{
		propertyBag->Release();
		return FALSE;
	}

	propertyBag->Read(L"DevicePath", &devicePath, NULL);

	hr = moniker->BindToObject(NULL, 0, IID_IBaseFilter, (void **)&filter);
	if (SUCCEEDED(hr))
	{
		IPin* pin = NULL;
		hr = GetPinOfFilter(filter, MEDIATYPE_Audio,
			PIN_CATEGORY_CAPTURE, PINDIR_OUTPUT, &pin);
		if (SUCCEEDED(hr))
		{
			AudioDeviceInfo info;
			if (EnumAudioCaps(pin, info.infos))
			{
				info.namew = deviceName.bstrVal;
				if (devicePath.bstrVal)
				{
					info.pathw = devicePath.bstrVal;
				}

				devices.push_back(info);
			}
			pin->Release();
		}
		filter->Release();
	}

	propertyBag->Release();
	return TRUE;
}

HRESULT EnumAllAudioInputDevices(std::vector<AudioDeviceInfo> &devices)
{
	devices.clear();

	ICreateDevEnum* deviceEnum = NULL;
	IEnumMoniker* enumMoniker = NULL;
	IMoniker* moniker = NULL;
	HRESULT hr;
	DWORD count = 0;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&deviceEnum);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	hr = deviceEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &enumMoniker, 0);
	if (FAILED(hr))
	{
		deviceEnum->Release();
		return E_FAIL;
	}

	if (hr == S_OK)
	{
		while (enumMoniker->Next(1, &moniker, &count) == S_OK)
		{
			(void)EnumAudioDevice(moniker, devices);
			moniker->Release();
		}
	}

	enumMoniker->Release();
	deviceEnum->Release();
	return S_OK;
}

static BOOL OnFindDevice(const wchar_t *name, const wchar_t *path,
	const wchar_t *destName, const wchar_t *destPath)
{
	if (!destName || wcslen(destName) == 0 || wcscmp(name, destName) != 0)
	{
		return FALSE;
	}

	if (!path && !destPath && wcscmp(path, destPath) != 0)
	{
		return FALSE;
	}

	return TRUE;
}

static BOOL GetDevice(const IID &type, IMoniker* moniker, const wchar_t *name, const wchar_t *path,
	IBaseFilter **filterOut)
{
	IPropertyBag* propertyBag = NULL;
	IBaseFilter* filter = NULL;
	HRESULT hr;

	hr = moniker->BindToStorage(0, 0, IID_IPropertyBag,
		(void **)&propertyBag);
	if (FAILED(hr))
	{
		return FALSE;
	}

	VARIANT deviceName;
	VARIANT devicePath;
	deviceName.vt = VT_BSTR;
	devicePath.vt = VT_BSTR;
	deviceName.bstrVal = NULL;
	devicePath.bstrVal = NULL;

	hr = propertyBag->Read(L"FriendlyName", &deviceName, NULL);
	if (FAILED(hr))
	{
		propertyBag->Release();
		return FALSE;
	}

	propertyBag->Read(L"DevicePath", &devicePath, NULL);

	hr = moniker->BindToObject(NULL, 0, IID_IBaseFilter, (void **)&filter);
	if (SUCCEEDED(hr))
	{
		if (OnFindDevice(deviceName.bstrVal, devicePath.bstrVal, name, path))
		{
			*filterOut = filter;
			propertyBag->Release();
			return TRUE;
		}
	}

	propertyBag->Release();
	return FALSE;
}


HRESULT GetDeviceFilter(const IID &type, const wchar_t *name, const wchar_t *path,
	IBaseFilter **filterOut)
{
	ICreateDevEnum* deviceEnum = NULL;
	IEnumMoniker* enumMoniker = NULL;
	IMoniker* moniker = NULL;
	HRESULT hr;
	DWORD count = 0;

	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&deviceEnum);
	if (FAILED(hr))
	{
		LOG_ERROR("Create ICreateDevEnum object failed.");
		return E_FAIL;
	}

	hr = deviceEnum->CreateClassEnumerator(type, &enumMoniker, 0);
	if (FAILED(hr))
	{
		LOG_ERROR("Create IEnumMoniker object failed.");

		deviceEnum->Release();
		return E_FAIL;
	}

	if (hr == S_OK)
	{
		while (enumMoniker->Next(1, &moniker, &count) == S_OK)
		{
			if (GetDevice(type, moniker, name, path, filterOut))
			{
				moniker->Release();
				enumMoniker->Release();
				deviceEnum->Release();

				return S_OK;
			}

			moniker->Release();
		}
	}

	LOG_ERROR("GetDeviceFilter filter Object not found.");

	enumMoniker->Release();
	deviceEnum->Release();
	return E_FAIL;
}

BOOL GetVideoInfo(const AM_MEDIA_TYPE &mt, const BYTE *data, VideoInfo &info)
{
	const VIDEO_STREAM_CONFIG_CAPS *vscc;
	const VIDEOINFOHEADER *viHeader;
	const BITMAPINFOHEADER *bmiHeader;
	VideoFormat format;

	vscc = reinterpret_cast<const VIDEO_STREAM_CONFIG_CAPS *>(data);
	viHeader = reinterpret_cast<const VIDEOINFOHEADER *>(mt.pbFormat);
	bmiHeader = &viHeader->bmiHeader;

	if (!GetMediaTypeVideoFormat(mt, format))
	{
		return FALSE;
	}

	info.format = format;

	if (vscc)
	{
		info.minInterval = vscc->MinFrameInterval;
		info.maxInterval = vscc->MaxFrameInterval;
		info.minCX = vscc->MinOutputSize.cx;
		info.minCY = vscc->MinOutputSize.cy;
		info.maxCX = vscc->MaxOutputSize.cx;
		info.maxCY = vscc->MaxOutputSize.cy;

		if (!info.minCX || !info.minCY || !info.maxCX || !info.maxCY)
		{
			info.minCX = info.maxCX = bmiHeader->biWidth;
			info.minCY = info.maxCY = bmiHeader->biHeight;
		}

		info.granularityCX = max(vscc->OutputGranularityX, 1);
		info.granularityCY = max(vscc->OutputGranularityY, 1);
	}
	else
	{
		info.minInterval = info.maxInterval = 10010000000LL / 60000LL;
		info.minCX = info.maxCX = bmiHeader->biWidth;
		info.minCY = info.maxCY = bmiHeader->biHeight;
		info.granularityCX = 1;
		info.granularityCY = 1;
	}

	return TRUE;
}

static void ClampToGranularity(LONG &val, int minVal, int granularity)
{
	val -= ((val - minVal) % granularity);
}

static int GetFormatRating(VideoFormat format)
{
	if (format >= VideoFormat::IYUV && format < VideoFormat::AYUV)
	{
		return 0;
	}
	else if (format >= VideoFormat::AYUV && format < VideoFormat::MJPEG)
	{
		return 5;
	}
	else if (format == VideoFormat::MJPEG)
	{
		return 10;
	}

	return 15;
}

static BOOL OnVideoStreamCaps(const AM_MEDIA_TYPE& mt, const BYTE *capData, BOOL &found, long long &bestMatch,
	const VideoDeviceConfig &videoConfig, AM_MEDIA_TYPE &mediaType)
{
	VideoInfo info;

	if (mt.formattype == FORMAT_VideoInfo)
	{
		if (!GetVideoInfo(mt, capData, info))
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	AM_MEDIA_TYPE copiedMT;
	ZeroMemory(&copiedMT, sizeof(AM_MEDIA_TYPE));
	CopyMediaType(&copiedMT, &mt);

	VIDEOINFOHEADER *vih = (VIDEOINFOHEADER *)copiedMT.pbFormat;
	BITMAPINFOHEADER *bmih = GetBitmapInfoHeader(copiedMT);

	if (videoConfig.internalFormat != VideoFormat::Any &&
		videoConfig.internalFormat != info.format)
	{
		FreeMediaType(copiedMT);
		return FALSE;
	}

	int xVal = 0;
	int yVal = 0;
	int formatVal = 0;
	long long frameVal = 0;

	if (videoConfig.cx < info.minCX)
	{
		xVal = info.minCX - videoConfig.cx;
	}
	else if (videoConfig.cx > info.maxCX)
	{
		xVal = videoConfig.cx - info.maxCX;
	}

	const int absMinCY = abs(info.minCY);
	const int absMaxCY = abs(info.maxCY);
	if (videoConfig.cyabs < absMinCY)
	{
		yVal = absMinCY - videoConfig.cyabs;
	}
	else if (videoConfig.cyabs > absMaxCY)
	{
		yVal = videoConfig.cyabs - absMaxCY;
	}

	const long long frameInterval = videoConfig.frameInterval;
	if (frameInterval < info.minInterval)
	{
		frameVal = info.minInterval - frameInterval;
	}
	else if (frameInterval > info.maxInterval)
	{
		frameVal = frameInterval - info.maxInterval;
	}

	formatVal = GetFormatRating(info.format);

	long long totalMatch = frameVal + yVal + xVal + formatVal;

	if (!found || totalMatch < bestMatch)
	{
		if (xVal == 0)
		{
			bmih->biWidth = videoConfig.cx;
			ClampToGranularity(bmih->biWidth, info.minCX,
				info.granularityCX);
		}

		if (yVal == 0)
		{
			LONG cy_abs_clamp = videoConfig.cyabs;
			ClampToGranularity(cy_abs_clamp, info.minCY,
				info.granularityCY);
			bmih->biHeight = videoConfig.cyflip ? -cy_abs_clamp
				: cy_abs_clamp;
		}

		if (frameVal == 0)
		{
			if (abs(vih->AvgTimePerFrame - frameInterval) > 1)
			{
				vih->AvgTimePerFrame = frameInterval;
			}
		}

		found = TRUE;
		bestMatch = totalMatch;

		FreeMediaType(mediaType);
		CopyMediaType(&mediaType, &copiedMT);

		if (totalMatch == 0)
		{
			FreeMediaType(copiedMT);
			return TRUE;
		}
	}

	FreeMediaType(copiedMT);
	return FALSE;
}

HRESULT GetClosestVideoMediaType(IAMStreamConfig* streamConfig,
	const VideoDeviceConfig &videoConfig, AM_MEDIA_TYPE &mediaType)
{
	HRESULT hr;
	int count;
	int size;

	hr = streamConfig->GetNumberOfCapabilities(&count, &size);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	std::vector<BYTE> caps;
	caps.resize(size);

	BOOL found = FALSE;
	long long bestMatch = 0;
	for (int i = 0; i < count; i++)
	{
		AM_MEDIA_TYPE* mt = NULL;
		hr = streamConfig->GetStreamCaps(i, &mt, caps.data());
		if (SUCCEEDED(hr))
		{
			if (OnVideoStreamCaps(*mt, caps.data(), found, bestMatch, videoConfig, mediaType))
			{
				FreeMediaType(*mt);
				CoTaskMemFree(mt);
				break;
			}

			FreeMediaType(*mt);
			CoTaskMemFree(mt);
		}
	}

	return S_OK;
}

static BOOL OnAudioStreamCaps(const AM_MEDIA_TYPE& mt, const BYTE *capData, BOOL &found, long long &bestMatch,
	const AudioDeviceConfig &audioConfig, AM_MEDIA_TYPE &mediaType)
{
	AudioInfo info;

	if (mt.formattype == FORMAT_WaveFormatEx)
	{
		if (!GetAudioInfo(mt, capData, info))
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	AM_MEDIA_TYPE copiedMT;
	ZeroMemory(&copiedMT, sizeof(AM_MEDIA_TYPE));
	CopyMediaType(&copiedMT, &mt);

	WAVEFORMATEX *wfex = (WAVEFORMATEX *)copiedMT.pbFormat;

	if (audioConfig.format != AudioFormat::Any &&
		audioConfig.format != info.format)
	{
		FreeMediaType(copiedMT);
		return FALSE;
	}

	int sampleRate = 0;
	int channels = 0;

	if (audioConfig.sampleRate < info.minSampleRate)
	{
		sampleRate = info.minSampleRate - audioConfig.sampleRate;
	}
	else if (audioConfig.sampleRate > info.maxSampleRate)
	{
		sampleRate = audioConfig.sampleRate - info.maxSampleRate;
	}
	else if (audioConfig.sampleRate == info.minSampleRate)
	{
		sampleRate = audioConfig.sampleRate;
	}
	else if (audioConfig.sampleRate == info.maxSampleRate)
	{
		sampleRate = audioConfig.sampleRate;
	}

	if (audioConfig.channels < info.minChannels)
	{
		channels = info.minChannels - audioConfig.channels;
	}
	else if (audioConfig.channels == info.minChannels)
	{
		channels = info.minChannels;
	}
	else if (info.maxChannels > audioConfig.channels)
	{
		channels = audioConfig.channels - info.maxChannels;
	}
	else if (audioConfig.channels == info.maxChannels)
	{
		channels = audioConfig.channels;
	}

	int totalVal = sampleRate + channels;

	if (!found || totalVal < bestMatch)
	{
		if (channels == 0)
		{
			LONG channels = audioConfig.channels;
			ClampToGranularity(channels, info.minChannels,
				info.channelsGranularity);
			wfex->nChannels = (WORD)channels;

			wfex->nBlockAlign =
				wfex->wBitsPerSample * wfex->nChannels / 8;
		}

		if (sampleRate == 0)
		{
			wfex->nSamplesPerSec = audioConfig.sampleRate;
			ClampToGranularity((LONG &)wfex->nSamplesPerSec,
				info.minSampleRate,
				info.sampleRateGranularity);
		}

		wfex->nAvgBytesPerSec =
			wfex->nSamplesPerSec * wfex->nBlockAlign;

		found = TRUE;
		bestMatch = totalVal;

		FreeMediaType(mediaType);
		CopyMediaType(&mediaType, &copiedMT);

		if (totalVal == 0)
		{
			FreeMediaType(copiedMT);
			return TRUE;
		}
	}

	FreeMediaType(copiedMT);
	return FALSE;
}

HRESULT GetClosestAudioMediaType(IAMStreamConfig* streamConfig, 
	const AudioDeviceConfig &audioConfig, AM_MEDIA_TYPE &mediaType)
{
	HRESULT hr;
	int count;
	int size;

	hr = streamConfig->GetNumberOfCapabilities(&count, &size);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	std::vector<BYTE> caps;
	caps.resize(size);

	BOOL found = FALSE;
	long long bestMatch = 0;
	for (int i = 0; i < count; i++)
	{
		AM_MEDIA_TYPE* mt = NULL;
		hr = streamConfig->GetStreamCaps(i, &mt, caps.data());
		if (SUCCEEDED(hr))
		{
			if (OnAudioStreamCaps(*mt, caps.data(), found, bestMatch, audioConfig, mediaType))
			{
				FreeMediaType(*mt);
				CoTaskMemFree(mt);
				break;
			}

			FreeMediaType(*mt);
			CoTaskMemFree(mt);
		}
	}

	return S_OK;
}

BOOL GetAudioInfo(const AM_MEDIA_TYPE &mt, const BYTE *data, AudioInfo &info)
{
	const AUDIO_STREAM_CONFIG_CAPS *audioCaps;
	const WAVEFORMATEX *waveFormat;

	audioCaps = reinterpret_cast<const AUDIO_STREAM_CONFIG_CAPS *>(data);
	waveFormat = reinterpret_cast<const WAVEFORMATEX *>(mt.pbFormat);

	if (!waveFormat || !audioCaps) 
	{
		return FALSE;
	}

	switch (waveFormat->wBitsPerSample) 
	{
	case 16:
		info.format = AudioFormat::Wave16bit;
		break;
	case 32:
		info.format = AudioFormat::WaveFloat;
		break;
	}

	info.minChannels = audioCaps->MinimumChannels;
	info.maxChannels = audioCaps->MaximumChannels;
	info.channelsGranularity = audioCaps->ChannelsGranularity;
	info.minSampleRate = audioCaps->MinimumSampleFrequency;
	info.maxSampleRate = audioCaps->MaximumSampleFrequency;
	info.sampleRateGranularity = audioCaps->SampleFrequencyGranularity;

	return TRUE;
}