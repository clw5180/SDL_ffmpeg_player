### 1、说明
本代码原作者：雷霄骅。向雷神致敬！  

在SDL游戏开发的过程中，我在SDL窗口中播放视频动画这个环节遇到了前所未有的障碍，在网上搜了很多教程，仍一无所获；万幸，我遇到了雷神，让我可以怀揣游戏开发的梦想继续前行。  

三个文件夹分别对应三个项目：  
* simplest_ffmpeg_decoder是一个FFmpeg解码器，实现了将视频文件(如xxx.mp4/mkv/avi/rmvb/flv等)解封装后得到的H.264等视频压缩文件转化为YUV格式的视频像素数据；  
* simplest_video_play_sdl2是一个SDL2编写的YUV格式的视频播放器，实现了将YUV格式的视频像素数据送到显卡并通过SDL创建的窗口显示出来；  
* simplest_ffmpeg_player_su是一个真正意义上的视频播放器，能够播放多种格式如.mp4/mkv/avi/rmvb/flv等，是将上述两个项目进行有机结合而生成的。  

我这里主要是修正了雷神上传版本的代码会造成的视频卡顿现象，同时在一些地方做了注释；  

本项目在Windows+VS2017+SDL2环境编译测试通过。  

在这里贴一下雷神的代码说明：  

```
/*  
 *最简单的基于FFmpeg的视频播放器2(SDL升级版)  
 *Simplest FFmpeg Player 2(SDL Update)  
 
 *雷霄骅 Lei Xiaohua  
 *leixiaohua1020@126.com  
 *中国传媒大学/数字电视技术  
 *Communication University of China / Digital TV Technology  
 *http://blog.csdn.net/leixiaohua1020  
 
 *第2版使用SDL2.0取代了第一版中的SDL1.2  
 *Version 2 use SDL 2.0 instead of SDL 1.2 in version 1.  
 
 *本程序实现了视频文件的解码和显示(支持HEVC，H.264，MPEG2等)。  
 *是最简单的FFmpeg视频解码方面的教程。  
 *通过学习本例子可以了解FFmpeg的解码流程。  
 *本版本中使用SDL消息机制刷新视频画面。  
 *This software is a simplest video player based on FFmpeg.  
 *Suitable for beginner of FFmpeg.  
 
 *备注:  
 *标准版在播放视频的时候，画面显示使用延时40ms的方式。这么做有两个后果：  
 *（1）SDL弹出的窗口无法移动，一直显示是忙碌状态  
 *（2）画面显示并不是严格的40ms一帧，因为还没有考虑解码的时间。  
 *SU（SDL Update）版在视频解码的过程中，不再使用延时40ms的方式，而是创建了  
 *一个线程，每隔40ms发送一个自定义的消息，告知主函数进行解码显示。这样做之后：  
 *（1）SDL弹出的窗口可以移动了  
 *（2）画面显示是严格的40ms一帧  
 *Remark:  
 *Standard Version use's SDL_Delay() to control video's frame rate, it has 2  
 *disadvantages:  
 *(1)SDL's Screen can't be moved and always "Busy".  
 *(2)Frame rate can't be accurate because it doesn't consider the time consumed  
 *by avcodec_decode_video2()  
 *SU（SDL Update）Version solved 2 problems above. It create a thread to send SDL  
 *Event every 40ms to tell the main loop to decode and show video frames.  
 */  
```
 
### 2、注意事项
由于原版程序是在VS2010的环境下调试的；我在使用VS2017调试的过程中遇到了几个问题，也在这里说一下相应的解决方法（ffmpeg, SDL）：

**（1）错误 LNK2019无法解析的外部符号 __imp__fprintf，该符号在函数 _ShowError 中被引用**  
解决方法：  
在程序中加入如下一行：  
#pragma comment(lib, "legacy_stdio_definitions.lib")
（或在链接器-》命令行 里加入legacy_stdio_definitions.lib）  
关于问题1的解释：  
stdio函数的其他链接错误 sprintf()，可以向链接器选项中添加 legacy_stdio_definitions.lib 。

**（2）错误 LNK2019无法解析的外部符号 __imp____iob_func，该符号在函数 _ShowError 中被引用**  
解决方法：  
加入如下一行：  
```
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; } 
```
关于问题2的解释：  
在visual studio 2015及以上版本中，stdin，stderr，stdout定义如下：
```
#define stdin（__acrt_iob_func（0））
#define stdout（__acrt_iob_func（1））
#define stderr（__acrt_iob_func（2））
```
但以前，它们被定义为：
```
#define stdin （& __ iob_func（）[0]）
#define stdout（& __ iob_func（）[1]）
#define stderr（& __ iob_func（）[2]）
```
所以现在没有定义__iob_func，导致在使用以前版本的visual studio编译的.lib文件时出现链接错误。

**（3）错误 LNK2019无法解析的外部符号 _main，该符号在函数 "int __cdecl invoke_main(void)" (?invoke_main@@YAHXZ) 中被引用**  
解决方法：  
1）程序中增加一行：#pragma comment(lib, "SDL2main.lib") // 程序与SDL有关，此方法未必有普适性，请具体问题具体对待
2）将main函数形式修改为：int main(int argc, char* argv[]) {...}


### 3、效果演示
**这里测试的是国产RPG巅峰之作，天地劫三部曲之《幽城幻剑录》的片头动画。**

![这里随便写文字](https://github.com/clw5180/SDL_ffmpeg_player/blob/master/screenshot/1.bmp)  
  

![这里随便写文字](https://github.com/clw5180/SDL_ffmpeg_player/blob/master/screenshot/2.bmp)  
  
  
![这里随便写文字](https://github.com/clw5180/SDL_ffmpeg_player/blob/master/screenshot/3.png)
