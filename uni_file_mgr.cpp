//**********************************************************************************
//  Copyright (c) 1998-2025 Derell Licht
//  media_list.cpp - list info about various media files
//**********************************************************************************

#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>
#include <memory>
#include <tchar.h>

#include "common.h"
#include "conio_min.h"
#include "uni_file_mgr.h"
#include "qualify.h"

//lint -esym(1058, print_file_info)

//  V1.01  Convert to Unicode
//  V1.02  Add support for SVG files
//  V1.03  Converted files linked list to vector/unique_ptr
#define  VER_NUMBER "1.00"

static TCHAR const * const Version = _T("UniFileMgr, Version " VER_NUMBER ) ;   //lint !e707

//  per Jason Hood, this turns off MinGW's command-line expansion, 
//  so we can handle wildcards like we want to.                    
//lint -e765  external '_CRT_glob' could be made static
//lint -e714  Symbol '_CRT_glob' not referenced
int _CRT_glob = 0 ;

double total_ptime = 0.0 ;

// lint -esym(534, FindClose)  // Ignoring return value of function
//lint -esym(818, filespec, argv)  //could be declared as pointing to const
// lint -e10  Expecting '}'

std::vector<ffdata> flist;

//lint -esym(843, filecount)  Variable could be declared as const
static uint filecount = 0 ;

//  name of drive+path without filenames
TCHAR base_path[MAX_FILE_LEN+1] ;
unsigned base_len ;  //  length of base_path

//lint -e129  declaration expected, identifier ignored
std::unique_ptr<conio_min> console ;

//**********************************************************************************
//  constructor for ffdata struct
//**********************************************************************************
ffdata::ffdata(
    DWORD sattrib,
    FILETIME sft,
    ULONGLONG sfsize,
    std::wstring sfilename,
    bool sdirflag
) :
attrib(sattrib),
ft(sft),
fsize(sfsize),
filename(sfilename),
dirflag(sdirflag)
{}

//**********************************************************************************
int read_files(TCHAR *filespec)
{
   WIN32_FIND_DATA fdata ; //  long-filename file struct

   HANDLE handle = FindFirstFile (filespec, &fdata);
   //  according to MSDN, Jan 1999, the following is equivalent
   //  to the preceding... unfortunately, under Win98SE, it's not...
   // handle = FindFirstFileEx(target[i], FindExInfoStandard, &fdata, 
   //                      FindExSearchNameMatch, NULL, 0) ;
   if (handle == INVALID_HANDLE_VALUE) {
      return -errno;
   }

   //  loop on find_next
   bool fn_okay = false ;
   bool done = false;
   while (!done) {
      if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_VOLID) != 0)
         fn_okay = false;
      else if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
         fn_okay = true;
      //  For directories, filter out "." and ".."
      else if (_tcscmp(fdata.cFileName, L".") == 0)
         fn_okay = false;
      else if (_tcscmp(fdata.cFileName, L"..") == 0)
         fn_okay = false;
      else
         fn_okay = true;

      if (fn_okay) {
         filecount++;

         // dputsf(_T("%s\n"), fdata.cFileName);
         // ffdata_t fttemp() ;
         //****************************************************
         //  allocate and initialize the structure
         //****************************************************
         // flist.emplace_back(ffdata_t());
        flist.emplace_back( fdata.dwFileAttributes,
                            fdata.ftCreationTime,
                           (fdata.nFileSizeHigh * 1LL<<32) + fdata.nFileSizeLow,
                            fdata.cFileName,
                           (fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false);
      }  //lint !e550  end while()

      //  search for another file
      if (FindNextFile (handle, &fdata) == 0) {
         done = true;
      }
   }

   FindClose (handle);
   return 0;
}

//********************************************************************************
//  this solution is from:
//  https://github.com/coderforlife/mingw-unicode-main/
//********************************************************************************
#if defined(__GNUC__) && defined(_UNICODE)

