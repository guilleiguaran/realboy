#include <math.h>
#include "gboy.h"

Uint32 scale=1;
Uint32 anti_alias=0;
SDL_Surface *x1;
SDL_Surface *x2;
SDL_Surface *x3;
SDL_Surface *x4;
SDL_Surface *back1;
SDL_Surface *zoomS;

typedef struct tColorRGBA {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} tColorRGBA;

typedef struct {
  Sint16 x,y;
  Uint8 xoff,yoff;
  Uint8 sizey;
  Uint8 xflip,yflip;
  Uint8 page,pal_col;
  Uint16 tile_num;
  Uint8 pal;
  Uint8 priority;
} GB_SPRITE;
GB_SPRITE gb_spr[40];

Uint8 back_col [170][170];

SDL_Surface *_zoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int flipx, int flipy, int smooth)
{
	static int sax[160*5];
	static int say[160*5];

	int x, y, sx, sy, ssx, ssy, *csax, *csay, *salast, csx, csy, ex, ey, cx, cy, sstep, sstepx, sstepy;
	tColorRGBA *c00, *c01, *c10, *c11;
	tColorRGBA *sp, *csp, *dp;
	int spixelgap, spixelw, spixelh, dgap, t1, t2;

	/*
	* Precalculate row increments 
	*/
	spixelw = (src->w - 1);
	spixelh = (src->h - 1);
	if (smooth) {
		//sx = (int) (65536.0 * (float) spixelw / (float) (dst->w - 1));
		//sy = (int) (65536.0 * (float) spixelh / (float) (dst->h - 1));
		sx = (65536 *  spixelw /  (dst->w - 1));
		sy = (65536 *  spixelh /  (dst->h - 1));
	} else {
		//sx = (int) (65536.0 * (float) (src->w) / (float) (dst->w));
		//sy = (int) (65536.0 * (float) (src->h) / (float) (dst->h));
		sx =  (65536 *  (src->w) /  (dst->w));
		sy =  (65536 *  (src->h) /  (dst->h));
	}

	/* Maximum scaled source size */
	ssx = (src->w << 16) - 1;
	ssy = (src->h << 16) - 1;

	/* Precalculate horizontal row increments */
	csx = 0;
	csax = sax;
	for (x = 0; x <= dst->w; x++) {
		*csax = csx;
		csax++;
		csx += sx;

		/* Guard from overflows */
		if (csx > ssx) { 
			csx = ssx; 
		}
	}

	/* Precalculate vertical row increments */
	csy = 0;
	csay = say;
	for (y = 0; y <= dst->h; y++) {
		*csay = csy;
		csay++;
		csy += sy;

		/* Guard from overflows */
		if (csy > ssy) {
			csy = ssy;
		}
	}

	sp = (tColorRGBA *) src->pixels;
	dp = (tColorRGBA *) dst->pixels;
	dgap = dst->pitch - dst->w * 4;
	spixelgap = src->pitch/4;

	if (flipx) sp += spixelw;
	if (flipy) sp += (spixelgap * spixelh);

	/*
	* Switch between interpolating and non-interpolating code 
	*/
	if (smooth) {

		/*
		* Interpolating Zoom 
		*/
		csay = say;
		for (y = 0; y < dst->h; y++) {
			csp = sp;
			csax = sax;
			for (x = 0; x < dst->w; x++) {
				/*
				* Setup color source pointers 
				*/
				ex = (*csax & 0xffff);
				ey = (*csay & 0xffff);
				cx = (*csax >> 16);
				cy = (*csay >> 16);
				sstepx = cx < spixelw;
				sstepy = cy < spixelh;
				c00 = sp;
				c01 = sp;
				c10 = sp;
				if (sstepy) {
					if (flipy) {
						c10 -= spixelgap;
					} else {
						c10 += spixelgap;
					}
				}
				c11 = c10;
				if (sstepx) {
					if (flipx) {
						c01--;
						c11--;
					} else {
						c01++;
						c11++;
					}
				}

				/*
				* Draw and interpolate colors 
				*/
				t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
				t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
				dp->r = (((t2 - t1) * ey) >> 16) + t1;
				t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
				t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
				dp->g = (((t2 - t1) * ey) >> 16) + t1;
				t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
				t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
				dp->b = (((t2 - t1) * ey) >> 16) + t1;
				t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff;
				t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff;
				dp->a = (((t2 - t1) * ey) >> 16) + t1;				
				/*
				* Advance source pointer x
				*/
				salast = csax;
				csax++;				
				sstep = (*csax >> 16) - (*salast >> 16);
				if (flipx) {
					sp -= sstep;
				} else {
					sp += sstep;
				}

				/*
				* Advance destination pointer x
				*/
				dp++;
			}
			/*
			* Advance source pointer y
			*/
			salast = csay;
			csay++;
			sstep = (*csay >> 16) - (*salast >> 16);
			sstep *= spixelgap;
			if (flipy) { 
				sp = csp - sstep;
			} else {
				sp = csp + sstep;
			}

			/*
			* Advance destination pointer y
			*/
			dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
		}
	} else {
		/*
		* Non-Interpolating Zoom 
		*/		
		csay = say;
		for (y = 0; y < dst->h; y++) {
			csp = sp;
			csax = sax;
			for (x = 0; x < dst->w; x++) {
				/*
				* Draw 
				*/
				*dp = *sp;

				/*
				* Advance source pointer x
				*/
				salast = csax;
				csax++;				
				sstep = (*csax >> 16) - (*salast >> 16);
				if (flipx) sstep = -sstep;
				sp += sstep;

				/*
				* Advance destination pointer x
				*/
				dp++;
			}
			/*
			* Advance source pointer y
			*/
			salast = csay;
			csay++;
			sstep = (*csay >> 16) - (*salast >> 16);
			sstep *= spixelgap;
			if (flipy) sstep = -sstep;			
			sp = csp + sstep;

			/*
			* Advance destination pointer y
			*/
			dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
		}
	}

	return (dst);
}
SDL_Surface *zoomSurface(SDL_Surface *rz_dst, SDL_Surface * src, int zoomx, int zoomy, int smooth)
{
	int dstwidth, dstheight;
	int is32bit;
	int i, src_converted;
	int flipx, flipy;

	/*
	* Sanity check 
	*/
	if (src == NULL)
		return (NULL);

	/*
	* New source surface is 32bit with a defined RGBA ordering 
	*/
	//SDL_BlitSurface(src, NULL, rz_src, NULL);
	//rz_src=src;

	/*
	* Lock source surface 
	*/
//if (SDL_MUSTLOCK(src)) {
//	SDL_LockSurface(src);
//}

	/*
	* Call the 32bit transformation routine to do the zooming (using alpha) 
	*/
	_zoomSurfaceRGBA(src, rz_dst, 0, 0, smooth);
	/*
	* Unlock source surface 
	*/
//if (SDL_MUSTLOCK(src)) {
//	SDL_UnlockSurface(src);
//}

	/*
	* Return destination surface 
	*/
	return (rz_dst);
}

