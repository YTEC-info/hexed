# hexed - Hexadecimal File Editor
#
# For options run 'make help'
#

.PHONY:	clean dirs help install

# C Preprocessor
CPP_FLAGS	= $(CPPFLAGS) -MMD -MP -MF $(basename $@).d

# C compile
C_INCLUDE	= $(addprefix -I ,$(inc_path))
C_FLAGS		= $(C_INCLUDE) $(CFLAGS)
COMP		= $(CC) $(CPPFLAGS) $(C_FLAGS) -c -o $@ $<

# Link
LD_FLAGS	= $(LDFLAGS)
LINK		= $(CC) $(LD_FLAGS) -o $@ $^

all:

# Project name
mod		:= hexed
mod_desc	:= Hexadecimal File Editor

inc_path	:= include

# Object files
objs		:= \
		hexed.o		\
		bfile.o		\
		bfcntl.o	\
		cmdargs.o	\
		hexcopy.o       \
		hexdump.o	\
		hexenter.o      \
		hexhelp.o       \
		hexinsert.o     \
		hexmove.o	\
		hexremove.o	\
		hexword.o

# main settings
OBJS		:= $(addprefix build/,$(objs))
MOD		:= $(addprefix bin/,$(mod))
CLEAN		:=

# Directory settings
INSTALLDIR	:= /usr/local/bin/
DIRS		:= bin/ $(sort $(dir $(OBJS)))

# General Tools
CP		:= cp
MKDIR		:= mkdir -p
RM		:= rm -f
RMDIR		:= rmdir

all: $(MOD)

$(MOD): $(OBJS)
	$(LINK)

$(OBJS): | dirs

install: $(INSTALLDIR)$(mod)

$(INSTALLDIR)$(mod): $(MOD)
	$(CP) $< $(INSTALLDIR)

uninstall:
	$(RM) $(INSTALLDIR)$(mod)

dirs:
	$(foreach d,$(DIRS),$(shell $(MKDIR) $(d)))

help_msg := "\n\
 $(mod) - $(mod_desc) - Makefile\n\n\
 options:\n\
\tmake all\tBuild the $(mod) project\n\
\tmake install\tBuild and copy to the installation directory\n\
\tmake clean\tRemove the build and bin files and directories\n\
"

help:
	@printf $(help_msg)

CLEAN		+= $(OBJS) $(MOD)
.IGNORE: clean
clean:
	$(RM) $(CLEAN)
	$(RMDIR) $(DIRS)

build/%.o: src/%.c
	$(COMP)

-include $(objs:.o=.d)
