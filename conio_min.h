//***************************************************************************
//  Copyright (c) 2025  Derell Licht
//  conio_min.h: Template class for minimal 32-bit console programs                        
//***************************************************************************

//lint -esym(1704, conio_min::conio_min)  has private access specification

//lint -esym(516, dputsf) Symbol has arg. type conflict 
//lint -esym(515, dputsf) Symbol has arg. type conflict 

//lint -esym(1740, conio_min::hStdOut, conio_min::hStdIn)
//lint -esym(1762, conio_min::get_scode)  Member function could be made const
//lint -e1714  Member function not referenced

class conio_min {
private:
   HANDLE hStdOut ;
   HANDLE hStdIn ;

   CONSOLE_SCREEN_BUFFER_INFO sinfo ;

   bool redirected = false ;

   WORD original_attribs = 3 ;
   bool init_success ;

   //  disable the assignment operator and copy constructor
   conio_min &operator=(const conio_min &src) ;
   conio_min(const conio_min&);
   
   //  local functions in class
   void dscroll(WORD tBG);
   void dgotoxy(int x, int y);
   
public:
   conio_min();
   ~conio_min();
   bool is_redirected(void) const 
      { return redirected ; };
   bool init_okay(void) const 
      { return init_success ; };
   CHAR get_char(void);
   WORD get_scode(void);
   void dclrscr(void);
   void dnewline(void);
   void dreturn(void);
   void dputs(const TCHAR *outstr);
   int  dputsf(const TCHAR *fmt, ...);

};

   
