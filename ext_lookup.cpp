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
#include "commonw.h"
#include "conio_min.h"
#include "media_list.h"
#include "file_fmts.h"

//************************************************************************
//  lookup tables for special-extension display functions
//************************************************************************
typedef struct mm_lookup_s {
   TCHAR  ext[MAX_EXT_SIZE] ;
   int (*func)(TCHAR *fname, char *mlstr) ;
} mm_lookup_t ;

static mm_lookup_t const mm_lookup[] = {
//  image formats
{ _T("ani"),  get_ani_info },
{ _T("avif"), get_mi_info },
{ _T("bmp"),  get_bmp_info },
{ _T("cur"),  get_ico_cur_info },
{ _T("ico"),  get_ico_cur_info },
{ _T("gif"),  get_gif_info },
{ _T("jpg"),  get_jpeg_info },
{ _T("thm"),  get_jpeg_info },
{ _T("png"),  get_png_info },
{ _T("sid"),  get_sid_info },
{ _T("tif"),  get_mi_info },
{ _T("tiff"), get_mi_info },
{ _T("webp"), get_webp_info },
{ _T("svg"),  get_svg_info },
//  audio formats
{ _T("flac"), get_mi_info },
{ _T("mp3"),  get_mi_info },
{ _T("ogg"),  get_mi_info },
{ _T("wav"),  get_mi_info },
{ _T("wma"),  get_mi_info },
//  video formats
{ _T("avi"),  get_mi_info },
{ _T("flv"),  get_mi_info },
{ _T("mkv"),  get_mi_info },
{ _T("mov"),  get_mi_info },
{ _T("mp4"),  get_mi_info },
{ _T("mpeg"), get_mi_info },
{ _T("mpg"),  get_mi_info },
{ _T("webm"), get_mi_info },
{ _T("wmv"),  get_mi_info },
{ _T(""), 0 }} ;

//************************************************************************
//lint -esym(759, print_media_info) header declaration for symbol could be moved from header to module
int print_media_info(ffdata& ftemp)
{
   ffdata *fptr = &ftemp ;
   char mlstr[31] = "";  //  holds media-specific data
   bool show_normal_info = true ;

   //  display directory entry
   if (fptr->dirflag) {
      dputsf(_T("%14s  "), _T(""));
      dputsf(_T("%30s"), _T(" "));
      dputsf(_T("[%s]\n"), fptr->filename.c_str());
   }

   //  display file entry
   else {
      TCHAR *p ;
      unsigned idx ;

      p = _tcsrchr((wchar_t *)fptr->filename.c_str(), _T('.')) ;
      if (p != 0  &&  _tcslen(p) <= MAX_EXT_SIZE) {
         p++ ; //  skip past the period

         for (idx=0; mm_lookup[idx].ext[0] != 0; idx++) {
            if (_tcsnicmp(p, mm_lookup[idx].ext, _tcslen(mm_lookup[idx].ext)) == 0) {
               //  call the special string generator function
               (*mm_lookup[idx].func)((wchar_t *)fptr->filename.c_str(), mlstr) ; //lint !e522
               show_normal_info = false ;
               break;
            }
         }
      }

      //  show file size
      // char *c2c = convert_to_commas(fptr->fsize, NULL) ;
      // dputsf(_T("%14s  "), ascii2unicode(c2c));
      dputsf(_T("%14s  "), convert_to_commas(fptr->fsize, NULL));

      //  process multimedia display
      if (!show_normal_info) {
         dputsf(_T("%-30s"), ascii2unicode(mlstr));
      } 
      //  display normal file listing
      else {
         dputsf(_T("%30s"), _T(" "));
      }

      //  format filename as required
      dputsf(L"%s\n", fptr->filename.c_str());
   }
   return 0 ;  //lint !e438
}  //lint !e550
