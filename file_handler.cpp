//**********************************************************************************
//  Copyright (c) 1998-2025 Daniel D. Miller                       
//  ext_lookup - file extension lookup
//  This file contains the extension-lookup and vectoring table and function.
//**********************************************************************************

#include <windows.h>
#include <string>
#include <memory>
#include <tchar.h>
#ifdef USE_64BIT
#include <fileapi.h>
#endif

#include "common.h"
#ifndef _lint
#include "conio_min.h"
#endif
#include "uni_file_mgr.h"

static uint max_filename_len = 0 ;
//************************************************************************
//lint -esym(759, print_file_info) header declaration for symbol could be moved from header to module
void calc_max_filename_len(ffdata& ftemp)
{
   // print_file_info(file);
   uint flen = ftemp.filename.length();
   if (max_filename_len < flen) {
      max_filename_len = flen ;
   }
}  //lint !e550 !e1764

//************************************************************************
//lint -esym(759, print_file_info) header declaration for symbol could be moved from header to module
int print_file_info(ffdata& ftemp)
{
   ffdata *fptr = &ftemp ;

   //  display directory entry
   if (fptr->dirflag) {
      console->dputsf(L"[%s]\n", fptr->filename.c_str());
   }
   //  display file entry
   else {
      console->dputsf(L"%-*s ", max_filename_len, fptr->filename.c_str());
      console->dputsf(L"|%14s\n", convert_to_commas(fptr->fsize, NULL));
   }
   return 0 ;  //lint !e438
}  //lint !e550
