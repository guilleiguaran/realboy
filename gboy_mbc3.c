#include "gboy.h"
extern char *addr_sp_ptrs[0x10]; // pointer to address spaces
extern long *addr_sp_bases[0x10]; // pointers to bases
//extern long test_tmp_val;

void
mbc3_ram_remap()
{
	if (!(gb_mbc.mbc_rtc_reg_sel)) {
		addr_sp_ptrs[0xa]=addr_sp_ptrs[0xb]=((char *)&gb_cart.cart_ram_banks[0x2000*gb_cart.cart_curam_bank])-0xa000;
	}
	else {
		addr_sp_ptrs[0xa]=addr_sp_ptrs[0xb]=&gb_mbc.mbc_rtc_regs[gb_mbc.mbc_rtc_reg_sel-8];
	}
}

void
mbc3_clk(int val)
{
	if (val==1)
		;
}

void
mbc3_ramrtc_bank(int val)
{
	if (val > 3)
		gb_mbc.mbc_rtc_reg_sel = val&0x0f;
	else {
		gb_mbc.mbc_rtc_reg_sel = (char)0; 
		gb_cart.cart_curam_bank = val;
	}
	mbc3_ram_remap();
}

void
mbc3_rom_bank(int val)
{
	if (val==0)
		gb_cart.cart_curom_bank = 1; // new ROM bank
	else
		gb_cart.cart_curom_bank = val&0x7f; // new ROM bank

	mbc_rom_remap(); // remap
}

void
mbc3_ramtim_en(int val)
{
	/* sync RAM file */
	if ((val&=0xf) != 0xa) {
		rewind(gb_cart.cart_ram_fd);
		fwrite(gb_cart.cart_ram_banks, 1, 1024*gb_cart.cart_ram_size, gb_cart.cart_ram_fd);
	}
}
