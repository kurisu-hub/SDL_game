//main函数冲突，防止冲突添加的define
#define SDL_MAIN_HANDLED

#include<iostream>

#include<SDL.h>
#include<SDL_image.h>
#include<SDL_mixer.h>
#include<SDL_ttf.h>
#include<SDL2_gfxPrimitives.h>
int main() {
	//初始化所有
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
//初始化音频库
	Mix_Init(MIX_INIT_MP3);
	//初始化字体
	TTF_Init();
	//声道设置
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	
	//创建窗口，其中title部分是u8文字，再就是传入的格式是否居中，再就是长宽大小，最后就是展示
	SDL_Window* window = SDL_CreateWindow("你好，世界", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);//渲染,第二个参数传-1自动帮你处理
	
	TTF_Font* font = TTF_OpenFont("ipix.ttf", 32);//字体选择
	SDL_Color color = { 255,255,255,255 };
	SDL_Surface*suf_text = TTF_RenderUTF8_Blended(font, "你好，世界", color);
	SDL_Texture* tex_text = SDL_CreateTextureFromSurface(renderer, suf_text);
	SDL_Rect rect_text;
	rect_text.h = suf_text->h;
	rect_text.w = suf_text->w;

	Mix_Music* music = Mix_LoadMUS("music.mp3");//音乐
	Mix_FadeInMusic(music, -1, 1500);
	

	SDL_Surface* suf_img = IMG_Load("avatar.jpg");//加载图片
	SDL_Texture* tex_img = SDL_CreateTextureFromSurface(renderer, suf_img);	//通过渲染器renderer进行渲染 
	SDL_Point pos_cursor = { 0,0 };//定义一个指针记录位置
	SDL_Rect rect_img;	//用于描述图片的参数
	//获取宽高
	rect_img.h = suf_img->h;
	rect_img.w = suf_img->w;

	int fps = 60;
	bool is_quit = false;
	SDL_Event event;
	Uint64 last_counter = SDL_GetPerformanceCounter();
	Uint64 counter_freq = SDL_GetPerformanceFrequency();
	while (!is_quit)
	{
		while (SDL_PollEvent(&event))
		{	//状态为关闭则退出循环
			if (event.type == SDL_QUIT) 
			{
				is_quit = true;
			}
			else if (event.type == SDL_MOUSEMOTION) 
			{	
				//鼠标移动更新位置数据
				pos_cursor.x = event.motion.x;
				pos_cursor.y = event.motion.y;
			}
		}
		Uint64 current_Counter = SDL_GetPerformanceCounter();
		double delta = (double)(current_Counter - last_counter) / counter_freq;
		last_counter = current_Counter;
		if (delta * 1000 < 1000.0 / 60) 
		{
			SDL_Delay((Uint32)(1000.0 / 60 - delta * 1000));
		}
		//处理数据
		rect_img.x = pos_cursor.x;
		rect_img.y = pos_cursor.y;
		rect_text.x = pos_cursor.x;
		rect_text.y = pos_cursor.y;
		//调色彩,255为描述其为纯黑
		SDL_SetRenderDrawColor(renderer,0,0,0,255);
		SDL_RenderClear(renderer);
		//渲染绘图,就是对图片进行绘画
		SDL_RenderCopy(renderer, tex_img, nullptr, &rect_img);
		filledCircleRGBA(renderer, pos_cursor.x, pos_cursor.y, 50, 255, 0, 0,125);
		SDL_RenderCopy(renderer, tex_text, nullptr, &rect_text);
		

		SDL_RenderPresent(renderer);
	}
	return 0;
}