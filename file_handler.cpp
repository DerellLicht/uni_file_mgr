//**********************************************************************************
//  Copyright (c) 1998-2025 Daniel D. Miller                       
//  ext_lookup - file extension lookup
//  This file contains the extension-lookup and vectoring table and function.
//**********************************************************************************

#undef __STRICT_ANSI__
#ifdef USE_64BIT
#define  _WIN32_WINNT    0x0600
#endif
#include <windows.h>
#include <string>
#include <tchar.h>
#ifdef USE_64BIT
#include <fileapi.h>
#endif

#include "common.h"
#include "conio_min.h"
#include "uni_file_mgr.h"

//************************************************************************
//lint -esym(759, print_file_info) header declaration for symbol could be moved from header to module
int print_file_info(ffdata& ftemp)
{
   ffdata *fptr = &ftemp ;

   //  display directory entry
   if (fptr->dirflag) {
      dputsf(_T("[%s]\n"), fptr->filename.c_str());
   }
   //  display file entry
   else {
      dputsf(L"%-20s ", fptr->filename.c_str());
      dputsf(_T("%14s\n"), convert_to_commas(fptr->fsize, NULL));
   }
   return 0 ;  //lint !e438
}  //lint !e550
