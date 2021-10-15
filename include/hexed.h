/******************************************************************************
 *
 *	hexed.h - Command line HEXadecimal file EDitor header
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


#ifndef _HEXED_H
#define _HEXED_H


// Version number
#define	VER_MAJOR	0
#define	VER_MINOR	2
#define	VER_REVISION	2


// hexed constants
#define PNAME		"hexed"		// program name
#define TEMP_FILE	"~hexed.tmp"	// temp file name
#define CMD_MAX_CNT	0x10            // command table max count
#define OPT_BUF_SZ	0x40            // option buffer size


// Word sizes
#define WORD_64         0x08
#define WORD_32         0x04
#define WORD_16         0x02
#define WORD_8          0x01


// Command flags
#define CMD_FL_CMDLINE  0x00000001      // command set on command line
#define CMD_FL_QUIT     0x00000002      // quit after command line
#define CMD_FL_OVERFLOW 0x00000003      // command overflow


#define RETURN_ERR(err) \
	{                               \
	        errno = err;            \
	        return -err;            \
	}


// Command list
enum{
	// command line options
	CMD_HELP = 1,
	CMD_COPY_OVER,
	CMD_COPY,
	CMD_DUMP,
	CMD_ENTER,
	CMD_FIND,
	CMD_INSERT,
	CMD_MOVE_OVER,
	CMD_MOVE,
	CMD_REMOVE,
	CMD_WORD,
	// internal options
	CMD_QUIT
};


typedef struct Command Command;
typedef struct Cmdoptions Cmdoptions;


// Command options
struct Cmdoptions{
	int word;
	long dest;
	long src;
	long len;
	long cnt;
	long bytes;
	int64_t buf[OPT_BUF_SZ];
};


// Command details
struct Command{
	char *cmd;
	int id;
	int flags;
	Cmdoptions opts;
};


extern int wordsize;
extern Bfile *hexfile;

void error(int eno, const char *fmt, ...);
int hexcopy(Command *cmd, int optc, char *opt);
int hexdump(Command *cmd, int optc, char *opt);
int hexenter(Command *cmd, int optc, char *opt);
int hexhelp(Command *cmd, int optc, char *opt);
int hexinsert(Command *cmd, int optc, char *opt);
int hexmove(Command *cmd, int optc, char *opt);
int hexremove(Command *cmd, int optc, char *opt);
int hexword(Command *cmd, int optc, char *opt);
void printhex(uint64_t i, int size);

#endif	// _HEXED_H
