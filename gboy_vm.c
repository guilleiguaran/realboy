#include "gboy.h"
#define CART_HDR 336
#define NIN_LOG 0x104
#define LOG_SIZ 29
#define NUM_SCAN 152
#define GAM_TIT 0x134 // ascii game title
#define MAN_COD 0x13f // manufacurer code
#define CGB_FLG 0x143 // CGB support
#define GAM_LIC_NEW 0x144 // licensee (new)
#define JAP_VER 0x14a // japanese version
#define GAM_LIC 0x14b // licensee
#define SGB_FLG 0x146 // SGB support
#define CAR_TYP 0x147 // memory sets supported by cartridge
#define ROM_SIZ 0x148 // ROM size in (32KB<<n) units
#define RAM_SIZ 0x149 // external RAM size

/* defined in gboy_cpu.S */
extern long gb_line_clks;
extern long gb_vbln_clks;
extern long gb_oam_clks;
extern long gb_vram_clks;
extern long gb_hblank_clks;
extern int *addr_sp_ptrs[0x10]; // pointers to address spaces

const char *gb_boot_strs[] = { "./boot_roms/dmg_rom.bin", "./boot_roms/cgb_bios.bin" };
/* Temporary space for cartridge's header */
static Uint8 cart_init_rd[335];
/* Nintendo Gameboy signature */
static const unsigned char nin_log[] = { 0xce, 0xed, 0x66, 0x66,  0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f,  0x88, 0x89, 0x00, 0x0e, 0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63,  0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e };


/*
 * Load boot ROM.
 */
static void
ld_boot()
{
	/* open file XXX */
	if ( (boot_fd = open(gb_boot_strs[type], 0)) == -1)
		printf("Error open\n");
	
	/* read to address space XXX */
	if (type==1)
		read(boot_fd, addr_sp, 0x8ff);
	else
		read(boot_fd, addr_sp, 256);

}
/*
 * Load boot ROM.
 */
static void
set_up()
{
	/* start with mode 2 */
	addr_sp[0xff41] |= 0x82;
}

/*
 * Parse cartridge information and print it.
 */
static void
parse_carthdr()
{
	int i, j;

	/* copy name XXX limit ourselves to 11 characters; this restriction apparently came with the CGB */
	for (i=0; i<16; i++)
		gb_cart.cart_name[i] = (cart_init_rd+GAM_TIT)[i];

	printf("\nCartridge \"%s\":\n", gb_cart.cart_name);
	printf("================================\n");
	if (cart_init_rd[GAM_LIC]==0x33) {
		gb_cart.cart_licensee[0] = (char)cart_init_rd[GAM_LIC_NEW];
		gb_cart.cart_licensee[1] = (char)cart_init_rd[GAM_LIC_NEW+1];
	}
	else {
		gb_cart.cart_licensee[0] = (char)cart_init_rd[GAM_LIC];
		gb_cart.cart_licensee[1] = (char)cart_init_rd[GAM_LIC+1];
	}
	gb_cart.cart_licensee[2] = 0;
	if ( ((gb_cart.cart_sgb |= cart_init_rd[SGB_FLG] & 0x03) == 0x03))
		printf("SGB Support\n");
	
	/* copy type, ROM size and RAM size */
	gb_cart.cart_type = cart_init_rd[CAR_TYP];
	gb_cart.cart_rom_size = cart_init_rd[ROM_SIZ];
	gb_cart.cart_ram_size = cart_init_rd[RAM_SIZ];

	/* print information */
	printf("%s\n", types_vec[gb_cart.cart_type&0xff]);
	printf("%s\n", rom_sz_vec[gb_cart.cart_rom_size&0xff]);
	printf("%s\n", ram_sz_vec[gb_cart.cart_ram_size&0xff]);
	if (cart_init_rd[GAM_LIC]==0x33)
		printf("Licensee code: %s\n", gb_cart.cart_licensee);
	else
		printf("Licensee code: 0x%X\n", (Uint8)*gb_cart.cart_licensee);
	if (cart_init_rd[JAP_VER]==0)
		printf("Japanese version\n");
	else
		printf("Non-japanese version\n");

	for (i=0x134, j=0; i<=0x14c; i++)
		j=j-cart_init_rd[i]-1;

	/* verify checksum */
	printf("Checksum: ");
	if ((Uint8)j != cart_init_rd[0x14d])
		printf("Failed!\n");
	else
		printf("OK\n");
									
}

