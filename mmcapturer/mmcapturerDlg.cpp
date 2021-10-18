
// mmcapturerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "mmcapturer.h"
#include "mmcapturerDlg.h"
#include "afxdialogex.h"

#include <new>
#include <comdef.h>
#include <dshow.h>

static const struct VIDEO_FORMAT_ENTRY {
	enum AVPixelFormat ffmpegFormat;
	VideoFormat videoFormat;

} VIDEO_FORMAT_MAP[] = {
	{ AV_PIX_FMT_YUV420P, VideoFormat::IYUV },
	{ AV_PIX_FMT_YUYV422, VideoFormat::YUY2 },
	{ AV_PIX_FMT_NV12, VideoFormat::NV12 },
	{ AV_PIX_FMT_UYVY422, VideoFormat::UYVY },
	{ AV_PIX_FMT_ARGB, VideoFormat::ARGB32 },
	{ AV_PIX_FMT_YVYU422, VideoFormat::YVYU },
	{ AV_PIX_FMT_RGB555,  VideoFormat::RGB555 },
	{ AV_PIX_FMT_RGB565,  VideoFormat::RGB565 },
	{ AV_PIX_FMT_RGB24,   VideoFormat::RGB24 },
};

// CmmcapturerDlg 对话框

CmmcapturerDlg::CmmcapturerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MMCAPTURER_DIALOG, pParent)
	, m_radio_play_type(0), m_render_video_thread_handle(NULL),
	m_render_video_event_handle(NULL), m_stop_video_event_handle(NULL),
	m_render_audio_thread_handle(NULL), m_render_audio_event_handle(NULL), 
	m_stop_audio_event_handle(NULL), m_has_error(FALSE), 
	m_media_box(NULL), m_ffmpeg_decoder(NULL),
	m_ffmpeg_transcoder(NULL), m_sdl_video_initialized(false),
	m_sdl_audio_initialized(false), m_audio_device_id(0),
	m_sdl_window(NULL), m_sdl_renderer(NULL), m_sdl_texture(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmmcapturerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_VIDEOS, m_combo_videos);
	DDX_Control(pDX, IDC_COMBO_AUDIOS, m_combo_audios);
	DDX_Radio(pDX, IDC_RADIO_AUDIO_VIDEO, m_radio_play_type);
}

BEGIN_MESSAGE_MAP(CmmcapturerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_QUERY, &CmmcapturerDlg::OnBnClickedBtnQuery)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CmmcapturerDlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_STOP, &CmmcapturerDlg::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDC_RADIO_AUDIO_VIDEO, &CmmcapturerDlg::OnBnClickedRadioPlayType)
	ON_BN_CLICKED(IDC_RADIO_VIDEO, &CmmcapturerDlg::OnBnClickedRadioPlayType)
	ON_BN_CLICKED(IDC_RADIO_AUDIO, &CmmcapturerDlg::OnBnClickedRadioPlayType)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CmmcapturerDlg 消息处理程序

