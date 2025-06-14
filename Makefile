# makefile for media_list app
# SHELL=cmd.exe
USE_DEBUG = NO
USE_64BIT = NO
USE_UNICODE = YES
USE_CLANG = NO
# use -static for clang and cygwin/mingw
USE_STATIC = NO

#  clang++ vs tdm g++
#  clang gives *much* clearer compiler error messages...
#  However, programs built with clang++ will require libc++.dll.
#  in order to be used elsewhere.
#  That is why the executable files are smaller than TDM ...
ifeq ($(USE_64BIT),YES)
TOOLS=d:\tdm64\bin
else
ifeq ($(USE_CLANG),YES)
TOOLS=d:\clang\bin
else
TOOLS=d:\tdm32\bin
endif
endif

ifeq ($(USE_DEBUG),YES)
CFLAGS = -Wall -g -c
CxxFLAGS = -Wall -g -c
LFLAGS = -g
else
CFLAGS = -Wall -O3 -c
CxxFLAGS = -Wall -O3 -c
LFLAGS = -s -O3
endif
CFLAGS += -Weffc++
CFLAGS += -Wno-write-strings
ifeq ($(USE_64BIT),YES)
CFLAGS += -DUSE_64BIT
CxxFLAGS += -DUSE_64BIT
endif

ifeq ($(USE_UNICODE),YES)
CFLAGS += -DUNICODE -D_UNICODE
CxxFLAGS += -DUNICODE -D_UNICODE
LiFLAGS += -dUNICODE -d_UNICODE
LFLAGS += -dUNICODE -d_UNICODE
endif

ifeq ($(USE_CLANG),YES)
CFLAGS += -DUSING_CLANG
endif
LiFLAGS += -Ider_libs
CFLAGS += -Ider_libs
CxxFLAGS += -Ider_libs
IFLAGS += -Ider_libs

ifeq ($(USE_STATIC),YES)
LFLAGS += -static
endif

CPPSRC=media_list.cpp ext_lookup.cpp file_fmts.cpp conio_min.cpp \
der_libs\common_funcs.cpp \
der_libs\common_win.cpp \
der_libs\qualify.cpp 

CXXSRC=MediaInfoDll.cxx

#  clang-tidy options
CHFLAGS = -header-filter=.*
CHTAIL = --
CHTAIL += -Ider_libs
ifeq ($(USE_64BIT),YES)
CHTAIL += -DUSE_64BIT
endif
ifeq ($(USE_UNICODE),YES)
CHTAIL += -DUNICODE -D_UNICODE
endif

LIBS=-lshlwapi -lgdi32 -lcomdlg32

OBJS = $(CPPSRC:.cpp=.o)  $(CXXSRC:.cxx=.o) 

GPP_NAME=g++
#GPP_NAME=clang++

#**************************************************************************
%.o: %.cpp
	$(TOOLS)\$(GPP_NAME) $(CFLAGS) -c $< -o $@

%.o: %.cxx
	$(TOOLS)\$(GPP_NAME) $(CxxFLAGS) -c $< -o $@

ifeq ($(USE_64BIT),NO)
BIN = MediaList.exe
else
BIN = media_list64.exe
endif

all: $(BIN)

clean:
	rm -f $(OBJS) *.exe *~ *.zip

dist:
	rm -f media_list.zip
	zip media_list.zip $(BIN) Readme.md MediaInfo.dll

wc:
	wc -l *.cpp

check:
	cmd /C "d:\clang\bin\clang-tidy.exe $(CHFLAGS) $(CPPSRC) $(CHTAIL)"

lint:
	cmd /C "c:\lint9\lint-nt +v -width(160,4) $(LiFLAGS) -ic:\lint9 mingw.lnt -os(_lint.tmp) lintdefs.cpp $(CPPSRC)"

depend: 
	makedepend $(IFLAGS) $(CPPSRC) $(CXXSRC)

$(BIN): $(OBJS)
	$(TOOLS)\$(GPP_NAME) $(OBJS) $(LFLAGS) -o $(BIN) $(LIBS) 

# DO NOT DELETE

media_list.o: der_libs/common.h conio_min.h media_list.h der_libs/qualify.h
ext_lookup.o: der_libs/common.h der_libs/commonw.h conio_min.h media_list.h
ext_lookup.o: file_fmts.h
file_fmts.o: der_libs/common.h der_libs/commonw.h conio_min.h media_list.h
file_fmts.o: file_fmts.h
conio_min.o: der_libs/common.h conio_min.h
der_libs\common_funcs.o: der_libs/common.h
der_libs\common_win.o: der_libs/common.h der_libs/commonw.h
der_libs\qualify.o: der_libs/qualify.h
MediaInfoDll.o: MediaInfoDLL.h der_libs/common.h der_libs/commonw.h
MediaInfoDll.o: media_list.h file_fmts.h