void
frame_update()
{
	static SDL_Surface *temp=NULL;

	switch (scale) {
		case 1:
			temp = x1;
			break;
		case 2:
			temp = x2;
			break;
		case 3:
			temp = x3;
			break;
		case 4:
			temp = x4;
			break;
	}
	if (temp==NULL)
		while (1) ;
	if (temp==x1)
		SDL_BlitSurface(back, NULL, screen, NULL);
	else {
		back1 = _zoomSurfaceRGBA(back, temp, 0, 0, anti_alias);
		SDL_BlitSurface(back1, NULL, screen, NULL);
	}
  SDL_Flip(screen);
}

void
render_win(unsigned int *buf)
{
	unsigned char *tile_map, *tile_ptr, c, shftr;
	short y, x, i, x1=0;
	short tile_num;
	
	if (((unsigned char)addr_sp[0xff4b])>= 166)
		return;
	
	if (addr_sp[0xff44]>= addr_sp[0xff4a]) {
		/* pointer to map */
		if (addr_sp[0xff40]&0x40)
			tile_map=&addr_sp[0x9c00];
		else
			tile_map=&addr_sp[0x9800];
		/* point to correct row of 32 tiles in the tile map */
		tile_map +=(((addr_sp[0xff44]-addr_sp[0xff4a])>>3)<<5);
		/* offset to correct row of 8 pixels in tile */
		y = (((addr_sp[0xff44]-addr_sp[0xff4a])&0x7)<<1);
		if ((addr_sp[0xff4b]-7)<0) x = 0;
		else x=addr_sp[0xff4b]-7;
		for (i=0; x<160; x+=8, i++) {
			tile_num = tile_map[i];
			if (!(addr_sp[0xff40]&0x10))
				tile_num=256+(signed char)tile_num;
			/* point to correct tile */
			tile_ptr = &addr_sp[0x8000+(tile_num<<4)];
			/* point to correct row of 8 pixels within tile */
			tile_ptr += y;
			for (shftr=7, x1=0; x1<8 && (x+x1)<160; x1++, shftr--) {
				c = ((tile_ptr[0]>>shftr)&1)|((((tile_ptr[1]>>shftr))&1)<<1);
				buf[x+x1] = grey[(addr_sp[0xff47]>>(c<<1))&3];
				back_col[x+x1][addr_sp[0xff44]]=c;
			}
		}
	}
}

