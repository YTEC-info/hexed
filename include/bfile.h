/******************************************************************************
 *
 *	bfile.h - Buffered file control header
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


#ifndef _BFILE_H
#define _BFILE_H


// Buffer size/alignment
#define BFILE_BUF_MIN		0x1000
#define BFILE_BUF_ALIGN(sz)	\
	((sz + BFILE_BUF_MIN - 1) / BFILE_BUF_MIN * BFILE_BUF_MIN)


// Buffered File flags
#define BFILE_FL_DIRTY	0x00000001


typedef struct Bfile Bfile;


// Buffered File
struct Bfile{
	FILE *fp;
	char *name;
	long size;
	int flags;
	long bufsz;
	char *buf;
};


long fsize(FILE *fp);
int bfflush(Bfile *bf);
Bfile *bfopen(char *name, char *mode);
int bfclose(Bfile *bf);
long bfclip(Bfile *bf, long off, long sz);
long bfcopy(Bfile *bf, long dest, long src, long sz);
long bfinsert(Bfile *bf, long off, void *mem, long sz);
long bfmove(Bfile *bf, long dest, long src, long sz);
long bfread(Bfile *bf);
long bftruncate(Bfile *bf, long sz);
long bfwrite(Bfile *bf, long off, void *mem, long sz);


#endif	// _BFILE_H
