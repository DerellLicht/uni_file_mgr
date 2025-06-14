//***************************************************************************
//  Copyright (c) 2025  Derell Licht
//  conio_min: Template class for 32-bit console programs                        
//***************************************************************************
//  Windows console data structures
//  
// typedef struct _CONSOLE_SCREEN_BUFFER_INFO { // csbi 
//     COORD      dwSize; 
//     COORD      dwCursorPosition; 
//     WORD       wAttributes; 
//     SMALL_RECT srWindow; 
//     COORD      dwMaximumWindowSize; 
// } CONSOLE_SCREEN_BUFFER_INFO ; 
// 
// typedef struct _SMALL_RECT { // srct 
//     SHORT Left; 
//     SHORT Top; 
//     SHORT Right; 
//     SHORT Bottom; 
// } SMALL_RECT; 
// 
// typedef struct _COORD { // coord. 
//     SHORT X;      // horizontal coordinate 
//     SHORT Y;      // vertical coordinate 
// } COORD; 
//***************************************************************************
  
//lint -esym(759, dclrscr, dputs, is_redirected, dnewline, dreturn, get_scode)

#include <windows.h>
#include <stdio.h>
#include <conio.h>   // _getch(), _kbhit()
#include <tchar.h>

#include "common.h"  //  syslog()
#include "conio_min.h"

//***************************************************************************
//                GENERIC 32-BIT CONSOLE I/O FUNCTIONS
//***************************************************************************

// #define  USE_CTRL_HANDLER
#undef   USE_CTRL_HANDLER

//**********************************************************
//lint -esym(759, control_handler) 
//lint -esym(765, control_handler)
#ifdef  USE_CTRL_HANDLER
BOOL WINAPI control_handler(DWORD dwCtrlType)
{
   BOOL bSuccess;
   DWORD dwMode;

   //  restore the screen mode
   bSuccess = GetConsoleMode(hStdOut, &dwMode);
   if (!bSuccess) {
      return FALSE;
   }
   bSuccess = SetConsoleMode(hStdOut, dwMode | ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT ) ;
   if (!bSuccess) {
      return FALSE;
   }

   //  display message and do other work
   return TRUE ;
}   //lint !e715  dwCtrlType not used
#endif

//**********************************************************
// BOOL ScrollConsoleScreenBuffer(
//   HANDLE hConsoleOutput,               // handle to a console screen buffer
//   CONST SMALL_RECT *lpScrollRectangle, // address of screen buffer rect. to move
//   CONST SMALL_RECT *lpClipRectangle,   // address of affected screen buffer rect.
//   COORD dwDestinationOrigin,           // new location of screen buffer rect.
//   CONST CHAR_INFO *lpFill              // address of fill character and color
// );
//**********************************************************
void conio_min::dscroll(WORD tBG)
{
   SMALL_RECT src ;
   // SMALL_RECT dest ;
   COORD co = { 0, 0 } ;
   // CHAR_INFO ci = { ' ', tBG } ;
   CHAR_INFO ci ;
   ci.Char.AsciiChar = ' ' ;
   ci.Attributes = tBG ;

   // GetConsoleScreenBufferInfo(hStdOut, &sinfo) ;
   // src.Left   = sinfo.srWindow.Left ;
   // src.Right  = sinfo.srWindow.Right ;
   // src.Top    = sinfo.srWindow.Top + 1 ;
   // src.Bottom = sinfo.srWindow.Bottom ;

   // Jason Hood's "buffer > screen" fix
   src.Left   = 0 ;
   src.Right  = sinfo.dwSize.X - 1;
   src.Top    = 1 ;
   src.Bottom = sinfo.dwSize.Y - 1;

   // dest.Left   = sinfo.srWindow.Left ;
   // dest.Right  = sinfo.srWindow.Right ;
   // dest.Top    = sinfo.srWindow.Top ;
   // dest.Bottom = sinfo.srWindow.Bottom ;

   ScrollConsoleScreenBuffer(hStdOut, &src, 0, co, &ci) ;
}

//**********************************************************
void conio_min::dgotoxy(int x, int y)
{
   sinfo.dwCursorPosition.X = x ;
   sinfo.dwCursorPosition.Y = y ;
   SetConsoleCursorPosition(hStdOut, sinfo.dwCursorPosition) ;
}

