#include "../gboy.h"

char gddb_msg[] = "\nThis is gddb\n";
char *gddb_cmds[] = { "print", "show", "disasm", "step", "break", "help", "cont" };
/* functions to handle commands */
extern void gddb_print(int, char **);
extern void gddb_show(int, char **);
extern void gddb_step(int, char **);
extern void gddb_cont(int, char **);
extern void gddb_dasm(int, char **);
extern void gddb_break(int, char **);
extern void gddb_help(int, char **);
extern void gddb_disasm(int, int);
extern char gddb_buf[512];
extern long regs_sets;

char *gboy_pc;
char *op_rec;
long gddb_contil=-1;
int gddb_loop=1;
int gddb_tmp=0;

void (*gddb_cmds_funcs[7])(int, char **) = { gddb_print, gddb_show, gddb_dasm, gddb_step, gddb_break, gddb_help, gddb_cont };

void
gddb_reset()
{
	gbddb=0;
	gddb_contil=-1;
	gddb_loop=1;
	gddb_tmp=0;
}

void
gddb_main(int null_null, char *ptr_gboy_pc, char *ptr_op_rec)
{
	int i;
	char *cmd_ptrs[MAX_STRS+2]; // array of pointers to strings (last two are NULL)

	if (gddb_contil != -1) {
		if (*(&regs_sets+5) == gddb_contil) {
			if (gddb_tmp) {
				gddb_tmp--; goto out;}
			printf("Breakpoint at 0x%x\n", gddb_contil), gddb_contil = -1;
		}
		else
			goto out;
	}
	gboy_pc = ptr_gboy_pc;
	op_rec = ptr_op_rec;
	gddb_disasm(1, 0);

	/* receive commands */
 	/* output message and input string parse string and get number */
	while (gddb_loop)
		gboy_interp("gddb> ", 7, gddb_cmds, gddb_cmds_funcs, gddb_step);
out:
	gddb_loop = 1;

}
