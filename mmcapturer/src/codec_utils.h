#ifndef _H_CODEC_UTILS_H_
#define _H_CODEC_UTILS_H_

#include <stdint.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h> 
}

#include "sample_utils.h"

/** 
 RFC-6184 -- RTP Payload Format for H.264 Video


 H264 NALU(network abstract layer unit)
 IDR: instantaneous decoding refresh is I frame, but I frame not must be IDR

 |0|1|2|3|4|5|6|7|
 |F|NRI|Type     |

 F: forbidden_zero_bit, must be 0
 NRI:nal_ref_idc, the importance indicator
 Type: nal_unit_type

 0x67(103) (0 11 00111) SPS     very important       type = 7
 0x68(104) (0 11 01000) PPS     very important       type = 8
 0x65(101) (0 11 00101) IDR     very important       type = 5
 0x61(97)  (0 11 00001) I       important            type = 1 it's not IDR
 0x41(65)  (0 10 00001) P       important            type = 1
 0x01(1)   (0 00 00001) B       not important        type = 1
 0x06(6)   (0 00 00110) SEI     not important        type = 6

 NAL Unit Type     Packet Type      Packet Type Name
 -------------------------------------------------------------
 0                 reserved         -
 1-23              NAL unit         Single NAL unit packet
 24                STAP-A           Single-time aggregation packet
 25                STAP-B           Single-time aggregation packet
 26                MTAP16           Multi-time aggregation packet
 27                MTAP24           Multi-time aggregation packet
 28                FU-A             Fragmentation unit
 29                FU-B             Fragmentation unit
 30-31             reserved
*/

const uint8_t* avc_find_start_code(const uint8_t *start, const uint8_t *end);
bool avc_find_key_frame(const uint8_t *data, size_t size);
int count_avc_key_frames(const uint8_t *data, size_t size);
int count_frames(const uint8_t *data, size_t size);

struct VideoFrameRender
{
	uint8_t *data[3];
	int linesize[3];
	uint32_t width;
	uint32_t height;
	uint64_t timestamp;
	VideoFormat format;
	int ffmpegFormat;
	bool isFFmpeg;

	VideoFrameRender()
	{
		data[0] = data[1] = data[2] = 0;
		linesize[0] = linesize[1] = linesize[2] = 0;
		width = height = 0;
		timestamp = 0;
		format = VideoFormat::Unknown;
		isFFmpeg = false;
	}

	inline bool InitFromFFmpeg(AVFrame* frame)
	{
		isFFmpeg = true;

		for (int i = 0; i < 3; i++)
		{
			data[i] = frame->data[i];
			linesize[i] = frame->linesize[i];
		}

		return true;
	}

	inline bool InitFrom(uint8_t* data_p, uint32_t len, VideoFormat fmt, uint32_t width_p, uint32_t height_p)
	{
		data[0] = data[1] = data[2] = 0;
		linesize[0] = linesize[1] = linesize[2] = 0;

		isFFmpeg = false;
		if (fmt == VideoFormat::RGB24)
		{
			data[0] = data_p;
			linesize[0] = width_p * 3;
		}
		else if (fmt == VideoFormat::ARGB32 || fmt == VideoFormat::XRGB32)
		{
			data[0] = data_p;
			linesize[0] = width_p * 4;
		}
		else if (fmt == VideoFormat::RGB555 || fmt == VideoFormat::RGB565)
		{
			data[0] = data_p;
			linesize[0] = width_p * 2;
		}
		else if (fmt == VideoFormat::IYUV)
		{
			data[0] = data_p;
			linesize[0] = width_p;

			data[1] = data_p + width_p * height_p;
			linesize[1] = width_p / 2;

			data[2] = data_p + width_p * height_p * 5 / 4;
			linesize[2] = width_p / 2;
		}
		else if (fmt == VideoFormat::NV12)
		{
			data[0] = data_p;
			linesize[0] = width_p;

			data[1] = data_p + width_p * height_p;
			linesize[1] = width_p ;
		}
		else if (fmt == VideoFormat::YV12)
		{
			data[0] = data_p;
			linesize[0] = width_p;

			data[2] = data_p + width_p * height_p;
			linesize[2] = width_p / 2;

			data[1] = data_p + width_p * height_p * 5 / 4;
			linesize[1] = width_p / 2;
		}
		else if (fmt == VideoFormat::AYUV)
		{
			data[0] = data_p;
			linesize[0] = width_p * 3;
		}
		else if (fmt == VideoFormat::YVYU || fmt == VideoFormat::YUY2 || fmt == VideoFormat::UYVY)
		{
			data[0] = data_p;
			linesize[0] = width_p * 2;
		}
		else
		{
			return false;
		}

		return true;
	}
};

#define ALIGN_SIZE(size, align) (((size) + (align - 1)) & (~(align - 1)))

struct YUV420PEncodeFrame
{
	uint8_t *buffer;
	uint8_t *data[3];
	int linesize[3];
	uint32_t width;
	uint32_t height;
	uint64_t timestamp;

	YUV420PEncodeFrame()
	{
		int size = ALIGN_SIZE(1024 * 1024 * 6, 32);
		buffer = (uint8_t*)_aligned_malloc(size, 32);

		data[0] = data[1] = data[2] = 0;
		linesize[0] = linesize[1] = linesize[2] = 0;
		width = height = 0;
		timestamp = 0;
	}

	~YUV420PEncodeFrame()
	{
		if (buffer)
		{
			_aligned_free(buffer);
		}
	}

	void InitFrom(struct VideoFrameRender& vfr)
	{
		if (!buffer)
		{
			return;
		}

		data[0] = data[1] = data[2] = 0;
		linesize[0] = linesize[1] = linesize[2] = 0;

		width = vfr.width;
		height = vfr.height;
		timestamp = vfr.timestamp;

		uint32_t offset[3] = { 0 };

		int size = width * height;
		size = ALIGN_SIZE(size, 32);
		offset[0] = size;

		size += width * height / 4;
		size = ALIGN_SIZE(size, 32);
		offset[1] = size;

		size += width * height / 4;
		size = ALIGN_SIZE(size, 32);

		data[0] = buffer;
		linesize[0] = vfr.linesize[0];

		data[1] = data[0] + offset[0];
		linesize[1] = vfr.linesize[1];

		data[2] = data[0] + offset[1];
		linesize[2] = vfr.linesize[2];

		memcpy(data[0], vfr.data[0], width * height);
		memcpy(data[1], vfr.data[1], width * height / 4);
		memcpy(data[2], vfr.data[2], width * height / 4);
	}
};

//H264 nalu data
struct H264Nalu
{
	uint8_t *buffer;
	size_t used_length;

	H264Nalu()
	{
		buffer = new uint8_t[1024 * 64];
		used_length = 0;
	}

	~H264Nalu()
	{
		if (buffer)
		{
			delete[] buffer;
			buffer = NULL;
		}
	}
};

#endif
