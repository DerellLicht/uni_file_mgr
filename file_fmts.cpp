//*************************************************************************
//  Copyright (c) 1998-2025 Daniel D. Miller                       
//  file_fmts.cpp - file-format parsers for multimedia info display.
//  This file contains all parsers which are not handled by MediaInfo.dll
//*************************************************************************

#define  OBSOLETE_PROTOCOLS

// #undef __STRICT_ANSI__
#include <windows.h>
#include <string>    //  std::wstring, sprintf
#include <fcntl.h>
#include <tchar.h>

//lint -esym(1055, atoi)

#include "common.h"
#include "commonw.h"
#include "conio_min.h"
#include "media_list.h"
#include "file_fmts.h"

static char tempstr[MAX_LINE_LEN+1] ;
static TCHAR fpath[MAX_FILE_LEN+2] ;

#define  DBUFFER_LEN    1024
static u8 dbuffer[DBUFFER_LEN] ;

//*********************************************************************
static unsigned short get2bytes(unsigned char const * p)
{
   ul2uc_t uconv ;
   
   uconv.uc[1] = *p++ ;
   uconv.uc[0] = *p ;
   return uconv.us[0] ;
}

//*********************************************************************
static unsigned get4bytes(unsigned char const * p)
{
   ul2uc_t uconv ;
   
   uconv.uc[3] = *p++ ;
   uconv.uc[2] = *p++ ;
   uconv.uc[1] = *p++ ;
   uconv.uc[0] = *p ;
   return uconv.ul ;
}

//************************************************************************
static int read_into_dbuffer(TCHAR *fname)
{
   int hdl, rbytes ;

   _stprintf(fpath, _T("%s\\%s"), base_path, fname) ;
   hdl = _topen(fpath, O_BINARY | O_RDONLY) ;
   if (hdl < 0) 
      return errno;
   
   //  read header data
   rbytes = _read(hdl, dbuffer, sizeof(dbuffer)) ;
   if (rbytes < 0) {
      close(hdl) ;
      return errno;
   }
   close(hdl) ;
   return 0;
}

//**********************************************************************
//lint -esym(751, icon_entry_t)  variable not referenced
typedef struct icon_entry_s {
   u8 Width ;        //  Cursor Width (16, 32 or 64)
   u8 Height ;       //  Cursor Height (16, 32 or 64 , most commonly = Width)
   u8 ColorCount ;   //  Number of Colors (2,16, 0=256)
   u8 Reserved   ;   //  =0
   u16 Planes    ;   //  =1
   u16 BitCount  ;   //  bits per pixel (1, 4, 8)
   u32 SizeInBytes ; //  Size of (InfoHeader + ANDbitmap + XORbitmap)
   u32 FileOffset ;  //  FilePos, where InfoHeader starts
} icon_entry_t, *icon_entry_p ;

//**********************************************************************
static int get_ico_cur_ani_info(u8 *bfr, char *mlstr, uint NumAniFrames)
{
   // hex_dump(bfr, 64);
   u16 *uptr = (u16 *) bfr ;
   if (*uptr != 0) {
      sprintf(tempstr, "offset 0 bad: 0x%04X", (u16) *uptr) ;
      sprintf(mlstr, "%-30s", tempstr) ;
      return 0;
   }
   uptr++ ;
   //  skip unused ico/cur decider; it is sometimes not correct anyway
   uptr++ ;
   u16 NumIcons = *uptr++ ;
   if (NumIcons == 0) {
      sprintf(tempstr, "No icons in file") ;
      sprintf(mlstr, "%-30s", tempstr) ;
      return 0;
   }
   
   //  get image-specific data
   //  Thoughts on ico/cur with multiple icons included...
   //  as a first pass, I'll try just ignoring this number,
   //  and looking at the first icon only...
   //  Let's see how that works...
   //  If we get icons that have different-sized images,
   //  this will get complicated...
   icon_entry_p iptr = (icon_entry_p) (char *) uptr ;
   //  It turns out, that the ico/cur header data is unreliable.
   //  I need to dive into the BMP/PNG data to get data for ico/cur
   // if (iptr->ColorCount == 0) {
   //       
   //    if (decider == 1) {  //  ico
   //       // colors = 1 << iptr->BitCount ;
   //       // printf("ICO:  %u: %u x %u, Planes: %u, BitCount: %u, colors: %u\n", 
   //       //    idx, iptr->Width, iptr->Height, iptr->Planes, iptr->BitCount, colors) ;
   //       sprintf(tempstr, "%4u x %5u, %u bpp", iptr->Width, iptr->Height, iptr->BitCount) ;
   //    }
   //    else {   //  cur
   //       // printf("CUR:  %u: %u x %u, colors: %u, dbytes: %u\n", 
   //       //    idx, iptr->Width, iptr->Height, iptr->ColorCount, iptr->SizeInBytes) ;
   //       sprintf(tempstr, "%4u x %5u, %u data bytes", iptr->Width, iptr->Height, iptr->SizeInBytes) ;
   //    }
   // }
   // else {
   //    sprintf(tempstr, "%4u x %5u, %u colors", iptr->Width, iptr->Height, iptr->ColorCount) ;
   // }
   // sprintf(mlstr, "%-30s", tempstr) ;

   // Recall that if an image is stored in BMP format, it must exclude the opening 
   // BITMAPFILEHEADER structure, whereas if it is stored in PNG format, 
   // it must be stored in its entirety.
   // 
   //  Note that the height of the BMP image must be twice 
   //  the height declared in the image directory. 
   // printf("NumIcons: %u, FileOffset: %X\n", NumIcons, iptr->FileOffset);
   
   //  biCompression field
// #define BI_RGB 0
// #define BI_RLE8 1
// #define BI_RLE4 2
// #define BI_BITFIELDS 3
// #define BI_JPEG 4
// #define BI_PNG 5
   
   PBITMAPINFOHEADER pmih = (PBITMAPINFOHEADER) &bfr[iptr->FileOffset] ;
   switch (pmih->biCompression) {
   case BI_RGB:
      if (pmih->biSize == sizeof(BITMAPINFOHEADER)) {
         // printf("BitCount: %u, ClrUsed: %u\n", pmih->biBitCount, (uint) pmih->biClrUsed);
         if (NumAniFrames > 1) {
            sprintf(tempstr, "%4u x %5u, %u bpp [%u]", 
               (uint) pmih->biWidth,
               (uint) pmih->biWidth, //  don't use biHeight: height is 2 * width, for bmp reasons
               pmih->biBitCount, NumAniFrames) ;
         }
         else if (NumIcons > 1) {
            sprintf(tempstr, "%4u x %5u, %u bpp [%u]", 
               (uint) pmih->biWidth,
               (uint) pmih->biWidth, //  don't use biHeight: height is 2 * width, for bmp reasons
               pmih->biBitCount, NumIcons) ;
         }
         else {
            sprintf(tempstr, "%4u x %5u, %u bpp", 
               (uint) pmih->biWidth,
               (uint) pmih->biWidth, //  don't use biHeight: height is 2 * width, for bmp reasons
               pmih->biBitCount) ;
         }
      } 
      else {
         sprintf(tempstr, "Incorrect BMP size [%X]", (unsigned) pmih->biSize);
      }
      break ;
      
   case BI_PNG:
      sprintf(tempstr, "data is PNG");
      break ;
      
   default:
      sprintf(tempstr, "Unsupp. comp. [%u]", (unsigned) pmih->biCompression);
      break ;
   }
   sprintf(mlstr, "%-30s", tempstr) ;
   // hex_dump(&dbuffer[iptr->FileOffset], 64);
   return 0 ;
}

