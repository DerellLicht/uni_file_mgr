//*****************************************************************
//  Copyright (c) 1998-2023 Daniel D. Miller                       
//  file_fmts.h - media-file parsing functions
//*****************************************************************

int get_jpeg_info(TCHAR *fname, char *mlstr);
int get_gif_info(TCHAR *fname, char *mlstr);
int get_bmp_info(TCHAR *fname, char *mlstr);
int get_png_info(TCHAR *fname, char *mlstr);
int get_ico_cur_info(TCHAR *fname, char *mlstr);
int get_ani_info(TCHAR *fname, char *mlstr);
int get_sid_info(TCHAR *fname, char *mlstr);
int get_webp_info(TCHAR *fname, char *mlstr);
int get_svg_info(TCHAR *fname, char *mlstr);
int get_mi_info(TCHAR *fname, char *mlstr);

