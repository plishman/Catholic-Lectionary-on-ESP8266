/* DotFactoryDLL.h

   Declares the functions to be imported by our application, and exported by our
   DLL, in a flexible and elegant way.
*/
//#include <stdint.h>
//#include <windows.h>
//#include <math.h>
//#include <glib.h>
//#include <glib-object.h>
//#include <pango/pangocairo.h>
////#include <pango/pangowin32.h>
//#include <cairo/cairo-win32.h>
//#include <fontconfig/fontconfig.h>
//#include <pango/pangofc-font.h>
#include <stdint.h>
#include <windows.h>

#ifdef _WIN32
    /* You should define DOTFACTORYDLL_EXPORTS *only* when building the DLL. */
    #ifdef DOTFACTORYDLL_EXPORTS
        #define DOTFACTORYDLLAPI __declspec(dllexport)
    #else
        #define DOTFACTORYDLLAPI __declspec(dllimport)
    #endif

    /* Define calling convention in one place, for convenience. */
    #define DOTFACTORYDLLCALL __cdecl
#else /* _WIN32 not defined. */
  /* Define with no value on non-Windows OSes. */
  #define DOTFACTORYDLLAPI
  #define DOTFACTORYDLLCALL
#endif

/* Make sure functions are exported with C linkage under C++ compilers. */

#ifdef __cplusplus
extern "C"
{
#endif

/* Declare our Add function using the above definitions. */
DOTFACTORYDLLAPI int     DOTFACTORYDLLCALL  Add (int a, int b);

DOTFACTORYDLLAPI int DOTFACTORYDLLCALL getGlyph (const char    *font_name,    double  font_size,           //in
                                                 int            is_bold,      int     is_italic,           //0 == false, true otherwise
                                                 unsigned int   codepoint,    double  gamma,               // invert monitor gamma (2.2) = 0.45454545.
                                                 unsigned char *scan0,                                     // pointer to bitmap data for calling windows function (ARGB32 format for image)
                                                 int            bm_width,     int      bm_height,

                                                 double        *width,        double  *height,             //glyph metrics (out)
                                                 double        *advanceWidth, double  *advanceHeight,
                                                 double        *bearingX,     double  *bearingY,
                                                 double        *ascent,       double  *descent,
                                                 double        *line_gap,     double  *line_height
                                                );

#ifdef __cplusplus
} // __cplusplus defined.
#endif