//***************************************************************************
int get_ico_cur_info(TCHAR *fname, char *mlstr)
{
   int result = read_into_dbuffer(fname) ;
   if (result != 0) {
      sprintf(mlstr, "%-30s", "unreadable file") ;
      return 0 ;
   } 
   return get_ico_cur_ani_info(&dbuffer[0], mlstr, 0);
}

//***************************************************************************
//  buffers/functions for compiling size subtotals
//***************************************************************************
//  NO docs mention the duplicate HeaderSize entry,
//  but all .ani appear to have it
typedef struct anih_header_s {
   uint HeaderSize ;
   uint HeaderSize2 ;
   uint NumFrames ;
   uint NumSteps ;
   uint Width ;   //  always 0
   uint Height ;  //  always 0
   uint BitCount ;   //  number of bits/pixel ColorDepth = 2BitCount
   uint NumPlanes ;  //  always 1
   uint DisplayRate ;
   uint Flags ;   //  bit0: T:frames are icon/cursor data, F:frames are raw data
} anih_header_t, *anih_header_p ;

//**********************************************************************
//  DDM 06/27/23
//  I've decided to abandon implementation of the ani handler,
//  as it is a great deal of work with relatively little benefit.
//  This code had gotten as far as locating the first ICON frame.
//  
//  I found the following comment on Daubnet:
//  I haven't seen any implementation of ANI software on other systems 
//  than MS-Windows95 and NT. But there is no reason why it can't be done, 
//  although it is questionable, why someone would want to do that. 
//  A full implementation would have to decode BMP, CUR and ICO structures. 
//  It doesn't have to include a full-sized RIFF decoder, since the structure 
//  of ANIs is fixed. See the RIFF, BMP, ICO and CUR page for additional information.          
//**********************************************************************
//lint -esym(438, list_len)
//lint -esym(550, list_len)
//lint -esym(751, anih_header_t)
//lint -esym(759, get_ani_info)
int get_ani_info(TCHAR *fname, char *mlstr)
{
   uint *u32ptr ;
   uint data_len ;
   int result = read_into_dbuffer(fname) ;
   if (result != 0) {
      sprintf(mlstr, "%-30s", "unreadable file") ;
   } else {
      u8 *uptr = (u8 *) dbuffer ;
      if (_strnicmp((const char *)uptr, "RIFF", 4) != 0) {
         sprintf(tempstr, "No RIFF") ;
         sprintf(mlstr, "%-30s", tempstr) ;
         goto error_exit;
      }
      uptr+=4 ;   //  point to data length
      // uint *u32ptr = (uint *) uptr ;
      // uint data_len = *u32ptr ;
      // sprintf(tempstr, "data len: %u", data_len) ;
      // sprintf(mlstr, "%-30s", tempstr) ;
      // return 0;
      uptr+=4 ;
      if (_strnicmp((const char *)uptr, "ACON", 4) != 0) {
         sprintf(tempstr, "No ACON") ;
         sprintf(mlstr, "%-30s", tempstr) ;
         goto error_exit;
      }
      uptr+=4 ;
      //  we actually need to scan for 'anih', and it may not 
      //  be on a u32 boundary, at least for Blue Sky Heart files.
      //  limit search to DBUFFER_LEN, though...
      uint ulen = (uint) (uptr - &dbuffer[0]) ;
      while (LOOP_FOREVER) {
         if (_strnicmp((const char *)uptr, "anih", 4) == 0) {
            break ;
         }
         uptr++ ;
         ulen++ ;
         if (ulen >= DBUFFER_LEN) {
            sprintf(tempstr, "No anih") ;
            sprintf(mlstr, "%-30s", tempstr) ;
            goto error_exit;
         }
      }
      uptr+=4 ;
      anih_header_p anih_header = (anih_header_p) uptr;
      
      //  search for 'list', then for first 'icon' 
      uptr += anih_header->HeaderSize + 4 ;
      //   29,326  frm/stp: 13, 13, rate         256swobusy.ani
      //   36,076  frm/stp: 16, 16, rate         Ecliptic.ani
      //   15,790  frm/stp: 07, 07, LIST         Flames.ani
      //   15,846  frm/stp: 07, 12, seq          Scroll.ani
      //   31,512  frm/stp: 14, 14, LIST         y1.ani
      //   12,552  frm/stp: 16, 23, seq          zorak.ani
      // sprintf(tempstr, "frm/stp: %02u, %02u, %c%c%c%c", 
      //    anih_header->NumFrames, anih_header->NumSteps,
      //    *(uptr), *(uptr+1), *(uptr+2), *(uptr+3)) ;
      // sprintf(mlstr, "%-30s", tempstr) ;
      // uint list_len ;
      // uint max_len = 0 ;
      while (LOOP_FOREVER) {
         if (_strnicmp((const char *)uptr, "LIST", 4) == 0) {
            uptr += 4 ;
            u32ptr = (uint *) uptr ;   //  NOLINT
            // list_len = *u32ptr ;
            uptr += 4 ;
            // printf("LIST: %u ", list_len);
         }
         else if (_strnicmp((const char *)uptr, "rate", 4) == 0) {
            uptr += 4 ;
            u32ptr = (uint *) uptr ;
            data_len = *u32ptr ;
            uptr += data_len + 4 ;
            // printf("rate: %u ", data_len);
         }
         else if (_strnicmp((const char *)uptr, "fram", 4) == 0) {
            uptr += 4 ;
         }
         else if (_strnicmp((const char *)uptr, "seq ", 4) == 0) {
            uptr += 4 ;
            u32ptr = (uint *) uptr ;
            data_len = *u32ptr ;
            uptr += data_len + 4 ;
            // printf("seq: %u ", data_len);
         }
         //  ICON data is now available
         else if (_strnicmp((const char *)uptr, "icon", 4) == 0) {
            uptr += 8 ; // skip ICON header
            get_ico_cur_ani_info(uptr, mlstr, anih_header->NumFrames);
            break ;
         }
         else {
            sprintf(tempstr, "frm/stp: %02u, %02u, %c%c%c%c", 
               anih_header->NumFrames, anih_header->NumSteps,
               *(uptr), *(uptr+1), *(uptr+2), *(uptr+3)) ;
            sprintf(mlstr, "%-30s", tempstr) ;
            break ;
         }
      }
      
      //  we have found first ICON block
      
   }
   // _tprintf(_T("good exit: %s: %s\n"), fname, ascii2unicode(mlstr));
   return 0 ;
   
error_exit:
   dputsf(_T("error exit: %s: %s\n"), fname, ascii2unicode(mlstr));
   return 0 ;   
}

