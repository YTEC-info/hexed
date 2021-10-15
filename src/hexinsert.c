/******************************************************************************
 *
 *	hexinsert.c - hexed insert command
 *
 ******************************************************************************
 *
 * Copyright (c) 2011, B.ZaaR
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "bfile.h"
#include "hexed.h"


/*
 * Insert len words at dest
 */
static int runinsert(Command *cmd)
{
	long dest, rpt;

	// default to a 0 (zero) value for 1 count
	cmd->opts.bytes = cmd->opts.cnt * cmd->opts.word;
	if(cmd->opts.bytes == 0){
	        cmd->opts.bytes = 1 * cmd->opts.word;
	}

	// insert bytes
	dest = cmd->opts.dest;
 	for(rpt = cmd->opts.len; rpt > 0; rpt--){
		bfinsert(hexfile, dest, cmd->opts.buf, cmd->opts.bytes);
		dest += cmd->opts.bytes;
	}
	return 0;
}


/*
 * Set the insert command
 */
static int setinsert(Command *cmd, int optc, char *opt)
{
	char *s;
	int64_t *hx, v;

	// set defaults
	if(optc == 0){
	        cmd->flags |= CMD_FL_QUIT;
	}

	// NULL option
	if(opt == NULL){
		RETURN_ERR(EINVAL);
	}
	v = strtoll(opt, &s, 0x10);

	// no value set in option
	if(s == opt){
		RETURN_ERR(EINVAL);
	}

	// set options
	switch(optc){

	case 0:
		// set destination
		cmd->opts.dest = v;
 		optc++;
 		break;

 	case 1:
		// set count (len)
		cmd->opts.len = v;
		optc++;
		break;

	default:
		// set values
		if(optc > 1 && cmd->opts.cnt < OPT_BUF_SZ){
			hx = cmd->opts.buf;
			switch(cmd->opts.word){
			case WORD_64:
	  		        *(hx + cmd->opts.cnt) = v;
				break;

			case WORD_32:
				*((int32_t *)hx + cmd->opts.cnt) = v;
				break;

			case WORD_16:
				*((int16_t *)hx + cmd->opts.cnt) = v;
				break;

			case WORD_8:
	  			*((int8_t *)hx + cmd->opts.cnt) = v;
				break;

			default:
	          	        break;
			}
			cmd->opts.cnt++;
			optc++;

		// buffer overflow
		}else{
			error(E2BIG, "Insert error: too many values set\n");
			//RETURN_ERR(E2BIG);
		}
	}
	return optc;
}


/*
 * Insert command
 */
int hexinsert(Command *cmd, int optc, char *opt)
{
	// invalid command
	if(cmd == NULL || cmd->id != CMD_INSERT){
		RETURN_ERR(EINVAL);
	}

	// set/run insert
	if(optc >= 0){
		optc = setinsert(cmd, optc, opt);
	}else{
		optc = runinsert(cmd);
	}
	return optc;
}
