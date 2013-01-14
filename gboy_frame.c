#include "gboy.h"
#define USC 1000000
#define FRAME_RATE 60

/* global variable settable through user interface */
int frames_per_second=60;

static int num_skips=0;
static Uint8 frm_cnt=0;
static struct timeval base={0, 0};
static struct timeval fps={0, 0};
static struct timeval ref={0, 0};
static useconds_t u_base;
static useconds_t u_fps;
static useconds_t u_ref;
static useconds_t u_frm=1000000/60; // microseconds per frame

void
frame_reset()
{
	base.tv_sec=0;
	base.tv_usec=0;
}

int
frame_skip()
{
	static int frame_rate_cnt=FRAME_RATE;

	if (base.tv_sec==0) {
		gettimeofday(&base, NULL);
		u_base=(base.tv_sec*USC)+base.tv_usec;
		frm_cnt=0;
		return 0;
	}

	u_base+=u_frm;

	if (num_skips>=10)
	{
		num_skips=0;
		frame_reset();
		return 1;
	}

	if (num_skips) {
		num_skips--;
		return 1;
	}

	gettimeofday(&ref, NULL);
	u_ref=(ref.tv_sec*USC)+ref.tv_usec;

	/* if not there yet */
	if (u_ref<u_base) {
		usleep((u_base-u_ref));
		if ((frame_rate_cnt-=frames_per_second)<=0)
		{
			frame_rate_cnt+=FRAME_RATE;
			frm_cnt++;
			return 0;
		}
		else
			return 1;
	}
	else {
		num_skips=(u_ref-u_base)/u_frm;
		return 0;
	}
}
