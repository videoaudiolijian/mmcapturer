## Windows平台音视频流捕获软件
本软件捕获windows系统上的视频流和音频流，并进行渲染。

捕获音视频流，使用的是微软的DirectShow组件。
使用SDL2.0进行视频渲染和音频的播放。
解码使用的是FFmpeg库，64位版本。

IDE使用的Visual Studio 2015社区版。代码下载后直接编译即可运行。  
由于SDL2.0的库和FFmpeg库太大，使用者需要自行下载。  
该系统使用的具体的FFmpeg版本为：ffmpeg-4.4-full_build-shared。  
SDL版本为SDL2-2.0.14  
UI使用MFC进行绘制。

第三方库的目录为：  
mmcapturer/library/ffmpeg-4.4-full_build-shared  
mmcapturer/library/SDL2-2.0.14

软件截图:
![Image text](https://raw.githubusercontent.com/videoaudiolijian/mmcapturer/master/screenshot/screenshot.png)