//************************************************************************
//  A JFIF-standard file will start with the four bytes (hex) FF D8 FF E0,
//  followed by two variable bytes (often hex 00 10), followed by 'JFIF'
//************************************************************************
// SOF0: Start Of Frame 0:
// ~~~~~~~~~~~~~~~~~~~~~~~
// 
//   - $ff, $c0 (SOF0)
//   - length (high byte, low byte), 8+components*3
//   - data precision (1 byte) in bits/sample, usually 8 (12 and 16 not
//    supported by most software)
//   - image height (2 bytes, Hi-Lo), must be >0 if DNL not supported
//   - image width (2 bytes, Hi-Lo), must be >0 if DNL not supported
//   - number of components (1 byte), usually 1 = grey scaled, 3 = color YCbCr
//    or YIQ, 4 = color CMYK)
//   - for each component: 3 bytes
//     - component id (1 = Y, 2 = Cb, 3 = Cr, 4 = I, 5 = Q)
//     - sampling factors (bit 0-3 vert., 4-7 hor.)
//     - quantization table number
// 
//  Remarks:
//   - JFIF uses either 1 component (Y, greyscaled) or 3 components (YCbCr,
//    sometimes called YUV, colour).
//************************************************************************

static char const * const jpeg_fmt[4] = {
   "Jpeg", "Jfif", "Exif", "thm"
} ;

#define  SOI_FLAG    ((unsigned short) 0xFFD8)
#define  SOF0_FLAG   ((unsigned short) 0xFFC0)
#define  SOF2_FLAG   ((unsigned short) 0xFFC2)
#define  APP0_FLAG   ((unsigned short) 0xFFE0)
#define  EXIF_FLAG   ((unsigned short) 0xFFE1)