void
render_spr(unsigned int *buf, GB_SPRITE *sp)
{
	unsigned char *tile_ptr;
	unsigned char nx;
	unsigned char wbit,c;
	
	/* pointer to tile */
	tile_ptr=&addr_sp[0x8000+(sp->tile_num<<4)];
	
	/* pointer to row in tile */
	if (!sp->yflip) tile_ptr+=((sp->yoff)<<1);
	else tile_ptr+=(sp->sizey-1-sp->yoff)<<1;
	
	for(nx=sp->xoff;nx<8;nx++) {
	  if (!sp->xflip) wbit=nx;
	  else wbit=7-nx;	
		wbit=((Uint8)(~wbit))%8; // shift factor
		c = ((tile_ptr[0]>>wbit)&1)|((((tile_ptr[1]>>wbit))&1)<<1);
	  if (c) {
			if (!(sp->priority)) buf[sp->x+nx] = grey[(addr_sp[0xff48+sp->pal]>>(c<<1))&3];
			else if (!(back_col[sp->x+nx][addr_sp[0xff44]]))
				buf[sp->x+nx] = grey[(addr_sp[0xff48+sp->pal]>>(c<<1))&3];
		}
	}
}

void
get_nb_spr()
{
	unsigned char *oam_ptr = &addr_sp[0xfe00];
	short tile_num, x, y, attr;
	unsigned char sizey;
	unsigned char i, yoff, xoff;

	nb_spr = 0;
	if ((!(addr_sp[0xff40]&2)) )
		return;

	if (addr_sp[0xff40]&4)
		sizey = 16;
	else
		sizey = 8;

	for (i=0;i<40;i++) {
		y=*oam_ptr;
		x=*(oam_ptr+1);
		tile_num=*(oam_ptr+2);
		attr=*(oam_ptr+3);
		oam_ptr += 4;
		y-=16;
		yoff=addr_sp[0xff44]-y;
		
		if ((addr_sp[0xff44]>=y) && (yoff<sizey) && (((x-8)<160))) {
			if (x<8)
				xoff=8-x;
			else
				xoff=0;
			gb_spr[nb_spr].sizey=sizey;
			gb_spr[nb_spr].x=x-8;
			gb_spr[nb_spr].y=y;
			gb_spr[nb_spr].xoff=xoff;
			gb_spr[nb_spr].yoff=yoff;
			gb_spr[nb_spr].xflip=(attr&0x20)>>5;
			gb_spr[nb_spr].yflip=(attr&0x40)>>6;
			gb_spr[nb_spr].pal_col=(attr&0x07);
			if (attr&0x10) gb_spr[nb_spr].pal=1;
			else gb_spr[nb_spr].pal=0;
			gb_spr[nb_spr].page=(attr&0x08)>>3;
			gb_spr[nb_spr].priority=(attr&0x80);
			if (sizey==16) gb_spr[nb_spr].tile_num=tile_num&0xfe;
			else  gb_spr[nb_spr].tile_num=tile_num;
  		 
			nb_spr++;
			/* Pas plus de 10 sinon la pause de eur_zld deconne */
			if (nb_spr>10) {
				nb_spr=10;
				return;
			}
		}
	}
}

