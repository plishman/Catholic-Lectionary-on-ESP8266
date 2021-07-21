
C:/msys64/mingw64/bin/gcc.exe -c -o "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/DotFactoryDLL.o -g -I$"C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL" -pthread -mms-bitfields -IC:/msys64/clang64/include/harfbuzz -IC:/msys64/clang64/include -IC:/msys64/clang64/include/cairo -IC:/msys64/clang64/include/lzo -IC:/msys64/clang64/include/pixman-1 -IC:/msys64/clang64/include -IC:/msys64/clang64/include/freetype2 -IC:/msys64/clang64/include/libpng16 -IC:/msys64/clang64/include -IC:/msys64/clang64/include/harfbuzz -IC:/msys64/clang64/include -IC:/msys64/clang64/include/glib-2.0 -IC:/msys64/clang64/lib/glib-2.0/include -IC:/msys64/clang64/include -IC:/msys64/mingw64/include/pango-1.0 -IC:/msys64/mingw64/include/fribidi -IC:/msys64/mingw64/include -LC:/msys64/clang64/lib -LC:/msys64/mingw64/lib -lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lintl -lharfbuzz -lcairo -lfontconfig -lfreetype  -lm "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/DotFactoryDLL.c -D DOTFACTORYDLL_EXPORTS
C:/msys64/mingw64/bin/gcc.exe -o DotFactoryDLL.dll "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/DotFactoryDLL.o DotFactoryDLLResource.o -s -shared -Wl,--subsystem,windows,--out-implib,libDotFactoryDLL.a 



C:/msys64/mingw64/bin/gcc.exe -c -o "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/DotFactoryDLL.o -g -I$"C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL" -pthread -mms-bitfields -IC:/msys64/clang64/include/harfbuzz -IC:/msys64/clang64/include -IC:/msys64/clang64/include/cairo -IC:/msys64/clang64/include/lzo -IC:/msys64/clang64/include/pixman-1 -IC:/msys64/clang64/include -IC:/msys64/clang64/include/freetype2 -IC:/msys64/clang64/include/libpng16 -IC:/msys64/clang64/include -IC:/msys64/clang64/include/harfbuzz -IC:/msys64/clang64/include -IC:/msys64/clang64/include/glib-2.0 -IC:/msys64/clang64/lib/glib-2.0/include -IC:/msys64/clang64/include -IC:/msys64/mingw64/include/pango-1.0 -IC:/msys64/mingw64/include/fribidi -IC:/msys64/mingw64/include -LC:/msys64/clang64/lib -LC:/msys64/mingw64/lib -lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lintl -lharfbuzz -lcairo -lfontconfig -lfreetype  -lm "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/DotFactoryDLL.c -D DOTFACTORYDLL_EXPORTS
C:/msys64/mingw64/bin/gcc.exe -o DotFactoryDLL.dll "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/DotFactoryDLL.o DotFactoryDLLResource.o -s -shared -Wl,--subsystem,windows,--out-implib,libDotFactoryDLL.a 
C:/msys64/mingw64/bin/gcc.exe -c -o DFTestHarness.c DFTestHarness.o
C:/msys64/mingw64/bin/gcc.exe -o DFTestHarness.exe -s DFTestHarness.o -L. -lDotFactoryDLL





"C:/msys64/mingw64/bin/gcc.exe" -c -o "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/DotFactoryDLL.o -g -I$"C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL" -pthread -mms-bitfields -IC:/msys64/clang64/include/harfbuzz -IC:/msys64/clang64/include -IC:/msys64/clang64/include/cairo -IC:/msys64/clang64/include/lzo -IC:/msys64/clang64/include/pixman-1 -IC:/msys64/clang64/include -IC:/msys64/clang64/include/freetype2 -IC:/msys64/clang64/include/libpng16 -IC:/msys64/clang64/include -IC:/msys64/clang64/include/harfbuzz -IC:/msys64/clang64/include -IC:/msys64/clang64/include/glib-2.0 -IC:/msys64/clang64/lib/glib-2.0/include -IC:/msys64/clang64/include -IC:/msys64/mingw64/include/pango-1.0 -IC:/msys64/mingw64/include/fribidi -IC:/msys64/mingw64/include  -lm "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/DotFactoryDLL.c -D DOTFACTORYDLL_EXPORTS






rem C:/msys64/mingw64/bin/gcc.exe -c -o "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/cairowin32.exe "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/cairowin32.c -g -I$"C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL" -pthread -mms-bitfields -IC:/msys64/clang64/include/harfbuzz -IC:/msys64/clang64/include -IC:/msys64/clang64/include/cairo -IC:/msys64/clang64/include/lzo -IC:/msys64/clang64/include/pixman-1 -IC:/msys64/clang64/include -IC:/msys64/clang64/include/freetype2 -IC:/msys64/clang64/include/libpng16 -IC:/msys64/clang64/include -IC:/msys64/clang64/include/harfbuzz -IC:/msys64/clang64/include -IC:/msys64/clang64/include/glib-2.0 -IC:/msys64/clang64/lib/glib-2.0/include -IC:/msys64/clang64/include -IC:/msys64/mingw64/include/pango-1.0 -IC:/msys64/mingw64/include/fribidi -IC:/msys64/mingw64/include -LC:/msys64/clang64/lib -LC:/msys64/mingw64/lib -lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lintl -lharfbuzz -lcairo -lfontconfig -lfreetype -lm -Wint-conversion
rem C:/msys64/mingw64/bin/gcc.exe -c -o "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/cairowin32.exe 
rem "C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL"/cairowin32.c -g -I$"C:\Users\Phil Lishman\Documents\VM shared folder\DotFactoryDLL" -pthread -mms-bitfields 
rem C:/msys64/clang64/include 
rem C:/msys64/clang64/include/harfbuzz 
rem C:/msys64/clang64/include/cairo 
rem C:/msys64/clang64/include/lzo 
rem C:/msys64/clang64/include/pixman-1 
rem C:/msys64/clang64/include/freetype2 
rem C:/msys64/clang64/include/libpng16 
rem C:/msys64/clang64/include/harfbuzz 
rem C:/msys64/clang64/include/glib-2.0 
rem C:/msys64/clang64/lib/glib-2.0/include 
rem C:/msys64/mingw64/include/pango-1.0 
rem C:/msys64/mingw64/include/fribidi 
rem C:/msys64/mingw64/include 
rem 
rem -LC:/msys64/clang64/lib -LC:/msys64/mingw64/lib -lpangocairo-1.0 -lpango-1.0 -lgobject-2.0 -lglib-2.0 -lintl -lharfbuzz -lcairo -lfontconfig -lfreetype -lm -Wint-conversion