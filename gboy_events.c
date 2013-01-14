#include "gboy.h"

long
proc_evts()
{
	static SDL_Event evnt;

	while (SDL_PollEvent(&evnt)) {
		switch (evnt.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				return (joy_event(&evnt.key, evnt.type));
		}
	}
}
