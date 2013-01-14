/*
 * Command-line user interface.
 */
#include "gboy.h"
extern struct cmd_stack *cmd_tail; // pointer to the last command
void (*gboy_last_func)(int, char **) = 0;

int
gboy_exec_strs(int num_cmds, const char * const gboy_cmds[], 
							 void (*gboy_funcs[])(int, char **))
{
	int m;

	/* iterate through available commands and call function if coincidence */
	for (m=0; m < num_cmds; m++) {
		if (!strncmp(cmd_ptrs[0], gboy_cmds[m], 255)) {
			gboy_funcs[m](cmd_tail->num_args, cmd_ptrs);
			gboy_last_func=gboy_funcs[m];
			break;
		}
	}

	if (m == num_cmds)
		printf("Command not found...\n");
}

/*
 * Parse input and return number of arguments.
 * Identify arguments by empty spaces (ie. ' ' character).
 * Arrange an array of pointers to strings in cmd_ptrs.
 */
int
gboy_pars_strs(int num_cmds, const char * const gboy_cmds[], 
			   void (*gboy_funcs[])(int, char **))
{
	int i = 0; // store number of arguments
	int m;
	char *ptr_buf = inp_buf; // pointer to beginning of string

	/* ignore spaces and tabs until first character found */
	while (*ptr_buf == ' ' || *ptr_buf == '\t')
		ptr_buf++;

	ptr_dup = strndup(ptr_buf, 255);
	cmd_ptrs[i]=ptr_buf; // pointer for the first string; the command
	i=1; // first count for argc
	/* convert each 'space' to 'end of string' (ie. ' ' -> '\0') and set pointers */
	while (*ptr_buf++ != '\0' && i <= MAX_STRS-1) {
		if (*ptr_buf==' ' || *ptr_buf=='\t') {
			*ptr_buf++='\0'; // replace with '\0' and advance to next character
			while (*ptr_buf==' ' || *ptr_buf=='\t')
				ptr_buf++;
			if (*ptr_buf == '\n' || *ptr_buf == '\0')
				break;
			else
				cmd_ptrs[i++]=ptr_buf;
		}
		else if (*ptr_buf == '\n')
			*ptr_buf++ = '\0';
	}
	*ptr_buf = '\0';

	/* did we get the maximum strings? */
	if (i<MAX_STRS-1) /* no */
		*(ptr_buf-1)='\0', cmd_ptrs[i+1]=NULL; // for reference

	/* queue commands */
	if (strncmp(ptr_dup, "\n", 255))
		cmd_add_que(ptr_dup, gboy_funcs[i], i, cmd_ptrs);

	gboy_exec_strs(num_cmds, gboy_cmds, gboy_funcs);
}
