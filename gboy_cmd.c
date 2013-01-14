#include "gboy.h"

void
gboy_ddb(int num_args, char **ptr_ptrs)
{
	if (num_args>1)
		printf("Usage: debug\n");
	else {
		if (gbddb == 0)
			gbddb = 1;
		else
			gbddb = 0;
	}

	printf("gboy debug is %d\n", gbddb);
}

void
gboy_play(int num_args, char **ptr_ptrs)
{
	int ret_val;

	if (num_args > 1)
		printf("Usage: play\n");
	else {
		if (rom_fd > 0)
			gbplay = 1;
		else
			printf("First load a cartridge.\n");
	}
}

void
gboy_help(int num_args, char **ptr_ptrs)
{
	if (num_args > 1)
		printf("Usage: help\n");
	else
		printf("Commands: load, play, debug, fps\n");
}

void
gboy_load(int num_args, char **ptr_ptrs)
{
	static char *rom_str;

	if (num_args!=2)
		printf("Usage: load 'file_path'\n");
	else {
		if (rom_fd!=-1 && rom_fd!=0) {
			close(rom_fd);
			printf("File %s closed\n", rom_str);
		}
		if ( (rom_fd=open(ptr_ptrs[1], O_RDWR)) == -1)
			perror(ptr_ptrs[1]);
		else
			printf("\nFile '%s' opened. (Use 'play' command to start emulation)\n", ptr_ptrs[1]), rom_str=strndup(ptr_ptrs[1], 255);
	}
}

void
gboy_mode(int num_args, char **ptr_ptrs)
{
	if (num_args!=2)
		printf("Mode\n");
	else {
		if (!(strncmp(ptr_ptrs[1], "CGB", 255)) || !(strncmp(ptr_ptrs[1], "cgb", 255)))
			printf("Mode set to CGB\n"), type=1;
		else if (!(strncmp(ptr_ptrs[1], "DMG", 255)) || !(strncmp(ptr_ptrs[1], "dmg", 255)))
			printf("Mode set to DMG\n"), type=0;
		else
			printf("Available modes are: \"DMG\" and \"CGB\"\n");
	}
}

extern int frames_per_second;
void
gboy_fps(int num_args, char **ptr_ptrs)
{
	int fps;

	if (num_args != 2)
		printf("Usage: fps int\n");
	else {
		fps=atoi(ptr_ptrs[1]);
		if (fps>0 && fps<=60)
			printf("FPS set to %d\n", frames_per_second=fps);
		else
			printf("Bad integer %d for FPS\n", fps);
	}
}
