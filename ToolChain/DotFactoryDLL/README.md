# DotFactoryDLL - to build on windows
Install MSYS2/MinGW64 and use pacman in bash terminal to install 
* glib-2.0 
* cairo 
* pango 
* pangocairo 
* fontconfig 
* freetype2

Open workspace in VS code, open DotfactoryDLL.c and press F5 to build dll and a test harness - DFTestHarness.c - which will generate sample calls to the dll.

The file glyph.png will be output if build is successful and the test harness runs.