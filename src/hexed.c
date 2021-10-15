/******************************************************************************
 *
 *	hexed.c - Command line HEXadecimal file EDitor
 *
 ******************************************************************************
 *
 * Copyright (c) 2010, B.ZaaR
 *
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *	Redistributions of source code must retain the above copyright notice,
 *	this list of conditions and the following disclaimer.
 *
 *	Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 *	The names of contributors may not be used to endorse or promote
 *	products derived from this software without specific prior written
 *	permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/


#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfile.h"
#include "cmdargs.h"
#include "hexed.h"


int wordsize;
Bfile *hexfile = NULL;
Command *cmd, cmdtbl[CMD_MAX_CNT];
// Valid args
Arglist arglist[] = {
	{"?", 0},
	{"co", CMDARGS_FL_OPTREQ},
	{"c", CMDARGS_FL_OPTREQ},
	{"d", CMDARGS_FL_OPTREQ},
	{"e", CMDARGS_FL_OPTREQ},
	{"f", CMDARGS_FL_OPTREQ},
	{"i", CMDARGS_FL_OPTREQ},
	{"mo", CMDARGS_FL_OPTREQ},
	{"m", CMDARGS_FL_OPTREQ},
	{"r", CMDARGS_FL_OPTREQ},
	{"w", CMDARGS_FL_OPTREQ},
	{NULL, 0}
};


/*
 * Print error msg and exit
 */
void error(int eno, const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	exit(-eno);
}


/*
 * Print 1-8 byte hexadecimal number
 */
void printhex(uint64_t i, int size)
{
	switch(size){
	case WORD_64:
		printf("%016" PRIx64, (uint64_t)i);
		break;
	case WORD_32:
		printf("%08x", (uint32_t)i);
		break;
	case WORD_16:
		printf("%04x", (uint16_t)i);
		break;
	case WORD_8:
		printf("%02x", (uint8_t)i);
		break;
	}
}


/*
 * Load a Buffered File hexfile
 */
int loadfile(char *name)
{
	// no file name
	if(name == NULL){
		errno = ENOENT;
		return -errno;
	}

	// open file
	hexfile = bfopen(name, "r+b");
	if(hexfile == NULL && errno == ENOENT){
		// create file
		hexfile = bfopen(name, "w+b");
	}

	// file didn't open
	if(hexfile == NULL){
		return -errno;
	}
	bfread(hexfile);
	return 0;
}


/*
 * Save a buffered file hexfile
 */
int savefile(Bfile *file)
{
	// invalid Bfile
	if(file == NULL || file->fp == NULL){
	        errno = EBADF;
	        return -errno;
	}
	bftruncate(file, file->size);
	return bfflush(file);
}


/*
 * Set or run a command
 */
int hexcmd(Command *cmd, int optc, char *opt)
{
	switch(cmd->id){
	case CMD_COPY:
	        optc = hexcopy(cmd, optc, opt);
		break;
	case CMD_COPY_OVER:
	        optc = hexcopy(cmd, optc, opt);
		break;
	case CMD_DUMP:
		optc = hexdump(cmd, optc, opt);
		break;
	case CMD_ENTER:
		optc = hexenter(cmd, optc, opt);
		break;
	case CMD_FIND:
	        optc = 0;
		break;
	case CMD_HELP:
	        optc = hexhelp(cmd, optc, opt);
		break;
	case CMD_INSERT:
	        optc = hexinsert(cmd, optc, opt);
		break;
	case CMD_MOVE:
	        optc = hexmove(cmd, optc, opt);
		break;
	case CMD_MOVE_OVER:
	        optc = hexmove(cmd, optc, opt);
		break;
	case CMD_REMOVE:
	        optc = hexremove(cmd, optc,opt);
		break;
	case CMD_WORD:
	        optc = hexword(cmd, optc,opt);
		break;
	default:
		RETURN_ERR(EINVAL);
	}

	return optc;
}


/*
 * new command found in the args
 */
Command *newcmd(int cmdno)
{
	Command *cmd;

	// command buffer overflow
	if(cmdno >= CMD_MAX_CNT){
		errno = E2BIG;
		perror(PNAME);
		exit(-errno);
	}

	// set defaults
	cmd = &cmdtbl[cmdno];
	memset(cmd, sizeof(Command), 0);
	cmd->id = cmdargs->argid;
	cmd->cmd = cmdargs->arg;
	cmd->flags = CMD_FL_CMDLINE;
	cmd->opts.word = wordsize;
	return cmd;
}


/*
 * Parse the command line arguments
 */
int parseargs(char *argv[])
{
	char *fname, *opt;
	int cmdcnt, optc;

	fname = NULL;
	cmd = cmdtbl;           // 1st command is reserved
	cmdcnt = 0;
	memset(cmd, sizeof(Command), 0);
	while(parsecmdargs(argv, arglist) != -1){

		// set new command
		if(cmdargs->argid > 0){
			cmdcnt++;
			optc = 0;
			opt = cmdargs->opt;
			cmd = newcmd(cmdcnt);

		// unknown arg
		}else{
			opt = cmdargs->arg;
		}

		// set command options
		if((optc = hexcmd(cmd, optc, opt)) < 0){

			// end of range option?
			if(*opt == '-'){

				// unknown option
				if(strlen(opt) > 1 && *(opt + 1) != '-'){
					error(EINVAL, "Unknown option: %s", opt);
				}

			// might be file name
			}else if(fname == NULL){
                                fname = opt;
				loadfile(fname);

			// unexpected option
			}else{
				error(EINVAL, "Unexpected option: %s", opt);
			}
		}

		// end of command options
		if(optc <= 0){
			cmd = cmdtbl;
			optc = 0;
		}
	}
	return 0;
}


/*
 * Run arguments from command line
 */
int runargs(void)
{
	int i, quit;

	// scan for help command
	quit = 0;
	cmd = &cmdtbl[1];		// skip 1st reserved command
	for(i = 1; i < CMD_MAX_CNT && cmd->id > 0; i++, cmd++){
	        if(cmd->id == CMD_HELP){
			hexcmd(cmd, -1, NULL);
			quit = CMD_QUIT;
			break;
		}
	}

	// loop through command list
	if(quit != CMD_QUIT){
		cmd = &cmdtbl[1];	// skip 1st reserved command
		for(i = 1; i < CMD_MAX_CNT && cmd->id > 0; i++, cmd++){
			hexcmd(cmd, -1, NULL);
			if((cmd->flags & CMD_FL_QUIT) != 0){
				quit = CMD_QUIT;
			}
		}
	}

	// save file
	if(quit == CMD_QUIT && hexfile->flags == BFILE_FL_DIRTY){
	        savefile(hexfile);
	}
	return quit;
}


/*
 * hexed - Hexadecimal File Editor
 */
int main(int argc, char *argv[])
{
	wordsize = WORD_8;

	// check command line arguments
	parseargs(argv);

	// open file
	if(hexfile == NULL){
		loadfile(TEMP_FILE);
	}

	// run commands
	runargs();
	bfclose(hexfile);
	remove(TEMP_FILE);
	return 0;
}