BOOL CmmcapturerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		LOG_ERROR("SDL_Init(SDL_INIT_VIDEO) failed, error:%s", SDL_GetError());

		m_has_error = TRUE;
		goto exitFlag;
	}

	m_render_video_thread_handle = NULL;
	m_render_video_event_handle = CreateEventA(NULL, FALSE, FALSE, NULL);
	if (!m_render_video_event_handle)
	{
		m_has_error = TRUE;
		goto exitFlag;
	}

	m_stop_video_event_handle = CreateEventA(NULL, FALSE, FALSE, NULL);
	if (!m_stop_video_event_handle)
	{
		m_has_error = TRUE;
		goto exitFlag;
	}

	m_render_audio_thread_handle = NULL;
	m_render_audio_event_handle = CreateEventA(NULL, FALSE, FALSE, NULL);
	if (!m_render_audio_event_handle)
	{
		m_has_error = TRUE;
		goto exitFlag;
	}

	m_stop_audio_event_handle = CreateEventA(NULL, FALSE, FALSE, NULL);
	if (!m_stop_audio_event_handle)
	{
		m_has_error = TRUE;
		goto exitFlag;
	}

	m_media_box = new (std::nothrow) MediaBox();
	if (!m_media_box)
	{
		m_has_error = TRUE;
		goto exitFlag;
	}
	else
	{
		m_media_box->set_video_sample_callback(std::bind(&CmmcapturerDlg::on_video_sample, this));
		m_media_box->set_audio_sample_callback(std::bind(&CmmcapturerDlg::on_audio_sample, this));
	}

	m_ffmpeg_decoder = new (std::nothrow) FFmpegDecoder();
	if (!m_ffmpeg_decoder)
	{
		m_has_error = TRUE;
		goto exitFlag;
	}

	m_ffmpeg_transcoder = new (std::nothrow) FFmpegTranscoder();
	if (!m_ffmpeg_transcoder)
	{
		m_has_error = TRUE;
		goto exitFlag;
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE

exitFlag:
	if (m_render_video_event_handle)
	{
		CloseHandle(m_render_video_event_handle);
		m_render_video_event_handle = NULL;
	}

	if (m_stop_video_event_handle)
	{
		CloseHandle(m_stop_video_event_handle);
		m_stop_video_event_handle = NULL;
	}

	if (m_render_audio_event_handle)
	{
		CloseHandle(m_render_audio_event_handle);
		m_render_audio_event_handle = NULL;
	}

	if (m_stop_audio_event_handle)
	{
		CloseHandle(m_stop_audio_event_handle);
		m_stop_audio_event_handle = NULL;
	}

	if (m_media_box)
	{
		delete m_media_box;
		m_media_box = NULL;
	}

	if (m_ffmpeg_decoder)
	{
		delete m_ffmpeg_decoder;
		m_ffmpeg_decoder = NULL;
	}

	if (m_ffmpeg_transcoder)
	{
		delete m_ffmpeg_transcoder;
		m_ffmpeg_transcoder = NULL;
	}

	return TRUE;
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CmmcapturerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CmmcapturerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD WINAPI CmmcapturerDlg::render_video_thread(LPVOID ptr)
{
	LOG_INFO("Render video thread starts........");

	CmmcapturerDlg* dlg = (CmmcapturerDlg*)ptr;
	dlg->render_video_run();

	LOG_INFO("Render video thread ends........");

	return 0;
}

void CmmcapturerDlg::render_video_run()
{
	HANDLE signals[2] = { m_render_video_event_handle, m_stop_video_event_handle };
	DWORD ret = WaitForMultipleObjects(2, signals, FALSE, INFINITE);
	while (ret == WAIT_OBJECT_0 )
	{
		while (VideoRawData* raw = m_media_box->begin_raw_video_data())
		{
			if (raw->format == VideoFormat::H264)
			{
				decode_video(AV_CODEC_ID_H264, raw->data, raw->used, raw->startTime, raw->stopTime);
			}
			else if (raw->format == VideoFormat::MJPEG)
			{
				decode_video(AV_CODEC_ID_MJPEG, raw->data, raw->used, raw->startTime, raw->stopTime);
			}
			else if (raw->format == VideoFormat::Any || raw->format == VideoFormat::Unknown)
			{
				LOG_ERROR("Unknown data arrived.");
			}
			else
			{
				process_yuv_or_rgb(raw->data, raw->used, raw->startTime, raw->stopTime, raw->format,
					raw->width, raw->height);
			}

			m_media_box->end_raw_video_data(raw);
		}

		ret = WaitForMultipleObjects(2, signals, FALSE, INFINITE);
	}
}

bool CmmcapturerDlg::decode_video(const AVCodecID& codecID, unsigned char *data,
	size_t size, REFERENCE_TIME startTime, REFERENCE_TIME stopTime)
{
	if (!m_ffmpeg_decoder->is_initialized() || !m_ffmpeg_decoder->validate(codecID))
	{
		bool ret = m_ffmpeg_decoder->init(codecID);
		if (!ret)
		{
			return false;
		}
	}

	AVFrame* frame;
	VideoFrameRender render;
	if (m_ffmpeg_decoder->send_video_data(data, size, startTime))
	{
		while ((frame = m_ffmpeg_decoder->receive_frame()) != NULL)
		{
			bool ret = render.InitFromFFmpeg(frame);
			if (!ret)
			{
				return false;
			}

			render.width = frame->width;
			render.height = frame->height;
			render.timestamp = frame->pts * 100;
			render.ffmpegFormat = frame->format;

			render_video_frame(render);
		}
	}
}

bool CmmcapturerDlg::process_yuv_or_rgb(unsigned char *data,
	size_t size, REFERENCE_TIME startTime, REFERENCE_TIME stopTime,
	VideoFormat format, int width, int height)
{
	VideoFrameRender render;
	AVFrame* frame;
	bool ret;

	AVPixelFormat pixelFmt = AV_PIX_FMT_NONE;
	for (int i = 0; i < sizeof(VIDEO_FORMAT_MAP) / sizeof(VIDEO_FORMAT_ENTRY); i++)
	{
		if (format == VIDEO_FORMAT_MAP[i].videoFormat)
		{
			pixelFmt = VIDEO_FORMAT_MAP[i].ffmpegFormat;
			break;
		}
	}

	if (AV_PIX_FMT_NONE == pixelFmt)
	{
		LOG_ERROR("Unsupported pixel format");
		return false;
	}

	ret = render.InitFrom(data, size, format, width, height);
	if (ret)
	{
		if (AV_PIX_FMT_YUV420P != pixelFmt)
		{
			ret = m_ffmpeg_transcoder->scale_yuv((uint8_t*)render.data, render.linesize, width, height, pixelFmt, &frame);
			bool ret = render.InitFromFFmpeg(frame);
			if (!ret)
			{
				return false;
			}
		}
	}
	if (!ret)
	{
		return false;
	}

	render.width = width;
	render.height = height;
	render.timestamp = startTime * 100;
	render.ffmpegFormat = AV_PIX_FMT_YUV420P;

	render_video_frame(render);
	return true;
}

bool CmmcapturerDlg::render_video_frame(VideoFrameRender& render)
{
	if (!m_sdl_video_initialized)
	{
		if (init_sdl_video_context(render))
		{
			m_sdl_video_initialized = true;
			sdl_render_video(render);
		}
	}
	else
	{
		sdl_render_video(render);
	}

	return true;
}

void CmmcapturerDlg::sdl_render_video(VideoFrameRender& render)
{
	int ret = SDL_UpdateYUVTexture(m_sdl_texture, NULL, render.data[0], render.linesize[0],
		render.data[1], render.linesize[1],
		render.data[2], render.linesize[2]);

	if (0 == ret)
	{
		if (0 == SDL_RenderClear(m_sdl_renderer))
		{
			if (0 == SDL_RenderCopy(m_sdl_renderer, m_sdl_texture, NULL, NULL))
			{
				SDL_RenderPresent(m_sdl_renderer);
			}
			else
			{
				LOG_ERROR("SDL_RenderCopy fail");
			}
		}
		else
		{
			LOG_ERROR("SDL_RenderClear fail");
		}
	}
	else
	{
		LOG_ERROR("SDL_UpdateTexture fail, error:%s", SDL_GetError());
	}
}

bool CmmcapturerDlg::init_sdl_video_context(VideoFrameRender& render)
{
	m_sdl_window = SDL_CreateWindowFrom(GetDlgItem(IDC_STATIC_PREVIEW)->GetSafeHwnd());
	if (!m_sdl_window)
	{
		LOG_ERROR("SDL: could not create window, errpr:%s", SDL_GetError());
		return false;
	}

	m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!m_sdl_renderer)
	{
		LOG_ERROR("SDL: could not create renderer, eror:%s", SDL_GetError());
		return false;
	}

	m_sdl_texture = SDL_CreateTexture(m_sdl_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, render.width, render.height);
	if (!m_sdl_texture)
	{
		LOG_ERROR("SDL: could not create texture[width:%d, height:%d], error:%s", render.width, render.height, SDL_GetError());
		return false;
	}

	return true;
}

