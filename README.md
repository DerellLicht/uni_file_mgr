# uni_file_mgr - demo program for Unicode, MinGW, console
This application is copyright (c) 2025  Derell Licht  

- All programs here are licensed under Creative Commons CC0 1.0 Universal;  
https://creativecommons.org/publicdomain/zero/1.0/

<hr>
This program will demonstrate how to handle console input and output, as well as reading
files, in a Unicode environment, using MinGW toolchain.

<hr>

#### building the application
This application is built using the MinGW toolchain; 
I recommend the [TDM](http://tdm-gcc.tdragon.net/) distribution, 
to avoid certain issues with library accessibility. 
The makefile also requires certain Cygwin tools (rm, make, etc).

#### NOTE: this program requires my ```der_libs``` submodule
If you clone the repository without the --recursive flag, 
you can recover the submodule later, with this command:

```git submodule update --init --recursive```

