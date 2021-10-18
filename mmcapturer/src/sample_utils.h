#ifndef _H_DIRECT_SHOW_SAMPLE_UTILS_H_
#define _H_DIRECT_SHOW_SAMPLE_UTILS_H_

#include <string>
#include <vector>
#include <functional>
#include <new>

#include <windows.h>
#include <strmif.h>
#include <dshow.h>
#include <dvdmedia.h>
#include <gdiplus.h>

enum class VideoFormat
{
	Any,
	Unknown,

	/*rgb*/
	RGB24 = 100,  //24 bits per pixel, R G B
	ARGB32,       //32 bits per pixel, A(alpha) R G B
	XRGB32,       //32 bits per pixel, the x is ignored, R G B
	RGB555,       //RGB 555, 16 bpp
	RGB565,       //RGB 565, 16 bpp, the G is 6 bits


	/*planar*/
	/* 4:2:0, 3 planes, IYUV consists of a Y plane, 
	 * followed by a U plane, followed by a V plane.
	 */
	IYUV = 200,

	/*
	 * 4:2:0, 2 planes
	 * All of the Y samples appear first in memory as an array of unsigned char values with an even number of lines.
	 * The Y plane is followed immediately by an array of unsigned char values that contains packed U (Cb) and V (Cr) samples. 
	 * When the combined U-V array is addressed as an array of little-endian WORD values, 
	 * the LSBs contain the U values, and the MSBs contain the V values. 
	 * NV12 is the preferred 4:2:0 pixel format for DirectX VA.
	 * It is expected to be an intermediate-term requirement for DirectX VA accelerators supporting 4:2:0 video. 
	 * The following illustration shows the Y plane and the array that contains packed U and V samples.
	 *
	 * Y0  Y1  Y2  Y3
	 * U0  V0  U1  V1
	 *
	 */
	NV12,

	/*
	 * 4:2:0, 3 planes
	 * All of the Y samples appear first in memory as an array of unsigned char values. 
	 * This array is followed immediately by all of the V (Cr) samples. 
	 * The stride of the V plane is half the stride of the Y plane; 
	 * and the V plane contains half as many lines as the Y plane. 
	 * The V plane is followed immediately by all of the U (Cb) samples, 
	 * with the same stride and number of lines as the V plane
	 *
	 * Y0  Y1  Y2  Y3
	 * V0  V1
	 * U0  U1
	 *
	 */
	YV12,

	/*packed*/
	AYUV = 300, //4:4:4
	YVYU,   //4:2:2
	YUY2,    //4:2:2
	UYVY,    //4:2:2

	MJPEG = 400,
	H264,
};

enum class AudioFormat
{
	Any,
	Unknown,

	/* raw formats */
	Wave16bit = 100,
	WaveFloat,

	/* encoded formats */
	AAC = 200,
	AC3,
	MPGA, /* MPEG 1 */
};

struct VideoInfo
{
	int minCX;
	int minCY;
	int maxCX;
	int maxCY;
	int granularityCX;
	int granularityCY;
	long long minInterval;
	long long maxInterval;
	VideoFormat format;
};

struct AudioInfo 
{
	int minChannels;
	int maxChannels;
	int channelsGranularity;
	int minSampleRate;
	int maxSampleRate;
	int sampleRateGranularity;
	AudioFormat format;
};

struct VideoDeviceInfo
{
	std::wstring namew;
	std::wstring pathw;
	std::string name;
	std::string path;

	std::vector<VideoInfo> infos;

	void ConvertString()
	{
		int len = WideCharToMultiByte(CP_UTF8, 0, namew.c_str(), -1, 0, 0, 0, 0);
		char *s = new (std::nothrow)char[len];
		if (s)
		{
			WideCharToMultiByte(CP_UTF8, 0, namew.c_str(), -1, s, len, 0, 0);
			name = std::string(s, len);
			delete[] s;
		}

		len = WideCharToMultiByte(CP_UTF8, 0, pathw.c_str(), -1, 0, 0, 0, 0);
		char *p = new (std::nothrow) char[len];
		if (p)
		{
			WideCharToMultiByte(CP_UTF8, 0, pathw.c_str(), -1, p, len, 0, 0);
			path = std::string(p, len);

			delete[] p;
		}
	}
};

struct AudioDeviceInfo
{
	std::wstring namew;
	std::wstring pathw;
	std::string name;
	std::string path;

	std::vector<AudioInfo> infos;

