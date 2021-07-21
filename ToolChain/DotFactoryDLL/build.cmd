set PKG_CONFIG_PATH=C:\msys64\clang64\lib\pkgconfig;C:\msys64\mingw64\lib\pkgconfig;C:\msys64\mingw64\share\pkgconfig;C:\msys64\usr\lib\pkgconfig;C:\msys64\usr\share\pkgconfig

set COMPILER="C:/msys64/mingw64/bin/gcc.exe"

windres -i DotFactoryDLLResource.rc -o DotFactoryDLLResource.o

for /f "usebackq tokens=*" %%a in (`C:/bin/pkg-config.exe --cflags glib-2.0 cairo pango pangocairo fontconfig freetype2`) do (   
    %COMPILER% -c -o %2/%3.o -g -I$%4 %%a -lm %2/%3.c -D DOTFACTORYDLL_EXPORTS 
)

for /f "usebackq tokens=*" %%b in (`C:/bin/pkg-config.exe --libs glib-2.0 cairo pango pangocairo fontconfig freetype2`) do (   
    %COMPILER% -o %3.dll %2/%3.o DotFactoryDLLResource.o %%b -s -shared -Wl,--subsystem,windows,--out-implib,lib%3.a
)

%COMPILER% -c %5.c -o %5.o
%COMPILER% -o %5.exe -s %5.o -L. -l%3