static void
alloc_addrsp()
{
	int i;

	/* set addresses starting with 0x0, 0x1, ..., 0xf to default address space */
	for (i=0; i<=16; i++)
		addr_sp_ptrs[i] = (int *)addr_sp;

	/* allocate space for rom banks */
	gb_cart.cart_rom_banks = malloc(0x8000<<gb_cart.cart_rom_size);
	/* initial ROM addresses */
	addr_sp_ptrs[4]=addr_sp_ptrs[5]=addr_sp_ptrs[6]=addr_sp_ptrs[7]=(int *)(gb_cart.cart_rom_banks-0x4000);

	/* determine RAM size: gb_cart.cart_ram_size*1024 bytes */
	switch (gb_cart.cart_ram_size) {
		case 0:
			gb_cart.cart_ram_size = 0;
			break;
		case 1:
			gb_cart.cart_ram_size = 2;
			break;
		case 2:
			gb_cart.cart_ram_size = 8;
			break;
		case 3:
			gb_cart.cart_ram_size = 32;
			break;
		default:
			;
	}
	
	/* if we have external RAM */
	if (gb_cart.cart_ram_size) 
	{
 		/* allocate space for RAM banks */
		gb_cart.cart_ram_banks = malloc(1024*gb_cart.cart_ram_size);
		/* try to open RAM file */
		if ((gb_cart.cart_ram_fd=fopen(gb_cart.cart_name, "r+"))==NULL)
		{
			/* there is no RAM file; create one */
			gb_cart.cart_ram_fd = fopen(gb_cart.cart_name, "w+");
			fwrite(gb_cart.cart_ram_banks, 1, 1024*gb_cart.cart_ram_size, gb_cart.cart_ram_fd);
		}
		/* there exists a RAM file, so read it to RAM space */
		else
			fread(gb_cart.cart_ram_banks, 1, 1024*gb_cart.cart_ram_size, gb_cart.cart_ram_fd);
		/* initial RAM addresses */
		addr_sp_ptrs[0xa]=addr_sp_ptrs[0xb]=(int *)(gb_cart.cart_ram_banks-0xa000);
	}
	/* the cartridge has no external RAM */
	else
		gb_cart.cart_ram_banks = NULL;

	/* if CGB, assign second bank of VRAM and WRAM banks */
	if (gb_cart.cart_gb&CGB)
	{
		gb_cart.cart_vram_bank=(char *)malloc(0x2000);
		gb_cart.cart_wram_bank=(char *)malloc(0x1000*7);
		gb_cart.cart_cuvram_bank=0;
		gb_cart.cart_cuwram_bank=0;
	}
	else
	{
		gb_cart.cart_vram_bank=NULL;
		gb_cart.cart_wram_bank=NULL;
	}

	/* set some initial values; don't assume boot ROM will set them */
	addr_sp[TIMA] = 0;
	addr_sp[TMA] = 0;
	addr_sp[TAC] = 0;
	addr_sp[NR10] = 0x80;
	addr_sp[NR11] = 0xbf;
	addr_sp[NR12] = 0xf3;
	addr_sp[NR14] = 0xbf;
	addr_sp[NR21] = 0x3f;
	addr_sp[NR22] = 0x00;
	addr_sp[NR24] = 0xbf;
	addr_sp[NR30] = 0x7f;
	addr_sp[NR31] = 0xff;
	addr_sp[NR32] = 0x9f;
	addr_sp[NR33] = 0xbf;
	addr_sp[NR41] = 0xff;
	addr_sp[NR42] = 0;
	addr_sp[NR43] = 0;
	addr_sp[NR30] = 0xbf;
	addr_sp[NR50] = 0x77;
	addr_sp[NR51] = 0xf3;
	addr_sp[NR52] = 0x80;
	addr_sp[LCDS_REG] = 0x80;
	addr_sp[LCDC_REG] = 0x0;
	addr_sp[SCY] = 0;
	addr_sp[SCX] = 0;
	addr_sp[LYC_REG] = 0;
	addr_sp[BGP_REG] = 0xfc;
	addr_sp[OBP0_REG] = 0xff;
	addr_sp[OBP1_REG] = 0xff;
	addr_sp[WY_REG] = 0;
	addr_sp[WX_REG] = 0;
	addr_sp[IE_REG] = 0;
	addr_sp[JOY_REG] = 0x0;
	addr_sp[0xffbb] = 0;
}

int
start_vm()
{
	int i;

	/* read cartridge's header */
	read(rom_fd, cart_init_rd, CART_HDR);

	/* check header */
	for (i=0; i<LOG_SIZ; i++) {
		if (((cart_init_rd+NIN_LOG)[i]) != nin_log[i])
			break;
	}

	/* if valid ROM (XXX this doesn't actually test for the validity of a ROM; this is done through checksuming */
	if (i==LOG_SIZ) {
		gb_cart.cart_gb = DMG;
		if (cart_init_rd[SGB_FLG] == 0x03)
			gb_cart.cart_gb |= SGB;
		/* get information from cartridge's header */
		parse_carthdr();
		/* allocate and initialize address space */
		alloc_addrsp();
		/* initialize LCD timings */
		set_up();
		/* load boot rom */
		ld_boot();
		/* initialize MBC driver */
		mbc_init(gb_cart.cart_type);
		/* load bank 0 to address space if DMG */
 		if (type==0)
			pread(rom_fd, addr_sp+0x100, 0x4000-0x100, 256);
		/* load additional banks */
		pread(rom_fd, gb_cart.cart_rom_banks, 0x8000<<gb_cart.cart_rom_size, 0x4000);
		/* set CGB if need to */
		if (type==1)
			addr_sp[0x143]=0xc0;
		/* initialize the video subsystem */
		vid_start();
		/* initialize the sound subsystem */
		snd_start();
		/* start execution */
		rom_exec();
	}
	/* else bad ROM */
	else
		return -1;

	/* free resources */
	free(gb_cart.cart_rom_banks);
	if (gb_cart.cart_ram_size && (gb_cart.cart_ram_banks!=NULL))
	{
		free(gb_cart.cart_ram_banks);
		fclose(gb_cart.cart_ram_fd);
	}
	if (gb_cart.cart_gb&CGB)
	{
		if (gb_cart.cart_vram_bank!=NULL)
			free(gb_cart.cart_vram_bank);
		if (gb_cart.cart_wram_bank!=NULL)
			free(gb_cart.cart_wram_bank);
	}
	close(boot_fd);
	close(rom_fd);
	vid_reset();
	snd_reset();
	gddb_reset();
	SDL_Quit();

	return 0;
}