int get_jpeg_info(TCHAR *fname, char *mlstr)
{
   int hdl, rbytes ;
   // unsigned char *hd ;
   // union ul2uc uconv ;
   // unsigned char utemp ;
   unsigned format = 0, rows, cols ;
   u8 density_units = 0xFF ;
   // unsigned short *usptr ;
   unsigned foffset = 0 ;  //  offset into file
   unsigned seglen ;
   unsigned short uflag ;
   int inbuffer ;
   // int result = 0 ;

   _stprintf(fpath, _T("%s\\%s"), base_path, fname) ;

   hdl = _topen(fpath, O_BINARY | O_RDONLY) ;
   if (hdl < 0) 
      goto jpeg_unreadable;

   rows = cols = 0 ;
   inbuffer = 1 ;
   while (inbuffer >= 0) {
      // printf("reading %5u bytes at offset %u\n", 100, foffset) ;
      rbytes = _read(hdl, dbuffer, 100) ;
      //  check for errors,
      if (rbytes < 0) {
         // result = errno ;
         break;
      } 
      //  see if we're done reading file
      else if (rbytes == 0) {
         // result = 0 ;
         break;
      }

      uflag = get2bytes(dbuffer) ;  
      switch (uflag) {
      case 0:  //  probably end of file
         inbuffer = -1 ; //  break out of parse loop, to read file
         // result = 0 ;
         break;
         
      case SOF0_FLAG:
      case SOF2_FLAG:
         seglen = get2bytes(&dbuffer[2]) ;
         foffset += seglen + 2 ;
         lseek(hdl, foffset, SEEK_SET) ;  // seek new file position

         //  extract the data that I want
         density_units = dbuffer[4] ;
         cols = get2bytes(&dbuffer[5]) ;
         rows = get2bytes(&dbuffer[7]) ;
         break;
         
      case SOI_FLAG:
         // puts("found JPEG START_OF_IMAGE flag") ;
         foffset += 2 ;
         lseek(hdl, foffset, SEEK_SET) ;  // seek new file position
         break;

      case APP0_FLAG:
         format = 1 ;
         seglen = get2bytes(&dbuffer[2]) ;
         foffset += seglen + 2 ;
         lseek(hdl, foffset, SEEK_SET) ;  // seek new file position
         break;

      case EXIF_FLAG:
         format = 2 ;
         seglen = get2bytes(&dbuffer[2]) ;
         foffset += seglen + 2 ;
         lseek(hdl, foffset, SEEK_SET) ;  // seek new file position
         break;

      default:
         seglen = get2bytes(&dbuffer[2]) ;
         // printf("found 0x%04X flag, segment length = %5u bytes\n", uflag, seglen) ;
         foffset += seglen + 2 ;
         lseek(hdl, foffset, SEEK_SET) ;  // seek new file position
         break;
      }  //  switch uflag 

   }  //  outer loop to read more buffer data
   close(hdl) ;

   //  build the return string
   if (rows == 0  &&  cols == 0) {
      sprintf(mlstr, "%4s - no SOF0 segment      ", jpeg_fmt[format]) ;
   } else {     //                             "
//    sprintf(mlstr, "%4u x %5u, %4s           ", 
      sprintf(mlstr, "%4u x %5u, %u bpp %4s     ", 
         rows, cols, density_units*3, jpeg_fmt[format]) ;
   }
   return 0 ;

jpeg_unreadable:
   sprintf(mlstr, "%-30s", "unreadable jpg ") ;
   return 0 ;
}

//*********************************************************
// WebP file header:
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      'R'      |      'I'      |      'F'      |      'F'      |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           File Size                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |      'W'      |      'E'      |      'B'      |      'P'      |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 
// Extended WebP file header:
// 
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                   WebP file header (12 bytes)                 |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                      ChunkHeader('VP8X')                      |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |Rsv|I|L|E|X|A|R|                   Reserved                    |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          Canvas Width Minus One               |             ...
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// ...  Canvas Height Minus One    |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// VP8X format:  1800x1200 = 0x0708, 0x04b0, off by 1
// https://www.ietf.org/id/draft-zern-webp-07.html#name-riff-file-format
// 0000 0000 52 49 46 46 30 bb 05 00  57 45 42 50 56 50 38 58  RIFF0+.WEBPVP8X
// 0000 0010 0a 00 00 00 20 00 00 00  07 07 00 af 04 00 49 43  .... ....�.IC

// VP8 format:   576x768 = x0240, 0x0300, off by 0
// 0000 0000 52 49 46 46 4c b9 01 00  57 45 42 50 56 50 38 20  RIFFL�.WEBPVP8
// 0000 0010 40 b9 01 00 10 dd 04 9d  01 2a 40 02 00 03 3e 51  @�.��*@.>Q
//                                         |wwwww|lllll|

// Future note:
// VP8L format:
// The beginning of the header has the RIFF container. This consists of the following 21 bytes:
// 
//    String "RIFF"
//    A little-endian 32 bit value of the block length, the whole size of the block 
//    controlled by the RIFF header. Normally this equals the payload size 
//    (file size minus 8 bytes: 4 bytes for the 'RIFF' identifier and 4 bytes for 
//    storing the value itself).
//    String "WEBP" (RIFF container name).
//    String "VP8L" (chunk tag for lossless encoded image data).
//    A little-endian 32-bit value of the number of bytes in the lossless stream.
//    One byte signature 0x2f.
// 
// The first 28 bits of the bitstream specify the width and height of the image. 
// Width and height are decoded as 14-bit integers as follows:
// 
// int image_width = ReadBits(14) + 1;
// int image_height = ReadBits(14) + 1;

