#ifndef _H_SAMPLE_MEDIA_RAW_DATA_H_
#define _H_SAMPLE_MEDIA_RAW_DATA_H_

#include <stdint.h>
#include "sample_utils.h"

constexpr int RAW_VIDEO_DATA_SIZE = 1024 * 2 * 1024 * 2;
constexpr int RAW_AUDIO_DATA_SIZE = 1024 * 256;

struct VideoRawData
{
	unsigned char *data;
	size_t used;
	REFERENCE_TIME startTime;
	REFERENCE_TIME stopTime;
	VideoFormat format;
	int width;
	int height;

	VideoRawData()
	{
		data = new unsigned char[RAW_VIDEO_DATA_SIZE];
		used = 0;
		startTime = 0;
		stopTime = 0;
		format = VideoFormat::Unknown;
		width = 0;
		height = 0;
	}

	~VideoRawData()
	{
		delete[] data;
	}

	void Reset()
	{
		used = 0;
		startTime = 0;
		stopTime = 0;
		format = VideoFormat::Unknown;
		width = 0;
		height = 0;
	}
};

struct AudioRawData
{
	unsigned char *data;
	size_t used;
	REFERENCE_TIME startTime;
	REFERENCE_TIME stopTime;
	AudioFormat format;
	int sampleRate;
	int channels;

	AudioRawData()
	{
		data = new unsigned char[RAW_VIDEO_DATA_SIZE];
		used = 0;
		startTime = 0;
		stopTime = 0;
		format = AudioFormat::Unknown;
	}

	~AudioRawData()
	{
		delete[] data;
	}

	void Reset()
	{
		used = 0;
		startTime = 0;
		stopTime = 0;
		format = AudioFormat::Unknown;
	}
};

#endif