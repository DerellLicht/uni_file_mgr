# makefile for uni_file_mgr app
# SHELL=cmd.exe
USE_DEBUG = NO
USE_64BIT = NO
USE_UNICODE = YES
USE_CLANG = YES

include ..\tool_select.mak 

ifeq ($(USE_DEBUG),YES)
CFLAGS = -Wall -g -c
LFLAGS = -g
else
CFLAGS = -Wall -O3 -c
LFLAGS = -s -O3
endif
CFLAGS += -Weffc++
CFLAGS += -Wno-write-strings

ifeq ($(USE_UNICODE),YES)
CFLAGS += -DUNICODE -D_UNICODE
LiFLAGS += -dUNICODE -d_UNICODE
LFLAGS += -dUNICODE -d_UNICODE
endif

ifeq ($(USE_CLANG),YES)
CFLAGS += -DUSING_CLANG
endif
LiFLAGS += -Ider_libs
CFLAGS += -Ider_libs
IFLAGS += -Ider_libs

# This is required for *some* versions of makedepend
IFLAGS += -DNOMAKEDEPEND

ifeq ($(USE_STATIC),YES)
LFLAGS += -static
endif

CPPSRC=uni_file_mgr.cpp file_handler.cpp \
der_libs/conio_min.cpp \
der_libs/common_funcs.cpp \
der_libs/qualify.cpp 

LINTFILES=lintdefs.cpp lintdefs.ref.h 

OBJS = $(CPPSRC:.cpp=.o)

LIBS=-lshlwapi -lcomdlg32

GNAME=g++
#GNAME=clang++

#**************************************************************************
%.o: %.cpp
	$(TOOLS)/$(GNAME) $(CFLAGS) -c $< -o $@

BIN = uni_file_mgr.exe

all: $(BIN)

clean:
	rm -f $(OBJS) *.exe *~ *.zip

dist:
	rm -f uni_file_mgr.zip
	zip uni_file_mgr.zip $(BIN) Readme.md

wc:
	wc -l $(CPPSRC)

cppc:
	cmd /C "cppcheck --project=compile_commands.json --std=c++14 --suppressions-list=./.suppress.cppcheck"

check:
	cmd /C "d:\llvm\bin\clang-tidy.exe $(CPPSRC)"

clint:
	cmd /C "python ..\ClaudeLint.py --exclude der_libs"
	
cstale:
	cmd /C "python ..\check_compile_commands_stale.py"

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) $(LINTFILES) $(CPPSRC)"

depend: 
	makedepend $(IFLAGS) $(CPPSRC)

$(BIN): $(OBJS)
	$(TOOLS)/$(GNAME) $(OBJS) $(LFLAGS) -o $(BIN) $(LIBS) 

# DO NOT DELETE

uni_file_mgr.o: der_libs/common.h der_libs/conio_min.h uni_file_mgr.h
uni_file_mgr.o: der_libs/qualify.h
file_handler.o: der_libs/common.h der_libs/conio_min.h uni_file_mgr.h
der_libs/conio_min.o: der_libs/common.h der_libs/conio_min.h
der_libs/common_funcs.o: der_libs/common.h
der_libs/qualify.o: der_libs/common.h der_libs/qualify.h