//***************************************************************************
//  This stores CONSOLE_SCREEN_BUFFER_INFO in global var sinfo
//***************************************************************************
conio_min::conio_min() :
hStdOut (nullptr),
hStdIn (nullptr),
sinfo ({}),
redirected (false),
original_attribs (3),
init_success(false)
{
   BOOL bSuccess;
   DWORD dwMode;

   //  this doesn't work either
   // _setmode(_fileno(stdout), _O_U16TEXT);
  
   hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
   if (hStdOut == INVALID_HANDLE_VALUE) {
      syslog(_T("GetStdHandle(STD_OUTPUT_HANDLE): %s\n"), get_system_message()) ;
      init_success = false ;
      return ;
   }
   // PERR(hStdOut != INVALID_HANDLE_VALUE, "GetStdHandle");
   hStdIn = GetStdHandle(STD_INPUT_HANDLE);
   if (hStdIn == INVALID_HANDLE_VALUE) {
      syslog(_T("GetStdHandle(STD_INPUT_HANDLE): %s\n"), get_system_message()) ;
      init_success = false ;
      return ;
   }
   // PERR(hStdIn != INVALID_HANDLE_VALUE, "GetStdHandle");

   //  Put up a meaningful console title.
   //  Will this *always* succeed???
   //  This doesn't appear to work on Windows 10 with tcc/le (in 2025)
   // if (title != NULL) {
   //    bSuccess = SetConsoleTitle(title);
   //    PERR(bSuccess, "SetConsoleTitle");
   // }

   //  Unfortunately, this also fails on all Bash-window terminals
   bSuccess = GetConsoleScreenBufferInfo(hStdOut, &sinfo) ;
   // PERR(bSuccess, "GetConsoleScreenBufferInfo");
   if (!bSuccess) {
      // printf("GetConsoleScreenBufferInfo: %s\n", get_system_message()) ;
      // exit(1) ;
      //  if we cannot get console info for StdOut, 
      //  assume StdOut is redirected to a file.
      //  Most console functions, such as WriteConsole(), do not 
      //  work with redirection.
      redirected = true ;
      init_success = true ;
      return ;
   }
   // [33240] dwSize: 200x2000, cursor: 0x0, max: 200x109, window: L0, T0, R199, B49
   // syslog("dwSize: %ux%u, cursor: %u,%u, max: %ux%u, window: L%u, T%u, R%u, B%u\n",
   //    sinfo.dwSize.X, sinfo.dwSize.Y,
   //    sinfo.dwCursorPosition.X,
   //    sinfo.dwCursorPosition.Y,
   //    sinfo.dwMaximumWindowSize.X, sinfo.dwMaximumWindowSize.Y, 
   //    sinfo.srWindow.Left, sinfo.srWindow.Top, sinfo.srWindow.Right, sinfo.srWindow.Bottom);
   
   //  on systems without ANSI.SYS, this is apparently 0...
   original_attribs = sinfo.wAttributes ;
   if (original_attribs == 0) {
       original_attribs = 7 ;
   }

   /* set up mouse and window input */
   bSuccess = GetConsoleMode(hStdOut, &dwMode);
   if (!bSuccess) {
      init_success = false ;
      return ;
   }

   /* when turning off ENABLE_LINE_INPUT, you MUST also turn off */
   /* ENABLE_ECHO_INPUT. */
   // bSuccess = SetConsoleMode(hStdIn, (dwMode & ~(ENABLE_LINE_INPUT |
   //     ENABLE_ECHO_INPUT)) | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
   bSuccess = SetConsoleMode(hStdOut, 
      dwMode & (ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT)) ;
//      (dwMode & ~(ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT) )) ;
   if (!bSuccess) {
      init_success = false ;
      return ;
   }
   
   //  this doesn't work
   // _setmode(_fileno(stdout), _O_U16TEXT);

   //  this doesn't work either
   // system( "chcp 65001 >nul" );        // Set the console to expect codepage 65001 = UTF-8.
   
   //  set up Ctrl-Break handler
#ifdef  USE_CTRL_HANDLER
   SetConsoleCtrlHandler((PHANDLER_ROUTINE) control_handler, TRUE) ;
#endif   
   init_success = true ;
   // syslog(L"conio_min: constructor success\n");
}   

//**********************************************************
conio_min::~conio_min(void)
{
   sinfo.wAttributes = original_attribs ;
   SetConsoleTextAttribute(hStdOut, sinfo.wAttributes) ;
#ifdef  USE_CTRL_HANDLER
   SetConsoleCtrlHandler((PHANDLER_ROUTINE) control_handler, FALSE) ;
#endif   
   // syslog(L"conio_min: destructor success\n");
}

