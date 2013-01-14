#include "gboy.h"
extern char *addr_sp_ptrs[0x10]; // pointer to address spaces

/* functions included in various mbc's */
extern void mbc1_mode(int);
extern void mbc1_ram_bank(int);
extern void mbc1_rom_bank(int);
extern void mbc1_ram_en(int);
extern void mbc2_rom_bank(int);
extern void mbc2_ram_en(int);
extern void mbc2_ram_wr(int);
extern void mbc3_clk(int);
extern void mbc3_ramrtc_rdwr(int, int);
extern void mbc3_ramrtc_bank(int);
extern void mbc3_rom_bank(int);
extern void mbc3_ramtim_en(int);

/*
 * General RAM remap function.
 */
void
mbc_ram_remap()
{
	addr_sp_ptrs[0xa] = addr_sp_ptrs[0xb] = ((char *)&gb_cart.cart_ram_banks[0x2000*gb_cart.cart_curam_bank])-0xa000;
}

/*
 * General ROM remap function.
 */
void
mbc_rom_remap()
{
	addr_sp_ptrs[7] = addr_sp_ptrs[4] = addr_sp_ptrs[5] = addr_sp_ptrs[6] = ((char *)&gb_cart.cart_rom_banks[0x4000*(gb_cart.cart_curom_bank-1)])-0x4000;
}

/*
 * Main structure holding the various mbcX-specific functions.
 * Used by Assembly routine gboy_cpu.S
 */
static void (*mbc_def_funcs[3][5])(int) = { mbc1_ram_en, mbc1_rom_bank, mbc1_ram_bank, mbc1_mode, NULL, mbc2_ram_wr, mbc2_ram_en, mbc2_rom_bank, NULL, NULL, mbc3_ramtim_en, mbc3_rom_bank, mbc3_ramrtc_bank, mbc3_clk, NULL };

/*
 * Initialize gb_mbc structure.
 */
void
mbc_init(int mbc_num)
{

	/* assign and offset */
	switch (mbc_num) {
		case 1:
		case 2:
		case 3:
			mbc_num = 0;
			break;
		case 5:
		case 6:
			mbc_num = 1;
			break;
		case 0x0f:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			mbc_num = 2;
			break;
	};

	/* assign functions */
	gb_mbc.mbc_funcs[0] = mbc_def_funcs[mbc_num][0];
	gb_mbc.mbc_funcs[1] = mbc_def_funcs[mbc_num][0];
	gb_mbc.mbc_funcs[2] = mbc_def_funcs[mbc_num][1];
	gb_mbc.mbc_funcs[3] = mbc_def_funcs[mbc_num][1];
	gb_mbc.mbc_funcs[4] = mbc_def_funcs[mbc_num][2];
	gb_mbc.mbc_funcs[5] = mbc_def_funcs[mbc_num][2];
	gb_mbc.mbc_funcs[6] = mbc_def_funcs[mbc_num][3];
	gb_mbc.mbc_funcs[7] = mbc_def_funcs[mbc_num][3];
	gb_mbc.mbc_funcs[8] = mbc_def_funcs[mbc_num][4];
	gb_mbc.mbc_funcs[9] = mbc_def_funcs[mbc_num][4];
}
