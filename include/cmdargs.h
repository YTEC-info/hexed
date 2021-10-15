/******************************************************************************
 *
 *	cmdargs.h - Command line argument parsing header
 *
 ******************************************************************************
 *
 *	Public Domain - No warranties expressed or implied
 *
 *****************************************************************************/


#ifndef _CMDARGS_H
#define _CMDARGS_H


// Command arguments flags
// Flags set by the parser
#define CMDARGS_FL_SETARG	0x00000001      // argument set with '-'
#define CMDARGS_FL_LONGARG      0x00000002      // long argument set with '--'
#define CMDARGS_FL_SINGLE       0x00000004      // single stepping args

// flags to keep set between single stepping args
#define CMDARGS_FL_RESET	(CMDARGS_FL_SINGLE | CMDARGS_FL_SETARG)

// Flags set by the caller
#define CMDARGS_FL_OPT          0x00010000	// options accepted
#define CMDARGS_FL_OPTREQ       (0x00020000 | CMDARGS_FL_OPT) // opts required


#define CMDARGS_FL_CALLMASK(fl)     (fl & 0xffff0000)


typedef struct Arglist Arglist;
typedef struct Cmdargs Cmdargs;


// Argument list table
struct Arglist{
	char *name;
	int flags;
};


// Command argument details
struct Cmdargs{
	int idx;                // last argument parsed
	int flags;              // argument flags
	int argid;              // argument id in Arglist
	char *arg;              // argument string
	char *opt;              // options string
};


extern Cmdargs *cmdargs;


int parsecmdargs(char *argv[], Arglist *arglist);


#endif // _CMDARGS_H