	void ConvertString()
	{
		int len = WideCharToMultiByte(CP_UTF8, 0, namew.c_str(), -1, 0, 0, 0, 0);
		char *s = new (std::nothrow)char[len];
		if (s)
		{
			WideCharToMultiByte(CP_UTF8, 0, namew.c_str(), -1, s, len, 0, 0);
			name = std::string(s, len);
			delete[] s;
		}

		len = WideCharToMultiByte(CP_UTF8, 0, pathw.c_str(), -1, 0, 0, 0, 0);
		char *p = new (std::nothrow) char[len];
		if (p)
		{
			WideCharToMultiByte(CP_UTF8, 0, pathw.c_str(), -1, p, len, 0, 0);
			path = std::string(p, len);

			delete[] p;
		}
	}
};

struct VideoDeviceConfig;
struct AudioDeviceConfig;

typedef std::function<void(const VideoDeviceConfig &config, unsigned char *data,
	size_t size, long long startTime, long long stopTime)> VideoCallback;

typedef std::function<void(const AudioDeviceConfig &config, unsigned char *data,
	size_t size, long long startTime, long long stopTime)> AudioCallback;

struct VideoDeviceConfig
{
	/*the callback of video data*/
	VideoCallback callback;

	/*use the device's default config*/
	bool useDefaultVideoConfig;

	/*name of the device*/
	std::wstring namew;
	/*path of the device*/
	std::wstring pathw;

	/*the desired with*/
	int cx;
	/*the desired height*/
	int cyabs;
	/*whether the y is negative*/
	bool cyflip;

	/* frame interval in 100-nanosecond units */
	long long frameInterval;

	/*the internal video format*/
	VideoFormat internalFormat;

	/*the desired video format*/
	VideoFormat format;

	VideoDeviceConfig()
	{
		useDefaultVideoConfig = true;
		cx = 0;
		cyabs = 0;
		cyflip = false;
		frameInterval = 0;
		internalFormat = VideoFormat::Any;
		format = VideoFormat::Any;
	}
};

struct AudioDeviceConfig
{
	/*the callback of audio data*/
	AudioCallback callback;

	/*use the device's default config*/
	bool useDefaultAudioConfig;

	/*name of the device*/
	std::wstring namew;
	/*path of the device*/
	std::wstring pathw;
	
	/*the desired sample rate*/
	int sampleRate;
	/*the desired channels*/
	int channels;

	/*the desired audio format*/
	AudioFormat format;

	AudioDeviceConfig()
	{
		useDefaultAudioConfig = true;
		sampleRate = 0;
		channels = 0;
		format = AudioFormat::Any;
	}
};


DWORD VideoFormatToFourCC(VideoFormat format);
WORD VideoFormatPlanes(VideoFormat format);
GUID VideoFormatToSubType(VideoFormat format);
WORD VideoFormatBits(VideoFormat format);

BOOL GetMediaTypeVideoFormat(const AM_MEDIA_TYPE &mt, VideoFormat &format);

BITMAPINFOHEADER *GetBitmapInfoHeader(const AM_MEDIA_TYPE &mt);
void FreeMediaType(AM_MEDIA_TYPE &mt);
HRESULT CopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource);

AM_MEDIA_TYPE CreateMediaType(VideoFormat format, int cx, int cy, long long interval);

HRESULT GetPinOfFilter(IBaseFilter *filter, const GUID &type, const GUID &category,
	const PIN_DIRECTION& dir, IPin **pin);

HRESULT GetPinOfFilterByDirection(IBaseFilter *filter, const PIN_DIRECTION& dir, IPin **pin);

HRESULT GetDeviceFilter(const IID &type, const wchar_t *name, const wchar_t *path,
	IBaseFilter **filterOut);

BOOL GetVideoInfo(const AM_MEDIA_TYPE &mt, const BYTE *data, VideoInfo &info);
BOOL GetAudioInfo(const AM_MEDIA_TYPE &mt, const BYTE *data, AudioInfo &info);

HRESULT GetClosestVideoMediaType(IAMStreamConfig* streamConfig, const VideoDeviceConfig &config,
	AM_MEDIA_TYPE &mt);
HRESULT GetClosestAudioMediaType(IAMStreamConfig* streamConfig, const AudioDeviceConfig &config,
	AM_MEDIA_TYPE &mt);

HRESULT EnumAllVideoDevices(std::vector<VideoDeviceInfo> &devices);
HRESULT EnumAllAudioInputDevices(std::vector<AudioDeviceInfo> &devices);

#endif