DWORD WINAPI CmmcapturerDlg::render_audio_thread(LPVOID ptr)
{
	LOG_INFO("Render audio thread starts........");

	CmmcapturerDlg* dlg = (CmmcapturerDlg*)ptr;
	dlg->render_audio_run();

	LOG_INFO("Render audio thread ends........");

	return 0;
}

void CmmcapturerDlg::render_audio_run()
{
	HANDLE signals[2] = { m_render_audio_event_handle, m_stop_audio_event_handle };
	DWORD ret = WaitForMultipleObjects(2, signals, FALSE, INFINITE);
	while (ret == WAIT_OBJECT_0)
	{
		while (AudioRawData* raw = m_media_box->begin_raw_audio_data())
		{
			if (!render_audio_data(raw))
			{
				SDL_CloseAudio();
				return;
			}
			m_media_box->end_raw_audio_data(raw);
		}
		ret = WaitForMultipleObjects(2, signals, FALSE, INFINITE);
	}

	SDL_CloseAudio();
}

bool CmmcapturerDlg::render_audio_data(AudioRawData* raw)
{
	if (!m_sdl_audio_initialized)
	{
		if (init_sdl_audio(raw))
		{
			m_sdl_audio_initialized = true;
		}
		else
		{
			return false;
		}
	}

	if (-1 == SDL_QueueAudio(m_audio_device_id, raw->data, raw->used))
	{
		LOG_ERROR("SDL_QueueAudio error");
		return false;
	}

	return true;
}

