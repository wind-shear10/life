FOR WINDOWS:
Use the GCC compiler or another compiler available for your system.
The following folders are required for compilation:
--include
--lib
Example compilation command:
gcc -IC:\libs\SDL2\include -LC:\libs\SDL2\lib -lmingw32 -lSDL2main -lSDL2 -o my_program.exe my_program.c
            ^                   ^
            |                   |

  //path to "include" folder   //path to "lib" folder

!!!Make sure the SDL2.dll file is located in the same directory as the .exe file

FOR MACOS:
download homebrew package manager 
install SDL2 library:
brew install SDL2
use the following flag during compilation:
clang path/to/.c/file -o path/to/.exec/file `sdl2-config --cflags --libs`

