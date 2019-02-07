/**
 * 最简单的SDL2播放视频的例子（SDL2播放RGB/YUV）
 * Simplest Video Play SDL2 (SDL2 play RGB/YUV) 
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序使用SDL2播放RGB/YUV视频像素数据！！！！！！SDL实际上是对底层绘图
 * API（Direct3D，OpenGL）的封装，使用起来明显简单于直接调用底层
 * API。
 *
 * 函数调用步骤如下: 
 *
 * [初始化]
 * SDL_Init(): 初始化SDL。
 * SDL_CreateWindow(): 创建窗口（Window）。
 * SDL_CreateRenderer(): 基于窗口创建渲染器（Render）。
 * SDL_CreateTexture(): 创建纹理（Texture）。
 *
 * [循环渲染数据]
 * SDL_UpdateTexture(): 设置纹理的数据。
 * SDL_RenderCopy(): 纹理复制给渲染器。
 * SDL_RenderPresent(): 显示。
 *
 * This software plays RGB/YUV raw video data using SDL2.
 * SDL is a wrapper of low-level API (Direct3D, OpenGL).
 * Use SDL is much easier than directly call these low-level API.  
 *
 * The process is shown as follows:
 *
 * [Init]
 * SDL_Init(): Init SDL.
 * SDL_CreateWindow(): Create a Window.
 * SDL_CreateRenderer(): Create a Render.
 * SDL_CreateTexture(): Create a Texture.
 *
 * [Loop to Render data]
 * SDL_UpdateTexture(): Set Texture's data.
 * SDL_RenderCopy(): Copy Texture to Render.
 * SDL_RenderPresent(): Show.
 */

#include <stdio.h>

extern "C"
{
#include "sdl/SDL.h"
};


//clw modify 20190206
#pragma comment(lib, "legacy_stdio_definitions.lib")
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }


const int bpp=12;

int screen_w=500,screen_h=500;
const int pixel_w=320,pixel_h=180;

unsigned char buffer[pixel_w*pixel_h*bpp/8];

//Refresh Event
#define REFRESH_EVENT  (SDL_USEREVENT + 1)

#define BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit=0;

int refresh_video(void *opaque)
{
	thread_exit=0;
	while (!thread_exit) 
	{
		SDL_Event event;
		event.type = REFRESH_EVENT; //clw note：自己定义的事件
		SDL_PushEvent(&event);
		SDL_Delay(40); //clw note：主循环中不要有延时，而是放在子线程中、                      //          这样做的好处是可以拖动窗口或放大窗口，
		               //          窗口上不会是一个忙碌的圆圈状态；
		               //          个人认为是由于延时移到了子线程，因此
		               //          主线程可以抽身来响应其他的事件，比如用户
		               //          拖动窗口或者点鼠标，而不是一直卡在Delay中；
		               //         （事件的处理时间相比Delay40毫秒要快得多！！）
	}
	thread_exit=0;
	//Break
	SDL_Event event;
	event.type = BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}

int main(int argc, char* argv[])
{
	if(SDL_Init(SDL_INIT_VIDEO)) 
	{  
		printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
		return -1;
	} 

	SDL_Window *screen; 
	//SDL 2.0 Support for multiple windows
	screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!screen) 
	{  
		printf("SDL: could not create window - exiting:%s\n",SDL_GetError());  
		return -1;
	}
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  

	Uint32 pixformat=0;

	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	pixformat= SDL_PIXELFORMAT_IYUV;  

	SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer,pixformat, SDL_TEXTUREACCESS_STREAMING,pixel_w,pixel_h);

	FILE *fp=NULL;
	fp=fopen("test_yuv420p_320x180.yuv","rb+"); 
	//clw note：读.YUV视频
	//另外可以从下面网址下载这个.yuv文件
	//sourceforge.net/projects/simplestmediaplay/

	if(fp==NULL)
	{
		printf("cannot open this file\n");
		return -1;
	}

	SDL_Rect sdlRect;  


	/*************************************************************/
	/* clw note：注意这里创建线程，多线程！！
	/*************************************************************/
	SDL_Thread *refresh_thread = SDL_CreateThread(refresh_video,NULL,NULL);
	/*************************************************************/

	SDL_Event event;
	while(1)
	{
		//Wait
		SDL_WaitEvent(&event); //clw note：等待事件到来，其实就是等上面新开的线程执行到SDL_PushEvent(&event);
		if(event.type==REFRESH_EVENT)  //clw note：自己定义的事件
		{
			//clw note：把fp对应文件的内容读到buffer中
			if (fread(buffer, 1, pixel_w * pixel_h * bpp/8, fp) != pixel_w * pixel_h * bpp/8) 
				// clw note：bpp是一个常量为12,12/8=1.5，代表每个像素占了多少bit，除以8代表每个像素占了多少字节，因此fread会一次读取这么多字节，即读取一帧完整的YUV数据，一共是w*h*12/8这么多字节。
				// 另外，由于YUV的特点，Y数据的量是宽高乘积，U、Y的数据的量分别是1/2宽和1/2高的乘积，也就是1/4，因此UV总数据量相当于Y数据量的1/2。
				// 1+1/4+1/4=1.5，也就是每个像素占1.5字节。
			{
				// Loop
				fseek(fp, 0, SEEK_SET);
				fread(buffer, 1, pixel_w*pixel_h*bpp/8, fp);
			}

			SDL_UpdateTexture( sdlTexture, NULL, buffer, pixel_w);  

			//FIX: If window is resize
			sdlRect.x = 0;  
			sdlRect.y = 0;  
			sdlRect.w = screen_w;  
			sdlRect.h = screen_h;  
			
			SDL_RenderClear( sdlRenderer );   
			SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, &sdlRect);  
			SDL_RenderPresent( sdlRenderer );  	
		}

		else if(event.type==SDL_WINDOWEVENT) //clw note：SDL内部包含的事件，在操作窗口的时候触发
		{
			//If Resize
			SDL_GetWindowSize(screen,&screen_w,&screen_h); 
			//clw note：通过重新获得拉伸后的screen_w和screen_h然后再传入SDL_RenderCopy，这样可以起到类似拉伸视频播放窗口的效果！
		}

		else if(event.type==SDL_QUIT)
		{
			thread_exit=1;
		}
		
		else if(event.type==BREAK_EVENT)
		{
			break;
		}
	}
	SDL_Quit();
	return 0;
}