bool CmmcapturerDlg::init_sdl_audio(AudioRawData* raw)
{
	if (raw->format != AudioFormat::Wave16bit &&
		raw->format != AudioFormat::WaveFloat)
	{
		LOG_ERROR("Unsupported audio format.");
		return false;
	}

	SDL_AudioSpec desired;
	desired.freq = raw->sampleRate;
	desired.format = raw->format == AudioFormat::Wave16bit ? AUDIO_S16SYS : AUDIO_F32SYS;
	desired.channels = raw->channels;
	desired.silence = 0;
	desired.samples = 2048;
	desired.callback = NULL;

	if ((m_audio_device_id = SDL_OpenAudioDevice(NULL, 0, &desired, NULL, 0)) < 2)
	{
		LOG_ERROR("SDL_OpenAudioDevice error.");
		return false;
	}

	SDL_PauseAudioDevice(m_audio_device_id, 0);

	return true;
}

void CmmcapturerDlg::on_video_sample()
{
	SetEvent(m_render_video_event_handle);
}

void CmmcapturerDlg::on_audio_sample()
{
	SetEvent(m_render_audio_event_handle);
}

void CmmcapturerDlg::play_video_audio()
{
	int vsel = m_combo_videos.GetCurSel();
	if (vsel < 0)
	{
		return;
	}

	int asel = m_combo_audios.GetCurSel();
	if (asel < 0)
	{
		return;
	}
	
	m_render_video_thread_handle = CreateThread(NULL, NULL, CmmcapturerDlg::render_video_thread, this, NULL, NULL);
	if (!m_render_video_thread_handle)
	{
		LOG_ERROR("Start render video thread error.");
		return;
	}

	m_render_audio_thread_handle = CreateThread(NULL, NULL, CmmcapturerDlg::render_audio_thread, this, NULL, NULL);
	if (!m_render_audio_thread_handle)
	{
		LOG_ERROR("Start render audio thread error.");
		return;
	}

	std::map<std::string, std::wstring> videoConfigs;
	videoConfigs["name"] = m_videos_vec[vsel].namew;
	videoConfigs["path"] = m_videos_vec[vsel].pathw;

	std::map<std::string, std::wstring> audioConfigs;
	audioConfigs["name"] = m_audios_vec[asel].namew;
	audioConfigs["path"] = m_audios_vec[asel].pathw;

	m_media_box->set_video_config(videoConfigs);
	m_media_box->set_audio_config(audioConfigs);
	m_media_box->queue_command(Command::ActivateAll);
}

void CmmcapturerDlg::play_video()
{
	int vsel = m_combo_videos.GetCurSel();
	if (vsel < 0)
	{
		return;
	}

	m_render_video_thread_handle = CreateThread(NULL, NULL, CmmcapturerDlg::render_video_thread, this, NULL, NULL);
	if (!m_render_video_thread_handle)
	{
		LOG_ERROR("Start render video thread error.");
		return;
	}

	std::map<std::string, std::wstring> videoConfigs;
	videoConfigs["name"] = m_videos_vec[vsel].namew;
	videoConfigs["path"] = m_videos_vec[vsel].pathw;

	m_media_box->set_video_config(videoConfigs);
	m_media_box->queue_command(Command::ActivateVideo);
}

void CmmcapturerDlg::play_audio()
{
	int asel = m_combo_audios.GetCurSel();
	if (asel < 0)
	{
		return;
	}

	m_render_audio_thread_handle = CreateThread(NULL, NULL, CmmcapturerDlg::render_audio_thread, this, NULL, NULL);
	if (!m_render_audio_thread_handle)
	{
		LOG_ERROR("Start render audio thread error.");
		return;
	}

	std::map<std::string, std::wstring> audioConfigs;
	audioConfigs["name"] = m_audios_vec[asel].namew;
	audioConfigs["path"] = m_audios_vec[asel].pathw;

	m_media_box->set_audio_config(audioConfigs);
	m_media_box->queue_command(Command::ActivateAudio);
}