void
render_back(unsigned int *buf)
{
	int i, j;
	unsigned char *ptr_data; // pointer to tiles
	unsigned char *ptr_map; // pointer to the tile map
	Uint8 indx, shftr, x, y, x1;
	Sint16 tile_num;
	
	/* point to tile map */
	if (addr_sp[0xff40]&0x8)
		ptr_map=addr_sp+0x9c00;
	else
		ptr_map=addr_sp+0x9800;
	
	/* get current line + SCROLL Y */
	y = addr_sp[0xff44]+addr_sp[0xff42];
	/* get SCROLL X */
	x = addr_sp[0xff43];
	x1 = x>>3;
	
	/* point to correct line of 32x32 tiles in tile map */
	ptr_map += ((y>>3)<<5)&0x3ff;
	
	/* get tile number */
	tile_num = ptr_map[x1&0x1f];
	x1++;
	
	if (!(addr_sp[0xff40]&0x10))
		tile_num = 256 + (signed char)tile_num;
	/* point to tile */
	ptr_data = addr_sp+0x8000+(tile_num<<4);
	
	/* point to correct bit row */
	ptr_data+=(y&7)<<1;
	
	x &= 7; // bit offset
	shftr=((Uint8)(~x))%8; // shift factor
	for (i=0; x<8; x++, shftr--) {
		indx = ((ptr_data[0]>>shftr)&1)|((((ptr_data[1]>>shftr))&1)<<1);
		buf[i] = grey[(addr_sp[0xff47]>>(indx<<1))&3];
		back_col[i][addr_sp[0xff44]]=indx;
		i++;
	}
	
	x=i;
	for (; x<160; x+=8) {
		tile_num = ptr_map[x1++&0x1f];
		if (!(addr_sp[0xff40]&0x10))
			tile_num = 256 + (signed char)tile_num;
		ptr_data = addr_sp+0x8000+(tile_num<<4);
		ptr_data+=(y&7)<<1;
		for (shftr=7, j=0; j<8 && (x+j)<160; shftr--, j++) {
			indx = ((ptr_data[0]>>shftr)&1)|((((ptr_data[1]>>shftr))&1)<<1);
			buf[i] = grey[(addr_sp[0xff47]>>(indx<<1))&3];
			back_col[i][addr_sp[0xff44]]=indx;
			i++;
		}
	}
}

void
render_scanline(long skip)
{
	if (skip)
		return;

	unsigned int *buf=(unsigned int *)back->pixels+((unsigned char)addr_sp[0xff44]*(back->pitch>>2));
	int i;

	if ((addr_sp[0xff40]&1))
		render_back(buf);
	if ((addr_sp[0xff40]&0x20))
		render_win(buf);

	if (addr_sp[0xff40]&2) {
		for (i=nb_spr-1; i>=0; i--)
			render_spr(buf, &gb_spr[i]);
	}

}