//*************************************************************************************
// * PURPOSE: get a single character from the standard input handle 
// *                                                                
// * INPUT: none                                                    
// *                                                                
// * RETURNS: the char received from the console                    
//*************************************************************************************
//  This function differs from get_scode() in that it does not require conio.h.
//  However, it does *not* return special keys such as function or keypad keys.
//*************************************************************************************
//lint -esym(759,get_char) -esym(765,get_char) -esym(714,get_char)
CHAR conio_min::get_char(void)
{
   DWORD dwInputMode; /* to save the input mode */
   BOOL bSuccess;
   CHAR chBuf; /* buffer to read into */
   DWORD dwRead;

   /* save the console mode */
   bSuccess = GetConsoleMode(hStdIn, &dwInputMode);
   if (!bSuccess) {
      return 0;
   }

   //  Disable line input. 
   //  Echo input must be disabled when disabling line input 
   bSuccess = SetConsoleMode(hStdIn, dwInputMode & ~ENABLE_LINE_INPUT & ~ENABLE_ECHO_INPUT);
   if (!bSuccess) {
      return 0;
   }

   /* read a character from the console input */
   bSuccess = ReadFile(hStdIn, &chBuf, sizeof(chBuf), &dwRead, 0);
   if (!bSuccess) {
      return 0;
   }

   /* restore the original console input mode */
   bSuccess = SetConsoleMode(hStdIn, dwInputMode);
   if (!bSuccess) {
      return 0;
   }

   return(chBuf);
}

//**********************************************************
WORD conio_min::get_scode(void)
{
   WORD inchr ;
   inchr = _getch() ;
   if (inchr == 0)
      {
      inchr = _getch() ;
      inchr <<= 8 ;
      }
   else if (inchr == 0xE0)
      {
      inchr = _getch() ;
      inchr <<= 8 ;
      inchr |= 0xE0 ;
      }   
   return inchr ;
}

//**********************************************************
void conio_min::dnewline(void)
{
   sinfo.dwCursorPosition.X = 0 ;
   //  *this* probably shouldn't use dwSize.Y either...
   if (sinfo.dwCursorPosition.Y >= (sinfo.dwSize.Y-1)) {
      dscroll(original_attribs) ;
      sinfo.dwCursorPosition.Y = sinfo.dwCursorPosition.Y ;
   }
   else {
      sinfo.dwCursorPosition.Y++ ;
   }

   SetConsoleCursorPosition(hStdOut, sinfo.dwCursorPosition) ;
}   
   
//**********************************************************
//  CR only, no LF
//**********************************************************
void conio_min::dreturn(void)
{
   sinfo.dwCursorPosition.X = 0 ;
   SetConsoleCursorPosition(hStdOut, sinfo.dwCursorPosition) ;
}   

//**********************************************************
void conio_min::dclrscr(void)
{
   COORD coord = { 0, 0 };
   int slen ;
   DWORD wrlen ;

   slen = sinfo.dwSize.X * sinfo.dwSize.Y ;

   FillConsoleOutputCharacter(hStdOut, ' ', slen, coord, &wrlen) ;
   FillConsoleOutputAttribute(hStdOut, original_attribs, slen, coord, &wrlen) ;

   dgotoxy(0,0) ; // home the cursor
}   

//******************************************************************************
//  Sadly, neither of these output functions handle Unicode on redirection
//******************************************************************************
void conio_min::dputs(const TCHAR *outstr)
{
   DWORD wrlen ;

   //  watch out for trouble conditions
   if (outstr == NULL  ||  *outstr == 0)
      return ;

   if (is_redirected()) {
      _tprintf(_T("%s"), outstr);
   }
   else {
      WORD slen = _tcslen(outstr) ;
      WriteConsole(hStdOut, outstr, slen, &wrlen, 0) ;
      sinfo.dwCursorPosition.X += slen ;
   }
}

//********************************************************************
//lint -esym(714, dputsf)
//lint -esym(759, dputsf)
//lint -esym(765, dputsf)
int conio_min::dputsf(const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[3000] ;
   va_list al; //lint !e522

//lint -esym(526, __builtin_va_start)
//lint -esym(628, __builtin_va_start)
   va_start(al, fmt);   //lint !e1055 !e530
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   // OutputDebugString(consoleBuffer) ;
   dputs(consoleBuffer) ;
   va_end(al);
   return 1;
}