int get_webp_info(TCHAR *fname, char *mlstr)
{
   _stprintf(fpath, _T("%s\\%s"), base_path, fname) ;
   int result = read_into_dbuffer(fname) ;
   if (result != 0) {
      sprintf(mlstr, "%-30s", "unreadable WebP") ;
   } 
      //  first, search for the "fmt" string
   else if (strncmp((char *)  dbuffer,    "RIFF", 4) != 0  ||
            strncmp((char *) &dbuffer[8], "WEBP", 4) != 0) {
      sprintf(mlstr, "%-30s", "unknown webp format") ;
   }
   else {
      ul2uc_t uconv ;
      u8 *hd ;
      unsigned width, height ;
      switch (dbuffer[15]) {
      case 'X':
         hd = (u8 *) &dbuffer[24] ;
         uconv.ul = 0;
         uconv.uc[0] = *hd++ ;
         uconv.uc[1] = *hd++ ;
         uconv.uc[2] = *hd++ ;
         width = uconv.ul + 1;
         uconv.ul = 0;
         uconv.uc[0] = *hd++ ;
         uconv.uc[1] = *hd++ ;
         height = uconv.ul + 1;
         sprintf(mlstr, "%4u x %5u, VP8X format    ", width, height) ;
         break ;
         
      case ' ':
         if (dbuffer[23] == 0x9D  &&
             dbuffer[24] == 0x01  &&
             dbuffer[25] == 0x2A) {
            hd = (u8 *) &dbuffer[26] ;
            uconv.ul = 0;
            uconv.uc[0] = *hd++ ;
            uconv.uc[1] = *hd++ ;
            width = uconv.ul ;
            uconv.ul = 0;
            uconv.uc[0] = *hd++ ;
            uconv.uc[1] = *hd++ ;
            height = uconv.ul ;
            sprintf(mlstr, "%4u x %5u, VP8 format     ", width, height) ;
         } else {
            sprintf(mlstr, "VP8X bad sync code    ") ;
         }
         break ;
      
      case 'L':
         sprintf(mlstr, "VP8L format - need support   ") ;
         break ;

      default:
         sprintf(mlstr, "VP8%c format - need support   ", dbuffer[15]) ;
         break ;      
      }
   }
   return 0;
}

//************************************************************************
//  SVG is actually a vector file format, with image-drawing options.
//  As a first pass, I will search for the following string:
//  style="width:4257px;height:7265px;background:#222222;"
//  I don't know if all svg files have this field, 
//  so more work may be required in the future.
//************************************************************************
// <svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" 
// contentStyleType="text/css" 
// height="7265px" preserveAspectRatio="none" 
// style="width:4257px;height:7265px;background:#222222;" 
// version="1.1" viewBox="0 0 4257 7265" 
// width="4257px" zoomAndPan="magnify"><defs/><g><rect fill="#222222" 
//************************************************************************
int get_svg_info(TCHAR *fname, char *mlstr)
{
   _stprintf(fpath, _T("%s\\%s"), base_path, fname) ;
   int result = read_into_dbuffer(fname) ;
   if (result != 0) {
      sprintf(mlstr, "%-30s", "unreadable SVG") ;
      return 0;
   } 
      //  first, search for the "fmt" string
   if (strncmp((char *)  dbuffer,    "<svg", 4) != 0) {
      sprintf(mlstr, "%-30s", "unknown svg format 1") ;
      return 0 ;
   }

   //  style="width:4257px;height:7265px;background:#222222;"
   char *hd = strstr((char *) dbuffer, "style=\"width:");   
   if (hd == NULL) {
      sprintf(mlstr, "%-30s", "unknown svg format 2") ;
      return 0 ;
   }
   char *tl = (hd + 13);
   uint width = atoi(tl) ;
   tl = strstr(hd, ";height:");
   if (tl == NULL) {
      sprintf(mlstr, "%-30s", "unknown svg format 3") ;
      return 0 ;
   }
   tl += 8 ;
   uint height = atoi(tl);
   char stemp[30] ;
   sprintf(stemp, "%4u x %5u", width, height);
   sprintf(mlstr, "%-30s", stemp) ;
   return 0;
}

