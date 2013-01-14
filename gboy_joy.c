#include "gboy.h"
Uint32 key_bitmap;
extern Uint32 scale;
extern Uint32 fullscreen;
extern Uint32 anti_alias;
int fullscr=0;

long
joy_event(SDL_KeyboardEvent *key, Uint32 type)
{

	if (type == SDL_KEYDOWN) {
		switch (key->keysym.sym) {
			case SDLK_p:
				exit(0);
				break;
			case SDLK_RETURN:
				key_bitmap|=RET_MASK;
				break;
			case SDLK_d:
				key_bitmap|=D_MASK;
				break;
			case SDLK_s:
				key_bitmap|=S_MASK;
				break;
			case SDLK_a:
				key_bitmap|=A_MASK;
				break;
			case SDLK_UP:
				key_bitmap|=UP_MASK;
				break;
			case SDLK_DOWN:
				key_bitmap|=DOWN_MASK;
				break;
			case SDLK_LEFT:
				key_bitmap|=LEFT_MASK;
				break;
			case SDLK_RIGHT:
				key_bitmap|=RIGHT_MASK;
				break;
			case SDLK_1:
				screen = SDL_SetVideoMode(160, 144, 32,SDL_RESIZABLE|fullscr);
				scale = 1;
				break;
			case SDLK_2:
				screen = SDL_SetVideoMode(160*2, 144*2, 32,SDL_RESIZABLE|fullscr);
				scale = 2;
				break;
			case SDLK_3:
				screen = SDL_SetVideoMode(160*3, 144*3, 32,SDL_RESIZABLE|fullscr);
				scale = 3;
				break;
			case SDLK_4:
				screen = SDL_SetVideoMode(160*4, 144*4, 32,SDL_RESIZABLE|fullscr);
				scale = 4;
				break;
			case SDLK_5:
				if (anti_alias==0)
					anti_alias++;
				else
					anti_alias--;
				break;
			case SDLK_q:
				key_bitmap=0;
				return 1;
			case SDLK_g:
				gbddb=1;
				break;
			case SDLK_f:
				if (fullscr==SDL_FULLSCREEN)
				{
					fullscr=0;
					SDL_WM_ToggleFullScreen(screen);
					screen = SDL_SetVideoMode(160*scale, 144*scale, 32, SDL_RESIZABLE);
				}
				else
				{
					fullscr=SDL_FULLSCREEN;
					SDL_WM_ToggleFullScreen(screen);
					screen = SDL_SetVideoMode(160*scale, 144*scale, 32, SDL_RESIZABLE|fullscr);
				}
				break;
		}
	}
	else if (type == SDL_KEYUP) {
		switch (key->keysym.sym) {
			case SDLK_UP:
				key_bitmap&=~UP_MASK;
				break;
			case SDLK_DOWN:
				key_bitmap&=~DOWN_MASK;
				break;
			case SDLK_LEFT:
				key_bitmap&=~LEFT_MASK;
				break;
			case SDLK_RIGHT:
				key_bitmap&=~RIGHT_MASK;
				break;
			case SDLK_a:
				key_bitmap&=~A_MASK;
				break;
			case SDLK_s:
				key_bitmap&=~S_MASK;
				break;
			case SDLK_d:
				key_bitmap&=~D_MASK;
				break;
			case SDLK_RETURN:
				key_bitmap&=~RET_MASK;
				break;
		}
	}

	return 0;
}
