#include "gboy.h"
static char welcome_message[] = "\n==================================\nWelcome to gboy: A x86/x86-64 hand-written Nintendo® Game Boy® Emulator\nType help\n";
/* strings of valid commands */
static const char * const gboy_cmds[] = { "help", "load", "play", "debug", "fps", "mode" };
/* functions for commands defined in gboy_cmd.c */
extern void gboy_ddb(int, char **);
extern void gboy_play(int, char **);
extern void gboy_load(int, char **);
extern void gboy_help(int, char **);
extern void gboy_fps(int, char **);
extern void gboy_mode(int, char **);
/* pointers to command functions */
void (*gboy_cmds_funcs[NUM_CMDS])(int, char **) = { gboy_help, gboy_load, gboy_play, gboy_ddb, gboy_fps, gboy_mode };

/*
 * Call the interpreter until a file is loaded and the 'play' command executed
 */
static void
gboy_init_interp()
{
	gbplay=0;

	printf("%s", welcome_message);
	while (gbplay == 0)
		gboy_interp("gboy> ", NUM_CMDS, gboy_cmds, gboy_cmds_funcs, NULL);
}

/*
 * Main function.
 * Loops through the command-line interpreter until ready to play.
 * XXX implement argv
 */
int
main(int argc, char *argv[])
{

	while (1) {
		if (argc==1)
			gboy_init_interp(); // call the command-line interface
		else {
			argc=1;
			if ( (rom_fd=open(argv[1], O_RDWR)) == -1) {
				write(1, argv[1], strnlen(argv[1], 255));
				perror(" couldn't be opened.\nopen()");
			}
			else {
				printf("File %s opened\n", argv[1]);
			}
		}
		while (1) {
			/*
			 * start_vm() returns when the file opened doesn't represent
			 * a valid Game Boy ROM, or when the user chooses to change
			 * the ROM.
			 */
			if ((start_vm(rom_fd))==-1)
				printf("File not a gb binary\n\n");
			close(rom_fd);
			rom_fd=0;
			break;
		}
	}

	/* execution never gets here; the program exits through a callback function */
	return 0;
}
