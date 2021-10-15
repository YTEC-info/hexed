/******************************************************************************
 *
 *	hexdump.c - hexed dump command
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "bfile.h"
#include "hexed.h"


/*
 * Dump the hex values from dest for len words
 */
static int rundump(Command *cmd)
{
	unsigned char *cur;
	int x, i;
	long off, last;

	// error - start is past EOF
	if(cmd->opts.src > hexfile->size){
		return EOF;
	}

	// show to EOF
	if(cmd->opts.bytes == 0){
		cmd->opts.bytes = hexfile->size;
	}

	// trim length to EOF
	if((cmd->opts.src + cmd->opts.bytes) > hexfile->size){
		cmd->opts.bytes = hexfile->size - cmd->opts.src;
	}

	// show file
	cur = (unsigned char *)(hexfile->buf + cmd->opts.src);
	off = cmd->opts.src;
	last = cmd->opts.src + cmd->opts.bytes;
	for(off; off < last; off += 0x10, cur += 0x10){
                printf("%08lx ", off);

		// print hex values
		for(x = 0; x < 0x10; x += cmd->opts.word){
			if(x % 8 == 0){
				printf(" ");
			}
			if(off + x < last){
				printhex(*(uint64_t *)(cur + x),
				cmd->opts.word);
			}else{
				// print space
				for(i = 0; i < cmd->opts.word; i++){
					printf("  ");
				}
			}
			printf(" ");
		}
		printf(" ");

		// print character data
		for(x = 0; x < 0x10; x++){
			if(off + x < last){
				if(*(cur + x) >= 0x20 && *(cur + x) < 0x80){
					printf("%c", *(cur + x));
				}else{
					printf(".");
				}
			}else{
				printf(" ");
			}
		}
		printf("\n");
	}
	printf("\n");
	return 0;
}


/*
 * Set the dump command
 */
static int setdump(Command *cmd, int optc, char *opt)
{
	char *s;
	long v;

	// set defaults
	if(optc == 0){
		cmd->flags |= CMD_FL_QUIT;
		cmd->opts.src = 0;
		cmd->opts.len = 0;
		cmd->opts.bytes = 0;
	}

	// NULL option
	if(opt == NULL){
		return 0;
	}
	v = strtoll(opt, &s, 0x10);

	// no value set in option
	if(s == opt){
	        return -1;
	}

	// set options
	switch(optc){
        case 0:
	        // set offset
		cmd->opts.src = v;
		optc++;
		break;

	case 1:
		// set length
		cmd->opts.len = v;
		cmd->opts.bytes = cmd->opts.len * cmd->opts.word;

	default:
		optc = 0;
	}
	return optc;
}


/*
 * Dump command
 */
int hexdump(Command *cmd, int optc, char *opt)
{
	// invalid command
	if(cmd == NULL || cmd->id != CMD_DUMP){
		return -1;
	}

	// set/run dump
	if(optc >= 0){
		optc = setdump(cmd, optc, opt);
	}else{
		optc = rundump(cmd);
	}
	return optc;
}
