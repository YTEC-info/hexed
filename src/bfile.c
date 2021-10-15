/******************************************************************************
 *
 *	bfile.c - buffered file control
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfile.h"


/*
 * Get file size
 */
long fsize(FILE *fp)
{
	long off, sz;

	// NULL FILE
	if(fp == NULL){
		errno = EBADF;
		return 0;
	}

	// get file size
	off = ftell(fp);
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	fseek(fp, off, SEEK_SET);
	return sz;
}


long bftruncate(Bfile *bf, long sz)
{	
	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return -EBADF;
	}

	// reopen file with truncate
	bf->fp = freopen(bf->name, "w+", bf->fp);
	return bfflush(bf);
}


/*
 * Allocate file buffer
 */
static void *bfallocbuf(Bfile *bf, long sz)
{
	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return NULL;
	}

	// allocate buffer
	sz = BFILE_BUF_ALIGN(sz);
	if((bf->buf = realloc(bf->buf, sz)) == NULL){
		return NULL;
	}
	
	// clear new memory
	if(sz > bf->bufsz){
		memset(bf->buf + bf->bufsz, 0, sz - bf->bufsz);
	}
	bf->bufsz = sz;
	return bf->buf;
}


/*
 * Free a Bfile
 */
static int bffree(Bfile *bf)
{
	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return -EBADF;
	}

	// free buffer
	if(bf->buf != NULL){
		free(bf->buf);
	}

	// free file name
	if(bf->name != NULL){
		free(bf->name);
	}
	free(bf);
	return 0;
}


/*
 * Flush buffer data to the file
 */
int bfflush(Bfile *bf)
{
	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return -EBADF;
	}

	// Check for file changes
	if(!(bf->flags & BFILE_FL_DIRTY)){
		return 0;
	}
	
	// write file
	errno = 0;
	fseek(bf->fp, 0, SEEK_SET);
	if(fwrite(bf->buf, 1, bf->size, bf->fp) == bf->size){
		bf->flags &= ~BFILE_FL_DIRTY;
	}
	fflush(bf->fp);
	return errno;
}


/*
 * Opens a Buffered File
 */
Bfile *bfopen(char *name, char *mode)
{
	Bfile *bf;

	// NULL file name
	if(name == NULL){
		errno = EINVAL;
		return NULL;
	}

	// allocate Bfile
	if((bf = malloc(sizeof(Bfile))) == NULL){
		return NULL;
	}
	memset(bf, 0, sizeof(Bfile));

	// set file name
	if((bf->name = malloc(strlen(name) + 1)) == NULL){
		bffree(bf);
		return NULL;
	}
	strcpy(bf->name, name);

	// open file
	if((bf->fp = fopen(bf->name, mode)) == NULL){
		bffree(bf);
		return NULL;
	}

	// set file buffer
	bf->buf = NULL;
	bf->size = fsize(bf->fp);
	if(bfallocbuf(bf, bf->size) == NULL){
		return NULL;
	}
	return bf;
}


/*
 * Closes a Buffered File
 */
int bfclose(Bfile *bf)
{
	int r;

	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return -EBADF;
	}
	bfflush(bf);

	// close file
	r = fclose(bf->fp);
	bffree(bf);
	return r;
}


/*
 * Remove bytes from the Buffered File
 *
 * Moves the data from the end of the buffer, then shrinks the file
 * size.
 */
long bfclip(Bfile *bf, long off, long sz)
{
	long cnt;

	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return EOF;
	}

	// negative error
	if(off < 0 || sz <= 0){
		return EOF;
	}

	// offset past EOF
	if(off > bf->size){
		return EOF;
	}

	// size past EOF
	if((off + sz) > bf->size){
		sz = bf->size - off;
	}

	// remove from file
	cnt = bf->size - (off + sz);
	memmove(bf->buf + off, bf->buf + off + sz, cnt);
	bf->size -= sz;
	bf->flags |= BFILE_FL_DIRTY;
	memset(bf->buf + bf->size, 0, sz);
	return cnt;
}