//************************************************************************
//  sf_1946.sid: 15176 x 27811 x 256  (3B48 x 6CA3 X 100)
// 00000000  6d 73 69 64 01 00 01 00  00 00 01 00 00 00 06 00  |msid............|
// 00000010  00 3b 48 00 00 6c a3 00  00 00 69 00 03 5c d4 00  |.;H..l....i..\..|
// 00000020  05 07 06 00 06 b3 8a 00  08 3f 72 00 0d 88 1e 00  |.........?r.....|
// 00000030  12 cd 29 00 17 a1 7e 00  25 d0 e5 00 34 1c 40 00  |..)...~.%...4.@.|
// 00000040  40 f2 30 00 64 17 0a 00  87 30 2a 00 a4 81 7e 00  |@.0.d....0*...~.|
// 00000050  d8 d5 87 01 11 f1 d4 01  32 7b 0a 01 32 7b 12 01  |........2{..2{..|
// 00000060  35 c6 72 ff d0 01 35 c6  7a ff ac 00 53 00 00 00  |5.r...5.z...S...|
// 00000070  01 00 00 00 85 00 00 00  00 00 00 00 00 00 00 00  |................|
// 00000080  ed 00 00 01 b2 ff aa 01  00 00 00 00 00 00 00 00  |................|
// 00000090  00 00 00 00 00 00 00 67  40 b9 51 65 00 00 00 80  |.......g@.Qe....|
// 000000a0  00 00 00 80 00 00 00 c4  00 00 88 e7 00 00 fd df  |................|
//************************************************************************
//  foothill.sid: 11520 x 11264 x truecolor  (2D00 x 2C00 x ???)
// 00000000  6d 73 69 64 01 00 01 00  00 00 03 00 00 00 09 00  |msid............|
// 00000010  00 2d 00 00 00 2c 00 00  00 00 8d 00 00 0d e0 00  |.-...,..........|
// 00000020  00 1a cc 00 00 28 3e 00  00 35 29 00 00 67 6e 00  |.....(>..5)..gn.|
// 00000030  00 9a 4a 00 00 cd 2d 00  01 94 6a 00 02 5c d6 00  |..J...-...j..\..|
// 00000040  03 25 68 00 06 29 c0 00  09 2a 86 00 0c 36 bc 00  |.%h..)...*...6..|
// 00000050  10 7f 89 00 14 c7 ec 00  18 a7 63 00 23 01 8d 00  |..........c.#...|
// 00000060  2d 50 5b 00 36 34 06 00  49 91 da 00 5c 86 1a 00  |-P[.64..I...\...|
// 00000070  6a 6d 18 00 88 1c 5f 00  a5 8b 48 00 ba 76 16 00  |jm...._...H..v..|
// 00000080  ba 76 1e 00 ba 76 26 ff  d0 00 ba 76 2e ff ac 00  |.v...v&....v....|
// 00000090  1e 00 00 00 01 00 00 00  b1 00 00 05 0c 00 00 09  |................|
// 000000a0  81 00 00 00 00 00 00 00  00 00 00 00 16 00 00 00  |................|
// 000000b0  15 ff aa 01 01 00 00 00  00 00 00 00 00 00 00 00  |................|
// 000000c0  00 00 00 67 68 40 30 57  77 00 00 00 80 00 00 00  |...gh@0Ww.......|
// 000000d0  80 00 00 00 d5 ff a3 82  48 c5 d5 65 4c 02 27 59  |........H..eL.'Y|
// 000000e0  51 f4 c7 70 58 1d 27 3b  14 e1 0b 64 06 37 d2 c2  |Q..pX.';...d.7..|
// 000000f0  51 8b 90 a8 96 5b 60 27  3f 96 a2 8a f2 85 ee 32  |Q....[`'?......2|
//************************************************************************
typedef struct sid_info_s {
   char msid[4] ;
   u8 unknown4[6] ;
   u8 color_clue1 ;
   u8 unknown11[3] ;
   u8 color_clue2 ;
   u32 width ;
   u32 height ;
// } sid_info_t, *sid_info_p ;
} __attribute__ ((packed)) sid_info_t ;
typedef sid_info_t *sid_info_p ;

u32 swap32(u32 invalue)
{
   ul2uc_t uconv ;
   uconv.ul = invalue ;
   u8 utemp ;
   utemp = uconv.uc[0] ;
   uconv.uc[0] = uconv.uc[3] ;
   uconv.uc[3] = utemp ;
   utemp = uconv.uc[1] ;
   uconv.uc[1] = uconv.uc[2] ;
   uconv.uc[2] = utemp ;
   return uconv.ul;
}

int get_sid_info(TCHAR *fname, char *mlstr)
{
   sid_info_p sid_info ;
   unsigned rows, cols, bpp ;

   int result = read_into_dbuffer(fname) ;
   if (result != 0) {
      sprintf(mlstr, "%-30s", "unreadable SID") ;
   } else if (strnicmp((char *) dbuffer, "msid", 4) != 0) {
      sprintf(mlstr, "%-30s", "unknown SID format") ;
   } else {
      sid_info = (sid_info_p) &dbuffer[0] ;
      cols = swap32(sid_info->width) ;
      rows = swap32(sid_info->height) ;
      bpp  = sid_info->color_clue1 * 8 ;

      sprintf(tempstr, "%4u x %5u, %u bpp", cols, rows, bpp) ;
      sprintf(mlstr, "%-30s", tempstr) ;
   }
   return 0 ;
}

//************************************************************************
//lint -esym(751, gif_info_t)  variable not referenced
typedef struct gif_info_s {
   unsigned width  : 16 ;
   unsigned height : 16 ;
   unsigned bpp : 3 ;   //  (bpp+1) = bits per pixel
   unsigned zro : 1 ;   //  always zero
   unsigned cr : 3 ; //  (cr + 1) = bits of color resolution
   unsigned M : 1 ;  //  1 if global color map follows descriptor
} gif_info_t, *gif_info_p ;

// 47 49 46 38 39 61 GIF89a
// 10 00  
// 10 00 
// 22 = 0 010 0 010
// b3 = 10 110 011

