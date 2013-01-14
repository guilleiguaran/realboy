/*
 * Routines for processing writes to io space (0xff00-0xffef).
 */
#include "gboy.h"

/* special assembly-exports declarations */
//extern char *addr_sp;
extern long cpu_cur_mode;
extern long gb_hblank_clks;
extern long gb_vbln_clks;
extern long lcd_vbln_hbln_ctrl;
extern Uint32 key_bitmap;
extern long tac_counter;
extern long tac_reload;
extern long tac_on;
extern char addr_sp_ptrs;
extern void write_sound_reg(unsigned short,unsigned char);
extern unsigned char read_sound_reg(unsigned short);
extern Uint8 back_col[170][170];
extern long win_curline;

static void
cgb_speed_switch(Uint8 val)
{
	if (val&1)
		addr_sp[0xff4d] |= 1, cpu_cur_mode = 1; // set double-speed mode
	else
		addr_sp[0xff4d] &= 0xfe, cpu_cur_mode = 0; // set normal-speed mode
}

static void
do_dma(unsigned int val)
{
	void *ptr_addr_ptrs = (void *)&addr_sp_ptrs;
	char *ptr_src;
	char *ptr_dst;
	int val_offs, i;

	val &= 0xff;
	val <<= 8; // scale
	val_offs = (val>>9)&0x78; // offset to pointers
	ptr_addr_ptrs += val_offs; // get addr_sp_ptrs
	ptr_addr_ptrs = (void *)(*(long *)ptr_addr_ptrs);
	ptr_src = (char *)((long)ptr_addr_ptrs+val);
	ptr_dst = addr_sp + 0xfe00; // OAM

	/* copy stream */
	for (i=0; i<160; i++)
		ptr_dst[i] = ptr_src[i];
}
static void
joy_update(Uint8 reg_new, Uint8 reg_old)
{
	addr_sp[0xff00] = reg_new|0xcf;

	if (!(reg_new&0x10)) {
		if (key_bitmap&UP_MASK)
		{
			addr_sp[0xff00]&=~4;
		}
		if (key_bitmap&DOWN_MASK)
		{
			addr_sp[0xff00]&=~8;
		}
		if (key_bitmap&LEFT_MASK)
		{
			addr_sp[0xff00]&=~2;
		}
		if (key_bitmap&RIGHT_MASK)
		{
			addr_sp[0xff00]&=~1;
		}
	}
	
	if (!(reg_new&0x20)) {
		if (key_bitmap&A_MASK)
		{
			addr_sp[0xff00]&=~4;
		}
		if (key_bitmap&RET_MASK)
		{
			addr_sp[0xff00]&=~8;
		}
		if (key_bitmap&S_MASK)
		{
			addr_sp[0xff00]&=~2;
		}
		if (key_bitmap&D_MASK)
		{
			addr_sp[0xff00]&=(Uint8)(~1);
		}
	}
}

static void
div_reset()
{
	*(addr_sp+0xff04) = 0; // reset DIV
}

static void
tac_update(int new_val)
{
	switch (new_val&3) {
		case 0:
			tac_counter = 64;
			break;
		case 1:
			tac_counter = 1;
			break;
		case 2:
			tac_counter = 4;
			break;
		case 3:
			tac_counter = 16;
			break;
	}

	tac_reload=tac_counter;
	if (new_val&0x4)
		tac_on=1;
	else
		tac_on=0;
}

static void
disable_boot()
{
	lseek(rom_fd, 0, SEEK_SET);

	/* read to address space */
	read(rom_fd, addr_sp, 256);

	if (type==1)
		pread(rom_fd, addr_sp+0x100, 0x4000-0x100, 256);
}

static void
lcd_gen(int new_val)
{

}

static void
lcd_sptr(int new_val)
{
// if (lcd_sprt_enable==0)
// 	lcd_sprt_enable=1;
// else
// 	lcd_sprt_enable=0;
}

static void
lcd_bg_disp(int new_val)
{
}

static void
lcd_bg_wt_dat_sel(int new_val)
{
// if (lcd_bg_win_datsel==0x8800)
// 	lcd_bg_win_datsel=0x8000;
// else
// 	lcd_bg_win_datsel=0x8800;
}

