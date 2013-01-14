#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <termios.h>

#define GB_CLK 4194304 // HZ
#define SGB_CLK 4295454 // HZ
#define CGB_CLK 8400000 // HZ
#define DIV_CLK_GB 16384 // HZ
#define DIV_CLK_SGB 16779 // HZ
#define DIV_CLK_CGB 32768 // HZ
#define SCR_RSH 60 // refresh screen 60 times per second
#define V_BLN_DUR 4560 // duration of v-blank in ticks
#define OAM_DUR 80 // duration of oam in ticks
#define TRNSFR_DUR 171 // duration of transfer in ticks
#define TOT_DUR 456 // duration of modes 0, 2, 3
#define JOY_REG 0xff00 // offset to div register
#define DIV_REG 0xff04 // offset to div register
#define TIMA 0xff05 // offset to tima register
#define TMA 0xff06 // offset to tma register
#define TAC 0xff07 // offset to tac register
#define NR10 0xff10 // offset to tac register
#define NR11 0xff11 // offset to tac register
#define NR12 0xff12 // offset to tac register
#define NR13 0xff13 // offset to tac register
#define NR14 0xff14 // offset to tac register
#define NR20 0xff15 // offset to tac register
#define NR21 0xff16 // offset to tac register
#define NR22 0xff17 // offset to tac register
#define NR23 0xff18 // offset to tac register
#define NR24 0xff19 // offset to tac register
#define NR30 0xff1a // offset to tac register
#define NR31 0xff1b // offset to tac register
#define NR32 0xff1c // offset to tac register
#define NR33 0xff1e // offset to tac register
#define NR34 0xff1f // offset to tac register
#define NR41 0xff20 // offset to tac register
#define NR42 0xff21 // offset to tac register
#define NR43 0xff22 // offset to tac register
#define NR44 0xff23 // offset to tac register
#define NR50 0xff24 // offset to tac register
#define NR51 0xff25 // offset to tac register
#define NR52 0xff26 // offset to tac register
#define LCDC_REG 0xff40 // offset to lcd control register
#define LCDS_REG 0xff41 // offset to lcd status register
#define SCY 0xff42 // offset to lcd control register
#define SCX 0xff43 // offset to lcd control register
#define LY_REG 0xff44 // offset to ly control register
#define LYC_REG 0xff45 // offset to ly control register
#define BGP_REG 0xff47 // offset to ly control register
#define OBP0_REG 0xff48 // offset to ly control register
#define OBP1_REG 0xff49 // offset to ly control register
#define WY_REG 0xff4a // offset to ly control register
#define WX_REG 0xff4b // offset to ly control register
#define IR_REG 0xff0f // offset to interrupt request register
#define IE_REG 0xffff // offset to interrupt enable register
#define CGB_ONLY 0xc
#define CGB 4
#define SGB 2
#define DMG 1
#define DIV 1
#define H_BLN_PER 0x00
#define V_BLN_PER 0x01
#define OAM_PER 0x02
#define TRNSFR_PER 0x03
#define V_BLN_MSK 0x08
#define H_BLN_INT 0x08 // set hblank interrupt in lcd status
#define V_BLN_INT 0x10 // set vblank interrupt in lcd status
#define OAM_INT 0x20 // set oam interrupt in lcd status
#define LY_LYC_INT 0x40 // set LY/LYC coincidence interrupt in lcd status
#define OFF 0
#define ON 1
#define MAX_STRS 8
#define NUM_CMDS 6
#define STR_OFF 8

/* register pairs (words) XXX offsets for amd64 */
#define AF 0
#define BC 8
#define DE 16
#define HL 24

/* special registers XXX offsets for amd64 */
#define SP 32
#define PC 40

/* general-purpose registers */
#define F_REG 0 # flag
#define A_REG 1 # accumulator
#define C_REG 4 # general
#define B_REG 5 # general
#define E_REG 8 # general
#define D_REG 9 # general
#define L_REG 12 # general
#define H_REG 13 # general

