#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
//#include "DFTestHarness.h"
#include "DotFactoryDLL.h"

//int main(int argc, char **argv, char **envp)
//void main(void)
//int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)


int main(int argc, char **argv, char **envp)
{
    printf("%d\n", Add(6, 24));

    double glyph_width;
    double glyph_height;
    double glyph_advanceWidth;
    double glyph_advanceHeight;
    double glyph_bearingX;
    double glyph_bearingY;
    double font_ascent, font_descent;
    double font_linegap, font_lineheight;

    double gamma = 1/2.2;

    for (int i = 0; i < 1; i++)
      {
        int res = getGlyph ("Droid Serif", 11, FALSE, FALSE, (unsigned int)'A', gamma, NULL, 100, 200, 
            &glyph_width, &glyph_height, &glyph_advanceWidth, &glyph_advanceHeight, &glyph_bearingX, &glyph_bearingY,
            &font_ascent, &font_descent, &font_linegap, &font_lineheight);
      }
    return EXIT_SUCCESS;
}