static void
lcd_init(int new_val)
{
	Uint8 i, j;
	static int lcd_enable=0;
	long *ptr_vh;
	
	if (lcd_enable==0 && (new_val&0x80))
	{
		lcd_enable=1;
		if (addr_sp[0xff44] == addr_sp[0xff45])
			addr_sp[0xff41]|=4;
		/* this 'h-blk' is 80 cycles */
		gb_hblank_clks=80;
		addr_sp[0xff41] = (addr_sp[0xff41]&0xfc);
		/* start with mode 0 */
		//addr_sp[0xff44] = 0;
	}
	/* 
	 * LCD is being turned off; reinitialize all values.
	 * LCD can only be turned off during VBLANK.
	 */
	else if (lcd_enable==1 && (!(new_val&0x80)))
	{
		lcd_enable=0;
		addr_sp[0xff41] = 0xc0;
		addr_sp[0xff44] = 0x00;
		ptr_vh = &gb_vbln_clks;
		ptr_vh[0] = lcd_vbln_hbln_ctrl = ptr_vh[1];
		//SDL_FillRect(back, NULL, grey[0]);
		//write_tmp();
		//for (i=0; i<170; i++)
		//	for (j=0; j<170; j++)
		//		back_col[i][j]=0;
	}
}

void (*lcdc_fptrs[8])(int) = { lcd_bg_disp, lcd_gen, lcd_gen, lcd_gen, lcd_bg_wt_dat_sel, lcd_gen, lcd_gen, lcd_init };

static void
lcd_ctrl(int lcdc_new)
{
	int i=1, m=0, cmp_val;
	static int lcdc_last=0;

	/* iterate through bits */
	do {
		if ((lcdc_last & i) != (lcdc_new & i))
			lcdc_fptrs[m](lcdc_new&i);
		m++;
	} while ((i<<=1) < 0x100);

	lcdc_last=lcdc_new;
}

