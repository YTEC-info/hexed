/******************************************************************************
 *
 *	cmdargs.c - Command line argument parsing
 *
 ******************************************************************************
 *
 *	Public Domain - No warranties expressed or implied
 *
 ******************************************************************************
 *
 *      Parses the command line arguments returning the argument id found,
 *      zero for an unknown argument or -1 for end of arguments. Cmdargs
 *      has 1 public struct pointer called cmdargs that sets the argument ID,
 *	the argument string and the option string.
 *
 *      example:
 *
 *              cmdargs->argid = Arglist argument id
 *              cmdargs->arg = argument string
 *              cmdargs->opt = options string (or NULL)
 *
 *      parsecmdargs() requires an Arglist struct to scan through the
 *	arguments for matches. Args may include options or can be a single
 *	character arg that can be set together with other args. An option
 *      can be set by using ':', '=' or white space between the arg and the
 *	option.
 *
 *      example:
 *
 *              program -abc -d option -longarg:option --longarg2=option
 *
 *
 *      Notes:	The returned argid is arglist index value + 1,
 *		use arglist[cmdargs->argid - 1] to find the arglist data.
 *
 *              The last arglist value must be {NULL, 0}
 *
 *		The Arglist struct must list alaphabetically ordered longer
 *		args first, then single args.
 *
 *      example:
 *
 *		Arglist arglist[] = {
 *                      {"bar", 0},             // set the bar arg
 *                      {"b", 0},               // set the b arg
 *			{"foobar", 0}, 		// set the foobar arg
 *			{"foo", 0},     	// set the foo arg
 *			{"f", 0},		// set the f arg
 *                      {NULL, 0}		// must be the last entry
 *		};
 *
 *****************************************************************************/


#include <stdlib.h>
#include <string.h>
#include "cmdargs.h"


Cmdargs *cmdargs = NULL;
static Cmdargs _cmdargs;


/*
 * Parse the command line arguments
 */
int parsecmdargs(char *argv[], Arglist *arglist)
{
	int i;
	char *arg;

	// set cmdargs
	if(cmdargs != &_cmdargs){
		cmdargs = &_cmdargs;
		memset(cmdargs, 0, sizeof(Cmdargs));
	}
	cmdargs->argid = 0;

	// get next arg
	if((cmdargs->flags & CMDARGS_FL_SINGLE) == 0){
		cmdargs->idx++;
		arg = argv[cmdargs->idx];
	       	cmdargs->flags = 0;

	// single step through this arg
	}else{
		arg = cmdargs->arg + 1;
		cmdargs->flags &= CMDARGS_FL_RESET;
	}

	// error or end of command line
	if(argv == NULL || arglist == NULL || argv[cmdargs->idx] == NULL){
		memset(cmdargs, 0, sizeof(Cmdargs));
		return -1;
	}

	// check for argument
	cmdargs->arg = arg;
	if(*arg == '-'){
       		cmdargs->flags |= CMDARGS_FL_SETARG;
		arg++;
		if(*arg == '-'){
			arg++;
			cmdargs->flags |= CMDARGS_FL_LONGARG;
		}
	}

	// scan arglist
	if((cmdargs->flags & CMDARGS_FL_SETARG) != 0){
		for(i = 1; arglist->name != NULL; i++, arglist++){

			// skip long args in single step mode
			if((cmdargs->flags & CMDARGS_FL_SINGLE) != 0
			&& strlen(arglist->name) > 1){
				continue;
			}

			// check arg for match in arglist
			if(strncmp(arg, arglist->name,
			strlen(arglist->name)) == 0){

				/* the long arg flag is set but we've found a
				single arg match */
				if(strlen(arglist->name) == 1
				&& (cmdargs->flags & CMDARGS_FL_LONGARG) != 0){
					break;
				}

				cmdargs->argid = i;
				break;
			}
		}
	}

	// found argument
	if(cmdargs->argid > 0){
		cmdargs->arg = arg;
		cmdargs->opt = NULL;
		cmdargs->flags |= CMDARGS_FL_CALLMASK(arglist->flags);
		arg += strlen(arglist->name);

                // argument accepts options
		if((cmdargs->flags & CMDARGS_FL_OPT) != 0){

			// option in same arg
			if(*arg != '\0'){

				// option set after ":="
				if(strchr(":=", *arg)){
					arg++;
					cmdargs->opt = arg;

				// error setting arg/option
				}else{
					cmdargs->opt = NULL;
				}

			// option in next arg
			}else if(argv[cmdargs->idx + 1] != NULL
			&& *argv[cmdargs->idx + 1] != '\0'
			&& *argv[cmdargs->idx + 1] != '-'){
                                cmdargs->idx++;
				cmdargs->opt = argv[cmdargs->idx];
			}
		}

		// single step through this arg
		if(strlen(arglist->name) == 1 && *arg != '\0'
		&& (cmdargs->flags & CMDARGS_FL_OPTREQ) != CMDARGS_FL_OPTREQ
		&& cmdargs->opt == NULL){
			cmdargs->flags |= CMDARGS_FL_SINGLE;
		}else{
			cmdargs->flags &= ~CMDARGS_FL_SINGLE;
		}

        // no valid argument found
	}else{
		cmdargs->flags = 0;
		cmdargs->opt = NULL;
	}

	return cmdargs->argid;
}