void CmmcapturerDlg::OnBnClickedRadioPlayType()
{
	UpdateData(TRUE);
}

void CmmcapturerDlg::OnBnClickedBtnQuery()
{
	LOG_INFO("begin to query video/audio devices.......");

	m_combo_videos.ResetContent();
	//query video devices
	HRESULT hr = EnumAllVideoDevices(m_videos_vec);
	if (FAILED(hr))
	{
		LOG_ERROR("query video devices error.");
		return;
	}

	LOG_INFO("[%d] video devices found", m_videos_vec.size());
	for (int i = 0; i < m_videos_vec.size(); i++)
	{
		m_videos_vec[i].ConvertString();

		LOG_INFO("Video Device[%d] name:[%s], path:[%s]", i, m_videos_vec[i].name.c_str(), m_videos_vec[i].path.c_str());
		m_combo_videos.AddString(_T(m_videos_vec[i].name.c_str()));
	}
	m_combo_videos.SetCurSel(0);

	//query audio devices
	m_combo_audios.ResetContent();
	hr = EnumAllAudioInputDevices(m_audios_vec);
	if (FAILED(hr))
	{
		LOG_ERROR("query audio devices error.");
		return;
	}

	LOG_INFO("[%d] audio input devices found", m_audios_vec.size());
	for (int i = 0; i < m_audios_vec.size(); i++)
	{
		m_audios_vec[i].ConvertString();
		if (is_utf8_str(m_audios_vec[i].name))
		{
			LOG_INFO("Audio Device[%d] name:[%s], path:[%s]", i, utf8_to_gbk(m_audios_vec[i].name).c_str(), m_audios_vec[i].path.c_str());
			m_combo_audios.AddString(_T(utf8_to_gbk(m_audios_vec[i].name).c_str()));
		}
		else
		{
			m_combo_audios.AddString(_T(m_audios_vec[i].name).c_str());
		}
	}
	m_combo_audios.SetCurSel(0);
}


void CmmcapturerDlg::OnBnClickedBtnPlay()
{
	if (m_render_video_thread_handle || m_render_audio_thread_handle || m_has_error)
	{
		return;
	}

	switch (m_radio_play_type)
	{
	case 0:  //video and audio
		play_video_audio();
		break;

	case 1:  //video
		play_video();
		break;

	case 2:  //audio
		play_audio();
		break;
	}
}


void CmmcapturerDlg::OnBnClickedBtnStop()
{
	if (!(m_render_video_thread_handle || m_render_audio_thread_handle) || m_has_error)
	{
		return;
	}

	if (m_media_box)
	{
		m_media_box->queue_command(Command::DeactivateAll);
	}

	if (m_render_video_thread_handle)
	{
		SetEvent(m_stop_video_event_handle);
		WaitForSingleObject(m_render_video_thread_handle, INFINITE);
		CloseHandle(m_render_video_thread_handle);
		m_render_video_thread_handle = NULL;
	}

	if (m_render_audio_thread_handle)
	{
		SetEvent(m_stop_audio_event_handle);
		WaitForSingleObject(m_render_audio_thread_handle, INFINITE);
		CloseHandle(m_render_audio_thread_handle);
		m_render_audio_thread_handle = NULL;
	}

	if (m_sdl_texture)
	{
		SDL_DestroyTexture(m_sdl_texture);
		m_sdl_texture = NULL;
	}

	if (m_sdl_renderer)
	{
		SDL_DestroyRenderer(m_sdl_renderer);
		m_sdl_renderer = NULL;
	}

	if (m_sdl_window)
	{
		SDL_DestroyWindow(m_sdl_window);
		m_sdl_window = NULL;
	}

	m_sdl_video_initialized = false;
	m_sdl_audio_initialized = false;
	GetDlgItem(IDC_STATIC_PREVIEW)->ShowWindow(SW_SHOWNORMAL);
}

void CmmcapturerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	OnBnClickedBtnStop();

	if (m_media_box)
	{
		delete m_media_box;
		m_media_box = NULL;
	}

	if (m_ffmpeg_decoder)
	{
		delete m_ffmpeg_decoder;
		m_ffmpeg_decoder = NULL;
	}

	if (m_ffmpeg_transcoder)
	{
		delete m_ffmpeg_transcoder;
		m_ffmpeg_transcoder = NULL;
	}

	SDL_Quit();
}
