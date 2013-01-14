#include "gboy.h"
extern SDL_Surface *back;
extern SDL_Surface *zoomS;
extern SDL_Surface *x1;
extern SDL_Surface *x2;
extern SDL_Surface *x3;
extern SDL_Surface *x4;
extern Uint32 scale;
extern Uint32 anti_alias;

void
vid_reset()
{
	SDL_FreeSurface(x1);
	SDL_FreeSurface(x2);
	SDL_FreeSurface(x3);
	SDL_FreeSurface(x4);
	SDL_FreeSurface(zoomS);
	SDL_FreeSurface(back);
	SDL_FreeSurface(screen);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	scale=1;
	anti_alias=0;
}

void
vid_start()
{
	int flags;

	flags = 0; // XXX
	/* gngn */
  grey[0]=0xffffff;//COL32_TO_16(0xffffff);//0xB8A68D); //0xc618; // ffe6ce
  grey[1]=0x917d5e;//COL32_TO_16(0x917D5E); //0x8410; // bfad9a
  grey[2]=0x635030;//COL32_TO_16(0x635030); //0x4208; // 7f7367
  grey[3]=0x211a10;//COL32_TO_16(0x211A10); //0x0000; // 3f3933

	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
	screen = SDL_SetVideoMode(160, 144, 32, SDL_RESIZABLE);
  back = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 146, 32,
			   0, 0, 0, 0);

	SDL_ShowCursor(0);
	SDL_WM_SetCaption(gb_cart.cart_name, gb_cart.cart_name);
	x1 = SDL_CreateRGBSurface(SDL_SWSURFACE, 160, 144+2 , 32, 0, 0, 0, 0);
	x2 = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 288+3 , 32, 0, 0, 0, 0);
	x3 = SDL_CreateRGBSurface(SDL_SWSURFACE, 480, 432+5 , 32, 0, 0, 0, 0);
	x4 = SDL_CreateRGBSurface(SDL_SWSURFACE, 640, 576+7 , 32, 0, 0, 0, 0);
}