int get_gif_info(TCHAR *fname, char *mlstr)
{
   int result ;
   gif_info_p gif_info ;
   unsigned rows, cols, bpp ;

   result = read_into_dbuffer(fname) ;
   if (result != 0) {
      sprintf(mlstr, "%-30s", "unreadable GIF") ;
   } else if (strnicmp((char *) dbuffer, "gif87a", 6) != 0   &&
              strnicmp((char *) dbuffer, "gif89a", 6) != 0) {
      sprintf(mlstr, "%-30s", "unknown GIF format") ;
   } else {
      gif_info = (gif_info_p) &dbuffer[6] ;  //  look past the label
      cols = gif_info->width ;
      rows = gif_info->height ;
      bpp  = gif_info->bpp + 1 ;

      // sprintf(tempstr, "%4u x %5u, %u colors", cols, rows, (1U << bpp)) ;
      sprintf(tempstr, "%4u x %5u, %u bpp", cols, rows, bpp) ;
      sprintf(mlstr, "%-30s", tempstr) ;
   }
   return 0 ;
}

//************************************************************************
int get_bmp_info(TCHAR *fname, char *mlstr)
{
   int result ;
   unsigned width, height, bpp ;
   // BITMAPFILEHEADER *bmfh ;
   BITMAPINFO *bmi ;

   result = read_into_dbuffer(fname) ;
   if (result != 0) {
      sprintf(mlstr, "%-30s", "unreadable BMP") ;
   } else {
      //  see what we got
      // bmfh = (BITMAPFILEHEADER *) dbuffer ;
      bmi  = (BITMAPINFO *) &dbuffer[sizeof(BITMAPFILEHEADER)] ;

      width = (uint) bmi->bmiHeader.biWidth ;
      height = (uint) bmi->bmiHeader.biHeight ;
      bpp  = bmi->bmiHeader.biBitCount ;

      sprintf(tempstr, "%4u x %5u, %u bpp", width, height, bpp) ;
      sprintf(mlstr, "%-30s", tempstr) ;
   }
   return 0 ;
}

//************************************************************************
// The IHDR chunk must appear FIRST. It contains:
// 
//    Width:              4 bytes
//    Height:             4 bytes
//    Bit depth:          1 byte
//    Color type:         1 byte
//    Compression method: 1 byte
//    Filter method:      1 byte
//    Interlace method:   1 byte
//************************************************************************
// Bit depth is a single-byte integer giving the number of bits per sample or 
// per palette index (not per pixel). Valid values are 1, 2, 4, 8, and 16, 
// although not all values are allowed for all color types.
// 
// Color type is a single-byte integer that describes the interpretation of the image data. 
// Color type codes represent sums of the following values: 
// 1 (palette used), 2 (color used), and 4 (alpha channel used). 
// Valid values are 0, 2, 3, 4, and 6.
// 
// Bit depth restrictions for each color type are imposed to simplify implementations 
// and to prohibit combinations that do not compress well. 
// Decoders must support all legal combinations of bit depth and color type. 
// The allowed combinations are:
// 
//    Color    Allowed    Interpretation
//    Type    Bit Depths
//    
//    0       1,2,4,8,16  Each pixel is a grayscale sample.
//    
//    2       8,16        Each pixel is an R,G,B triple.
//                        DDM: so bpp is 3 * bit depth
//    
//    3       1,2,4,8     Each pixel is a palette index; a PLTE chunk must appear.
//    
//    4       8,16        Each pixel is a grayscale sample,
//                        followed by an alpha sample.
//    
//    6       8,16        Each pixel is an R,G,B triple, followed by an alpha sample.
//                        DDM: so bpp is 4 * bit depth

//************************************************************************
// typedef struct png_info_s {
//    unsigned width ;
//    unsigned height ;
//    unsigned char  bitDepth ;
//    unsigned char  colorType ;
// } png_info_t, *png_info_p ;

int get_png_info(TCHAR *fname, char *mlstr)
{
   int result ;
   unsigned rows, cols, bpp ;
   unsigned char bitDepth, colorType ;
   unsigned char *p ;
   unsigned bcount = 0 ;

   result = read_into_dbuffer(fname) ;
   if (result != 0) {
      sprintf(mlstr, "%-30s", "unreadable PNG") ;
   } else {
      //  scan along for the IHDR string
      p = dbuffer ;
      while (LOOP_FOREVER) {
         if (strncmp((char *) p, "IHDR", 4) == 0) {
            p += 4 ;
            break;
         }
         p++ ;
         if (++bcount == sizeof(dbuffer)) {
            p = 0 ;
            break;
         }
      }
      if (p == 0) {
         sprintf(mlstr, "%-30s", "no IHDR in PNG") ;
         return 0;
      }
      
      //  see what we got
      cols = get4bytes(p) ;  p += 4 ;
      rows = get4bytes(p) ;  p += 4 ;
      bitDepth  = *p++ ;
      colorType = *p++ ;

      switch (colorType) {
      case 2:
         bpp = 3 * bitDepth ;
         sprintf(tempstr, "%4u x %5u, %u bpp", cols, rows, bpp) ;
         break ;
         
      case 6:
         bpp = 4 * bitDepth ;
         sprintf(tempstr, "%4u x %5u, %u bpp", cols, rows, bpp) ;
         break ;
         
      // case 0:
      // case 3:
      // case 4:
      default:
         sprintf(tempstr, "%4u x %5u, [%u, %u]", cols, rows, colorType, bitDepth) ;
         break ;
      }
      sprintf(mlstr, "%-30s", tempstr) ;
   }
   return 0 ;
}

//************************************************************************
//  NOTE: this parser is no longer used; I am using the MediaInfo DLL
//        to handle all video formats, including AVI.
//        This code is retained here merely for reference.
//************************************************************************
#ifndef  OBSOLETE_PROTOCOLS
typedef struct {
   long           chunkSize;
   short          wFormatTag;
   unsigned short wChannels;
   unsigned long  dwSamplesPerSec;
   unsigned long  dwAvgBytesPerSec;
   unsigned short wBlockAlign;
   unsigned short wBitsPerSample;
   /* Note: there may be additional fields here, depending upon wFormatTag. */
} FormatChunk;
                
