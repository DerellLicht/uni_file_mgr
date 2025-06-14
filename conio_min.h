//***************************************************************************
//  Copyright (c) 1995-2025  Derell Licht
//  conio_min.h: Template for minimal 32-bit console programs                        
//***************************************************************************

void console_init(void);
void restore_console_attribs(void);
bool is_redirected(void);
CHAR get_char(void);
WORD get_scode(void);
void dclrscr(void);
void dnewline(void);
void dreturn(void);
void dputs(const TCHAR *outstr);
int  dputsf(const TCHAR *fmt, ...);
