#include "gboy.h"

void
mbc1_mode(int val)
{
	if ((val&=1) == 0)
		gb_mbc.mbc_ram_rom_mode = ROM_BANK_MODE, gb_cart.cart_curom_bank |= gb_mbc.mbc_ram_rom_upp<<5;
	else
		gb_mbc.mbc_ram_rom_mode = RAM_BANK_MODE, gb_cart.cart_curom_bank &= 0x1f;
	mbc_rom_remap();
}

void
mbc1_ram_bank(int val)
{
	gb_mbc.mbc_ram_rom_upp = val &= 0x3;

	/* if ROM_BANK_MODE remap ROM bank with 'val' as upper bits */
	if (gb_mbc.mbc_ram_rom_mode == ROM_BANK_MODE) {
		gb_cart.cart_curom_bank |= val<<5;
		mbc_rom_remap();
	}
	/* else remap RAM bank */
	else {
		gb_cart.cart_curam_bank = val;
		mbc_ram_remap();
	}
}

/* select ROM bank for mbc1 */
void
mbc1_rom_bank(int val)
{
	if (val == 0x20 || val == 0x40 || val == 0x60 || val == 0)
		gb_cart.cart_curom_bank = (val&0x3f)+1;
	else
		gb_cart.cart_curom_bank = val&0x3f; // update current ROM bank

	mbc_rom_remap();
}

/* enable RAM for mbc1 */
void
mbc1_ram_en(int val)
{
	/* sync RAM file */
	if (((val&=0xf) != 0xa) && (gb_cart.cart_ram_banks!=NULL)) {
		rewind(gb_cart.cart_ram_fd);
		fwrite(gb_cart.cart_ram_banks, 1, 1024*gb_cart.cart_ram_size, gb_cart.cart_ram_fd);
	}
}