int get_wave_info(TCHAR *fname, char *mlstr)
{
   int result, rbytes ;
   FormatChunk *fcptr ;
   struct stat st ;
   off_t fsize ;
   char *hd ;
   unsigned srate ;
   double ptime ;

   sprintf(fpath, "%s\\%s", base_path, fname) ;

   //  get file size
   result = stat(fpath, &st) ;
   if (result < 0) {
      sprintf(mlstr, "%-30s", "cannot stat Wave file") ;
      return 0;
   }
   fsize = st.st_size ;

   result = read_into_dbuffer(fname) ;
   if (result != 0) {
      sprintf(mlstr, "%-30s", "unreadable Wave") ;
   } 
      //  first, search for the "fmt" string
   else if (strncmp((char *)  dbuffer,    "RIFF", 4) != 0  ||
            strncmp((char *) &dbuffer[8], "WAVE", 4) != 0) {
      sprintf(mlstr, "%-30s", "unknown wave format") ;
   }
   else {
      hd = (char *) &dbuffer[12] ;
      rbytes = 0 ;
      while (LOOP_FOREVER) {
         if (strncmp(hd, "fmt ", 4) == 0) {
            hd += 4 ;
            break;
         }
         hd++ ;
         //  make sure we stop before search string overruns buffer
         if (++rbytes == (sizeof(dbuffer) - 4)) {
            // return EILSEQ;
            sprintf(mlstr, "%-30s", "no fmt in Wave file") ;
            return 0;
         }
      }
      fcptr = (FormatChunk *) hd ;
      srate = fcptr->dwSamplesPerSec ;
      ptime = (double) fsize / (double) fcptr->dwAvgBytesPerSec ;
      total_ptime += ptime ;
      unsigned uplay_secs = (unsigned) ptime ;
      unsigned uplay_mins = uplay_secs / 60 ;
      uplay_secs = uplay_secs % 60 ;
      
      if (ptime < 60.0) {
         sprintf(mlstr, "%5u hz, %6.2f seconds    ", srate, ptime) ;
      } else {
         // ptime /= 60.0 ;
         sprintf(mlstr, "%5u hz, %3u:%02u minutes    ", srate, uplay_mins, uplay_secs) ;
      }
   }
   return 0;
}
#endif

#ifndef  OBSOLETE_PROTOCOLS
//************************************************************************
//  NOTE: this parser is no longer used; I am using the MediaInfo DLL
//        to handle all video formats, including AVI.
//        This code is retained here merely for reference.
//************************************************************************
typedef struct {
    DWORD  dwMicroSecPerFrame;
    DWORD  dwMaxBytesPerSec;
    DWORD  dwReserved1;
    DWORD  dwFlags;
    DWORD  dwTotalFrames;
    DWORD  dwInitialFrames;
    DWORD  dwStreams;
    DWORD  dwSuggestedBufferSize;
    DWORD  dwWidth;
    DWORD  dwHeight;
    DWORD  dwScale;
    DWORD  dwRate;
    DWORD  dwStart;
    DWORD  dwLength;
} MainAVIHeader;

//lint -esym(759, get_avi_info)
int get_avi_info(TCHAR *fname, char *mlstr)
{
   int result ;
   unsigned rows, cols, bcount ;
   double run_time ;
   MainAVIHeader *avih ;
   char *hd ;

   result = read_into_dbuffer(fname) ;
   if (result != 0) 
      goto avi_unreadable;

   //  1. check for "RIFFnnnnAVI LIST"
   if (strncmp((char *) dbuffer, "RIFF", 4) != 0  ||
       strncmp((char *) &dbuffer[8], "AVI LIST", 8) != 0) 
      goto avi_unknown;
   
   //  2. search for "hdrlavih", which is followed by MainAVIHeader
   hd = (char *) &dbuffer[16] ;
   bcount = 0 ;
   rows = cols = 0 ;
   run_time = 0.0 ;
   while (LOOP_FOREVER) {
      if (strncmp(hd, "hdrlavih", 8) == 0) {
         avih = (MainAVIHeader *) (hd+12) ;
         cols = avih->dwWidth ;
         rows = avih->dwHeight ;
         run_time = (double) (avih->dwMicroSecPerFrame / 1000000.0)
                  * (double)  avih->dwTotalFrames ;
         break;
      }
      hd++ ;
      if (++bcount >= sizeof(dbuffer))
         goto avi_no_avih;
   }

   if (run_time < 60.0) {
      sprintf(tempstr, "%4u x %5u, %.2f secs", cols, rows, run_time) ;
   } else {
      run_time /= 60.0 ;
      sprintf(tempstr, "%4u x %5u, %.2f mins", cols, rows, run_time) ;
   }
   sprintf(mlstr, "%-30s", tempstr) ;
   return 0 ;

avi_unreadable:
   sprintf(mlstr, "%-30s", "unreadable avi ") ;
   return 0 ;
avi_unknown:
   sprintf(mlstr, "%-30s", "unknown avi format") ;
   return 0 ;
avi_no_avih:
   sprintf(mlstr, "%-30s", "no hdrlavih header found") ;
   return 0 ;
}
#endif