/*
 * Insert bytes into the Buffered File
 *
 * Increases the file size, moves the current data and inserts the
 * new data.
 */
long bfinsert(Bfile *bf, long off, void *mem, long sz)
{
	long cnt;

	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return EOF;
	}

	// negative error
	if(off < 0 || sz <= 0){
		return EOF;
	}

	// increase buffer size
	if(bf->bufsz < (bf->size + off + sz)){
		bfallocbuf(bf, bf->size + off + sz);
	}

	// move data
	if(off < bf->size){
		cnt = bf->size - off;
		memmove(bf->buf + off + sz, bf->buf + off, cnt);
	}
	memmove(bf->buf + off, mem, sz);

	// increase file size
	if(off > bf->size){
		bf->size += off -  bf->size;
	}
	bf->size += sz;
	bf->flags |= BFILE_FL_DIRTY;
	return sz;
}


/*
 * Copies bytes from src to off
 */
long bfcopy(Bfile *bf, long off, long src, long sz)
{
	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return EOF;
	}
	
	// error: source past EOF
	if(src > bf->size){
		return EOF;
	}
	
	// adjust sz to EOF
	if((src > off) && (src + sz > bf->size)){
		sz = bf->size - src;
	}

	// adjust source/length for insert
	if(src >= off){
		if(src + sz > bf->size){
			sz = bf->size - src;
		}
		src += sz;
	}	
	
	return bfinsert(bf, off, bf->buf + src, sz);	
}


/*
 * Moves bytes from src to off
 *
 * Moves the data from src to off then removes the data from the original src
 * Moves data in chunks to save memory allocation
 */
long bfmove(Bfile *bf, long off, long src, long sz)
{
	long adj, cnt, len;

	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return EOF;
	}
	
	// error: source past EOF
	if(src > bf->size){
		return EOF;
	}
	
	// adjust sz to EOF
	if((src > off) && (src + sz > bf->size)){
		sz = bf->size - src;
	}

	// adjust source/offset
	len = BFILE_BUF_MIN > sz ? sz : BFILE_BUF_MIN;
	if(src > off){
		src += len;
		adj = len;
	}else{
		off += sz;
		adj = 0;
	}

	// move data in chunks
	for(cnt = sz; cnt; cnt -= len){

		// end of count?
		if(cnt < len){
			len = cnt;
		}

		// move data
		bfinsert(bf, off, bf->buf + src, len);
		bfclip(bf, src, len);
		off += adj;
	}
	return sz;
}


/*
 * Read Buffered File
 *
 * Reads an entire file to the buffer.
 */
long bfread(Bfile *bf)
{
	long r;

	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return EOF;
	}

	// check buffer size
	bf->size = fsize(bf->fp);
	if(bf->size > bf->bufsz){
		if(bfallocbuf(bf, bf->size) == NULL){
			return EOF;
		}
	}

	// read whole file into the buffer
	if((r = fread(bf->buf, 1, bf->size, bf->fp)) == bf->size){
		bf->flags &= ~BFILE_FL_DIRTY;
	}
	return r;
}


/*
 * Write Buffered File
 *
 * Writes data to the buffer, the buffer still needs to be flushed
 * to the file before closing or reading.
 */
long bfwrite(Bfile *bf, long off, void *mem, long sz)
{
	long r;

	// NULL Bfile
	if(bf == NULL){
		errno = EBADF;
		return EOF;
	}

	// increase buffer size
	if(bf->bufsz < off + sz){
		bfallocbuf(bf, off + sz);
	}
	memmove(bf->buf + off, mem, sz);

	// increase file size
	if(bf->size < off + sz){
		bf->size = off + sz;
	}
	bf->flags |= BFILE_FL_DIRTY;
	return sz;
}