#ifndef __MSVCRT__
#error Unicode main function requires linking to MSVCRT
#endif

#include <wchar.h>
#include <stdlib.h>

extern int _CRT_glob;
extern 
#ifdef __cplusplus
"C" 
#endif
void __wgetmainargs(int*,wchar_t***,wchar_t***,int,int*);

#ifdef MAIN_USE_ENVP
int wmain(int argc, wchar_t *argv[], wchar_t *envp[]);
#else
int wmain(int argc, wchar_t *argv[]);
#endif

int main() 
{
   wchar_t **enpv, **argv;
   int argc, si = 0;
   __wgetmainargs(&argc, &argv, &enpv, _CRT_glob, &si); // this also creates the global variable __wargv
#ifdef MAIN_USE_ENVP
   return wmain(argc, argv, enpv);
#else
   return wmain(argc, argv);
#endif
}

#endif //defined(__GNUC__) && defined(_UNICODE)

//********************************************************************************
// static TCHAR file_spec[MAX_FILE_LEN+1] = _T("") ;
static std::wstring file_spec(L"");

int wmain(int argc, wchar_t *argv[])
{
   int result ;
   
   console = std::make_unique<conio_min>() ;
   if (!console->init_okay()) {  //lint !e530
      wprintf(L"console init failed\n");
      return 1 ;
   }
   console->dputsf(L"%s, %u-bit\n", Version, get_build_size());
   //  okay, the cause of this, is that apparently I have to use
   //  double-backslash to put a quote after a backslash...
   //  But forward slash works fine...
   // > medialist glock17\"буяновский страйкбол"
   // arg 1: [glock17"буяновский]
   // arg 2: [страйкбол]
   
   //  unicons derelict cornucopia "буяновский страйкбол"
   // > medialist glock17\\"буяновский страйкбол"
   // filespec: D:\SourceCode\Git\media_list\glock17\буяновский страйкбол\*, fcount: 3
   
   if (argc == 1) {
      file_spec = L"." ;
   }
   else if (argc == 2) {
      file_spec = argv[1] ;
   }
   else {
      console->dputsf(_T("Usage: uni_file_mgr [file_path]\n"));
      console->dputsf(_T("If file_path is not specified, current folder will be used\n"));
      return 1 ;
   }

   console->dclrscr();
   // console->dputsf(L"pre:  %s\n", file_spec);
   result = qualify(file_spec) ;
   if (result == QUAL_INV_DRIVE) {
      console->dputsf(_T("%s: %d\n"), file_spec.c_str(), result);
      return 1 ;
   }
   // console->dputsf(L"post: %s\n", file_spec);
   
   //  Extract base path from first filespec,
   //  and strip off filename
   //  base_path is used by programs such as folder-scanners,
   //  which need to access full path to current files.
   _tcscpy(base_path, file_spec.c_str()) ;
   TCHAR *strptr = _tcsrchr(base_path, _T('\\')) ;
   if (strptr != 0) {
       strptr++ ;  //lint !e613  skip past backslash, to filename
      *strptr = 0 ;  //  strip off filename
   }
   base_len = _tcslen(base_path) ;
   // printf("base path: %s\n", base_path);

   result = read_files((TCHAR *)file_spec.c_str());
   if (result < 0) {
      console->dputsf(_T("filespec: %s, %s\n"), file_spec.c_str(), strerror(-result));
      return (-result);
   }
   
   console->dputsf(_T("filespec: %s, fcount: %u\n"), file_spec.c_str(), filecount);
   if (filecount > 0) {
      //  find max filename length
      for(auto &file : flist)
      {
         calc_max_filename_len(file);
      }
      console->dputsf(L"\n"); //lint !e681  Loop is not entered
      
      for(auto &file : flist)
      {
         print_file_info(file);
      }
   }  //lint !e681 !e42 !e529
   console->dputsf(L"\n");
      
   // restore_console_attribs(); //  we don't have to do this any more!!
   return 0;
}