#define BYTE 0xffffff00
#define WORD 0xffff0000
/* addressing bytes or words */
/* addressing modes */
#define IMP 0x01
#define REG 0x02
#define REG_IND 0x04
#define IMM 0x08
#define IMM_IND 0x10

#define READ 0
#define WRITE 1
#define ROM_BANK_MODE 0x01
#define RAM_BANK_MODE 0x02

/*
 * Structures used by gboy
 */

/* information about the cartridge ROM being executed */
struct gb_cart {
	Uint8 cart_type;
	Uint8 cart_gb;
	Uint8 cart_types[3];
	char cart_name[17];
	char cart_licensee[3];
	int cart_size;
	unsigned char cart_rom_size;
	unsigned char cart_ram_size;
	int cart_cgb;
	int cart_sgb;
	int cart_curom_bank;
	int cart_curam_bank;
	int cart_cuvram_bank;
	int cart_cuwram_bank;
	char *cart_rom_banks;
	char *cart_ram_banks;
	FILE *cart_ram_fd;
	char *cart_vram_bank;
	char *cart_wram_bank;
} gb_cart;

/* information about the boot ROM */
struct gb_boot {
	int boot_size;
} gb_boot;

/* information about the cartridge's MBC chip */
struct gb_mbc {
	void (*mbc_funcs[10])(int); // maximum 7 mbc functions
	char mbc_ram_rom_mode;
	char mbc_ram_rom_upp;
	char mbc_rtc_reg_sel;
	char mbc_rtc_regs[5];
} gb_mbc;

/* 
 * Structures used by the commmand-line interpreter.
 */
/* list containing commands entered */
struct cmd_stack {
	char *cmd_buf;
	void (*cmd_fun)(int, char **);
	int num_args;
	char **cmd_ptrs;
	struct cmd_stack *ptr_fw;
	struct cmd_stack *ptr_bk;
};

/* pointers to beginning, end and current character of input buffer */
struct cmd_line {
	char *ptr_cur; // pointer to current byte in buffer
	char *ptr_end; // pointer at the end of the buffer
	char *ptr_beg; // pointer at the beginning of buffer
} cmd_line;

/* defined in globals.c */
extern char inp_buf[512];
extern char *ptr_dup;
extern char *cmd_ptrs[MAX_STRS+2];
extern unsigned int grey[4];
extern long nb_spr;
extern char *rom_sz_vec[];
extern char *ram_sz_vec[];
extern const int gb_sizes[];
extern const int gb_rates[];
extern const int gb_divs[];
extern const char *types_vec[];
extern int ints_offs[];
extern SDL_Surface *screen;
extern SDL_Surface *back;
extern SDL_Surface *back_save;
extern SDL_AudioSpec desired;
extern int boot_fd;
extern unsigned char addr_sp[0xffff];
extern int gbddb;
extern int gbplay;
extern int lcd_bg_win_datsel;
extern int lcd_sprt_enable;
extern int lcd_bg_dis_enable;
extern char *shift_var;
extern int lcd_stat;
extern int gb_int_tcks[2];
extern int gb_vb_tcks[2];
extern int gb_clk_rate;
extern Sint16 *playbuf;
extern double freq_tbl[2048];
extern double freq_tbl_snd3[2048];
extern double freq_tbl_snd4[256];
extern int type; // default to CGB
extern int rom_fd;
#define COL32_TO_16(col) ((((col&0xff0000)>>19)<<11)|(((col&0xFF00)>>10)<<5)|((col&0xFF)>>3))
/* File descriptor for cartridge */
#define RET_MASK 0x1
#define D_MASK 0x2
#define S_MASK 0x4
#define A_MASK 0x8
#define UP_MASK 0x10
#define DOWN_MASK 0x20
#define LEFT_MASK 0x40
#define RIGHT_MASK 0x80