void
io_ctrl(Uint8 io_off, Uint8 io_new)
{
	char *ptr_reg;

	switch (io_off) {
		case 0x00:
			joy_update(io_new, addr_sp[0xff00]);
			return;
		case 0x01:
			break;
		case 0x02:
			break;
		case 0x03:
			break;
		case 0x04:
			div_reset();
			io_new=0;
			break;
		case 0x05:
			break;
		case 0x06:
			break;
		case 0x07:
			tac_update(io_new);
			break;
		case 0x08:
			break;
		case 0x09:
			break;
		case 0x0a:
			break;
		case 0x0b:
			break;
		case 0x0c:
			break;
		case 0x0d:
			break;
		case 0x0e:
			break;
		case 0x0f:
			break;
		case 0x10:
			write_sound_reg(io_off, io_new);
			return;
		case 0x11:
			write_sound_reg(io_off, io_new);
			return;
		case 0x12:
			write_sound_reg(io_off, io_new);
			return;
		case 0x13:
			write_sound_reg(io_off, io_new);
			return;
		case 0x14:
			write_sound_reg(io_off, io_new);
			return;
		case 0x15:
			write_sound_reg(io_off, io_new);
			return;
		case 0x16:
			write_sound_reg(io_off, io_new);
			return;
		case 0x17:
			write_sound_reg(io_off, io_new);
			return;
		case 0x18:
			write_sound_reg(io_off, io_new);
			return;
		case 0x19:
			write_sound_reg(io_off, io_new);
			return;
		case 0x1a:
			write_sound_reg(io_off, io_new);
			return;
		case 0x1b:
			write_sound_reg(io_off, io_new);
			return;
		case 0x1c:
			write_sound_reg(io_off, io_new);
			return;
		case 0x1d:
			write_sound_reg(io_off, io_new);
			return;
		case 0x1e:
			write_sound_reg(io_off, io_new);
			return;
		case 0x1f:
			write_sound_reg(io_off, io_new);
			return;
		case 0x20:
			write_sound_reg(io_off, io_new);
			return;
		case 0x21:
			write_sound_reg(io_off, io_new);
			return;
		case 0x22:
			write_sound_reg(io_off, io_new);
			return;
		case 0x23:
			write_sound_reg(io_off, io_new);
			return;
		case 0x24:
			write_sound_reg(io_off, io_new);
			return;
		case 0x25:
			write_sound_reg(io_off, io_new);
			return;
		case 0x26:
			write_sound_reg(io_off, io_new);
			return;
		case 0x27:
			write_sound_reg(io_off, io_new);
			return;
		case 0x28:
			write_sound_reg(io_off, io_new);
			return;
		case 0x29:
			write_sound_reg(io_off, io_new);
			return;
		case 0x2a:
			write_sound_reg(io_off, io_new);
			return;
		case 0x2b:
			write_sound_reg(io_off, io_new);
			return;
		case 0x2c:
			write_sound_reg(io_off, io_new);
			return;
		case 0x2d:
			write_sound_reg(io_off, io_new);
			return;
		case 0x2e:
			write_sound_reg(io_off, io_new);
			return;
		case 0x2f:
			write_sound_reg(io_off, io_new);
			return;
		case 0x30:
			write_sound_reg(io_off, io_new);
			return;
		case 0x31:
			write_sound_reg(io_off, io_new);
			return;
		case 0x32:
			write_sound_reg(io_off, io_new);
			return;
		case 0x33:
			write_sound_reg(io_off, io_new);
			return;
		case 0x34:
			write_sound_reg(io_off, io_new);
			return;
		case 0x35:
			write_sound_reg(io_off, io_new);
			return;
		case 0x36:
			write_sound_reg(io_off, io_new);
			return;
		case 0x37:
			write_sound_reg(io_off, io_new);
			return;
		case 0x38:
			write_sound_reg(io_off, io_new);
			return;
		case 0x39:
			write_sound_reg(io_off, io_new);
			return;
		case 0x3a:
			write_sound_reg(io_off, io_new);
			return;
		case 0x3b:
			write_sound_reg(io_off, io_new);
			return;
		case 0x3c:
			write_sound_reg(io_off, io_new);
			return;
		case 0x3d:
			write_sound_reg(io_off, io_new);
			return;
		case 0x3e:
			write_sound_reg(io_off, io_new);
			return;
		case 0x3f:
			write_sound_reg(io_off, io_new);
			return;
		case 0x40:
			lcd_ctrl(io_new);
			break;
		case 0x41:
			break;
		case 0x42:
			break;
		case 0x43:
			break;
		case 0x44:
			break;
		case 0x45:
			break;
		case 0x46:
			do_dma((unsigned int)io_new);
			break;
		case 0x47:
			break;
		case 0x48:
			break;
		case 0x49:
			break;
		case 0x4a:
			break;
		case 0x4b:
			break;
		case 0x4c:
			break;
		case 0x4d:
			cgb_speed_switch(io_new);
			break;
		case 0x4e:
			break;
		case 0x4f:
			break;
		case 0x50:
			disable_boot();
			break;
		case 0x51:
			break;
		case 0x52:
			break;
		case 0x53:
			break;
		case 0x54:
			break;
		case 0x55:
			break;
		case 0x56:
			break;
		case 0x57:
			break;
		case 0x58:
			break;
		case 0x59:
			break;
		case 0x5a:
			break;
		case 0x5b:
			break;
		case 0x5c:
			break;
		case 0x5d:
			break;
		case 0x5e:
			break;
		case 0x5f:
			break;
		case 0x60:
			break;
		case 0x61:
			break;
		case 0x62:
			break;
		case 0x63:
			break;
		case 0x64:
			break;
		case 0x65:
			break;
		case 0x66:
			break;
		case 0x67:
			break;
		case 0x68:
			break;
		case 0x69:
			break;
		case 0x6a:
			break;
		case 0x6b:
			break;
		case 0x6c:
			break;
		case 0x6d:
			break;
		case 0x6e:
			break;
		case 0x6f:
			break;
		case 0x70:
			break;
		case 0x71:
			break;
		case 0x72:
			break;
		case 0x73:
			break;
		case 0x74:
			break;
		case 0x75:
			break;
		case 0x76:
			break;
		case 0x77:
			break;
		case 0x78:
			break;
		case 0x79:
			break;
		case 0x7a:
			break;
		case 0x7b:
			break;
		case 0x7c:
			break;
		case 0x7d:
			break;
		case 0x7e:
			break;
	}
	ptr_reg = addr_sp+io_off+0xff00; // pointer to register
	*ptr_reg &= 0x0; // reset register
	*ptr_reg |= io_new; // write new register
}
