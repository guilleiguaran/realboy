#include <SDL/SDL.h>
#define MAX_STRS 8
#define GB_CLK 4194304 // HZ
#define SGB_CLK 4295454 // HZ
#define CGB_CLK 8400000 // HZ
#define DIV_CLK_GB 16384 // HZ
#define DIV_CLK_SGB 16779 // HZ
#define DIV_CLK_CGB 32768 // HZ

char *rom_sz_vec[] = { "Rom size 32kb", "Rom size 64kb", "Rom size 128kb", "Rom size 256kb", "Rom size 512kb", "Rom size 1mb", "Rom size 2mb", "Rom size 4mb" };
char *ram_sz_vec[] = { "None", "Ram size 2kb", "Ram size 8kb", "Ram size 32kb" };
const int gb_sizes[] = { 2303, 256, 256 };
const int gb_rates[] = { CGB_CLK, GB_CLK, GB_CLK };
const int gb_divs[] = { DIV_CLK_CGB, DIV_CLK_GB, DIV_CLK_GB };;

/* strings for verbosity */
const char *types_vec[] = { "Rom only", "MBC1", "MBC1+RAM", "MBC1+RAM+BATTERY", NULL, "MBC2", "MBC2+BATTERY", NULL, "ROM+RAM", "ROM+RAM+BATTERY", NULL, "MMM01", "MMM01+RAM", "MMM01+RAM+BATTERY", NULL, "MBC3+TIMER+BATTERY", "MBC3+TIMER+RAM+BATTERY", "MBC3", "MBC3+RAM", "MBC3+RAM+BATTERY", NULL, NULL, NULL, NULL, NULL, "MBC5", "MBC5+RAM", "MBC5+RAM+BATTERY", "MBC5+RUMBLE", "MBC5+RUMBLE+RAM", "MBC5+RUMBLE+RAM+BATTERY" };
//"Game %s\n",
//"SGB support\n",
//"CGB support\n",
//"Cartridge type %s\n",
//"Ram size %dkb\n",
int ints_offs[] = { 0x40, 0x48, 0x50, 0x58, 0x60 };
char inp_buf[512];
char *ptr_dup;
char *cmd_ptrs[MAX_STRS+2]; // array of pointers to strings (last two are NULL)
unsigned int grey[4];
SDL_Surface *screen;
SDL_Surface *back;
SDL_Surface *back_save;
SDL_AudioSpec desired;
/* File descriptor for boot ROM */
int boot_fd;
/* Pointer to address space */
unsigned char addr_sp[0xffff];
/* enable (1)/disable (0) debug */
int gbddb = 0;
/* control variable set when ROM loaded and 'play' command */
int gbplay = 0;
Sint16 *playbuf = NULL;

double freq_tbl[2048];
double freq_tbl_snd3[2048];
double freq_tbl_snd4[256];
int type=0; // default to DMG
int rom_fd;
