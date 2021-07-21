/* DotFactoryDLL.c
  The function GetGlyph draws a glyph to a bitmap scan0 buffer (supplied buffer must be in ARGB32 format) 
  using the same approach as GIMP, applying gamma correction to the output bitmap.
*/
#include <stdint.h>
#include <windows.h>
#include <math.h>
#include <glib.h>
#include <glib-object.h>
#include <pango/pangocairo.h>
#include <pango/pangofc-font.h>
//#include <pango/pangowin32.h>
#include <cairo/cairo-win32.h>
//#include <cairo/cairo-platform.h>
//#include <cairo/cairo-ft.h>
#include <fontconfig/fontconfig.h>
#include <pango/pangofc-font.h>
#include <freetype/ftglyph.h>
#include "DotFactoryDLL.h"
#include <hb-ot.h>

#include <cairo-ft.h>
#include <ft2build.h>
#include FT_SFNT_NAMES_H
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BBOX_H
#include FT_TYPE1_TABLES_H




FcConfig *fontConfig;

void 
Init ()
{
  /*
  FontConfig - get list of fonts
  */
  printf ("Init: creating fontConfig\n");

  fontConfig = FcConfigCreate();

  gchar* workingDir  = g_get_current_dir();
  gchar* resourceDir = g_strjoin (NULL, workingDir, "/Fonts", (char*)0);
  
  gchar* sysFontsDir = "C:/Windows/Fonts";

  FcConfigAppFontAddDir (fontConfig, (const FcChar8*)sysFontsDir);
  FcConfigAppFontAddDir (fontConfig, (const FcChar8*)resourceDir);
  g_free (workingDir);
  g_free (resourceDir);
  FcConfigBuildFonts (fontConfig);
  FcConfigSetCurrent (fontConfig);
}

void
End ()
{
  printf ("Destroying fontConfig\n");
  FcConfigDestroy (fontConfig);
}

int DOTFACTORYDLLCALL Add(int a, int b)
{
  return (a + b);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        OutputDebugString("DLL_PROCESS_ATTACH");
        Init();
        break;

    case DLL_THREAD_ATTACH:
        OutputDebugString("DLL_THREAD_ATTACH");
        break;

    case DLL_THREAD_DETACH:
        OutputDebugString("DLL_THREAD_DETACH");
        break;

    case DLL_PROCESS_DETACH:
        OutputDebugString("DLL_PROCESS_DETACH");
        End();
        break;
    }

    return TRUE;
}


GString* utf8fromCodepoint (guint32 c) 
{
	unsigned char char0;
	unsigned char char1;
	unsigned char char2;
	unsigned char char3;
	unsigned char char4;	//PLL-22-01-2019
	unsigned char char5;	//

	if (c < 0x80) {
		char0 = c & 0x7f;					// 1 byte
		gchar u[] = { char0, '\0' };
		return g_string_new(u);
	}

	if (c >= 0x80 && c < 0x800) {					// 2 bytes
		char1 = ((c & 0x3f) | 0x80);
		char0 = ((c & 0x7c0) >> 6 | 0xc0);
		gchar u[] = { char0, char1, '\0' };
		return g_string_new(u);
	}

	if (c >= 0x800 && c < 0x10000) {				// 3 bytes
		char2 = ((c & 0x3f) | 0x80);
		char1 = (((c & 0xfc0) >> 6) | 0x80);
		char0 = (((c & 0xf000)) >> 12 | 0xe0);
		gchar u[] = { char0, char1, char2, '\0' };
		return g_string_new(u);
	}

	if (c >= 0x10000 && c < 0x200000) {				// 4 bytes
		char3 = ((c & 0x3f) | 0x80);
		char2 = (((c & 0xfc0) >> 6) | 0x80);
		char1 = (((c & 0x3f000) >> 12) | 0x80);
		char0 = (((c & 0x1c0000)) >> 18 | 0xF0);
		gchar u[] = { char0, char1, char2, char3, '\0' };
		return g_string_new(u);
	}

	//PLL-22-01-2019
	if (c >= 0x200000 && c < 0x4000000) {				// 5 bytes
		char4 = ((c & 0x3f) | 0x80);
		char3 = (((c & 0xfc0) >> 6) | 0x80);
		char2 = (((c & 0x3f000) >> 12) | 0x80);
		char1 = (((c & 0x1c0000)) >> 18 | 0x80);
		char0 = (((c & 0x3f000000)) >> 24 | 0xF8);
		gchar u[] = { char0, char1, char2, char3, char4, '\0' };
		return g_string_new(u);
	}
	
	if (c >= 0x4000000 && c < 0x7FFFFFFF) {				// 6 bytes
		char5 = ((c & 0x3f) | 0x80);
		char4 = (((c & 0xfc0) >> 6) | 0x80);
		char3 = (((c & 0x3f000) >> 12) | 0x80);
		char2 = (((c & 0x1c0000)) >> 18 | 0x80);
		char1 = (((c & 0x3f000000)) >> 24 | 0x80);
		char0 = (((c & 0xc0000000)) >> 30 | 0xFC);
		gchar u[] = { char0, char1, char2, char3, char4, char5, '\0' };
		return g_string_new(u);
	}
	
	return g_string_new("");
}


cairo_font_options_t *
text_get_font_options ( void )
{
  cairo_font_options_t *options = cairo_font_options_create ();

  cairo_font_options_set_antialias (options, CAIRO_ANTIALIAS_GRAY); //CAIRO_ANTIALIAS_DEFAULT);

  typedef enum {TEXT_HINT_STYLE_NONE, TEXT_HINT_STYLE_SLIGHT, TEXT_HINT_STYLE_MEDIUM, TEXT_HINT_STYLE_FULL} HintStyle;
  HintStyle hint_style = TEXT_HINT_STYLE_NONE;
  
  switch (hint_style)
    {
    case TEXT_HINT_STYLE_NONE:
      cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_NONE);
      break;

    case TEXT_HINT_STYLE_SLIGHT:
      cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_SLIGHT);
      break;

    case TEXT_HINT_STYLE_MEDIUM:
      cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_MEDIUM);
      break;

    case TEXT_HINT_STYLE_FULL:
      cairo_font_options_set_hint_style (options, CAIRO_HINT_STYLE_FULL);
      break;
    }

  return options;
}

typedef enum /*< skip >*/
{
  GIMP_UNIT_PIXEL   = 0,

  GIMP_UNIT_INCH    = 1,
  GIMP_UNIT_MM      = 2,
  GIMP_UNIT_POINT   = 3,
  GIMP_UNIT_PICA    = 4,

  GIMP_UNIT_END     = 5,

  GIMP_UNIT_PERCENT = 65536 /*< pdb-skip >*/
} GimpUnit;

typedef struct
{
  gdouble      factor;
  gint         digits;
  const gchar *identifier;
  const gchar *symbol;
  const gchar *abbreviation;
  const gchar *singular;
  const gchar *plural;
} GimpUnitDef;

/*
gdouble
_gimp_unit_get_factor (Gimp     *gimp,
                       GimpUnit  unit)
{
  g_return_val_if_fail (unit < (GIMP_UNIT_END + gimp->n_user_units) ||
                        (unit == GIMP_UNIT_PERCENT),
                        gimp_unit_defs[GIMP_UNIT_INCH].factor);

  if (unit < GIMP_UNIT_END)
    return gimp_unit_defs[unit].factor;

  if (unit == GIMP_UNIT_PERCENT)
    return gimp_unit_percent.factor;

  return _gimp_unit_get_user_unit (gimp, unit)->factor;
}
*/
/*  these are the built-in units
 */

static const gdouble factor[GIMP_UNIT_END] = {0.0, 1.0, 25.4, 72.0, 6.0};
// scale factors for pixel, inch, mm, point, pica to 1 inch

//static const GimpUnitDef gimp_unit_defs[GIMP_UNIT_END] =
//{
  /* pseudo unit */
//  { FALSE,  0.0, 0, "pixels",      "px", "px",
//    NC_("unit-singular", "pixel"),      NC_("unit-plural", "pixels")      },

  /* standard units */
//  { FALSE,  1.0, 2, "inches",      "''", "in",
//    NC_("unit-singular", "inch"),       NC_("unit-plural", "inches")      },

//  { FALSE, 25.4, 1, "millimeters", "mm", "mm",
//    NC_("unit-singular", "millimeter"), NC_("unit-plural", "millimeters") },

  /* professional units */
//  { FALSE, 72.0, 0, "points",      "pt", "pt",
//    NC_("unit-singular", "point"),      NC_("unit-plural", "points")      },

//  { FALSE,  6.0, 1, "picas",       "pc", "pc",
//    NC_("unit-singular", "pica"),       NC_("unit-plural", "picas")       }
//};


/*  not a unit at all but kept here to have the strings in one place
 */
//static const GimpUnitDef gimp_unit_percent =
//{
//    FALSE,  0.0, 0, "percent",     "%",  "%",
//    NC_("singular", "percent"),    NC_("plural", "percent")
//};



//const GimpUnitDef unit_defs[] =
//{
//  {  0.0, 0, "pixels",      "px", "px", "pixel",      "pixels"      },
//  {  1.0, 2, "inches",      "''", "in", "inch",       "inches"      },
//  { 25.4, 1, "millimeters", "mm", "mm", "millimeter", "millimeters" }
//};

/**
 * gimp_unit_get_factor:
 * @unit: The unit you want to know the factor of.
 *
 * A #GimpUnit's @factor is defined to be:
 *
 * distance_in_units == (@factor * distance_in_inches)
 *
 * Returns 0 for @unit == GIMP_UNIT_PIXEL.
 *
 * Returns: The unit's factor.
 **/
gdouble
gimp_unit_get_factor (GimpUnit unit)
{
  //return unit_defs[unit].factor;
  return factor[unit];
}

gdouble
gimp_units_to_points (gdouble  value,
                      GimpUnit unit,
                      gdouble  resolution)
{
  char* gimpunit_type = unit == GIMP_UNIT_PIXEL ? "GIMP_UNIT_PIXEL" :
                        unit == GIMP_UNIT_INCH ? "GIMP_UNIT_INCH" :
                        unit == GIMP_UNIT_MM ? "GIMP_UNIT_MM" :
                        unit == GIMP_UNIT_POINT ? "GIMP_UNIT_POINT" :
                        unit == GIMP_UNIT_PICA ? "GIMP_UNIT_PICA" :
                        unit == GIMP_UNIT_END ? "GIMP_UNIT_END" : "GIMP_UNIT_PERCENT";

  printf ("\ngimp_units_to_points(): value = %lf, unit = %s, resolution = %lf\n", value, gimpunit_type, resolution);

  if (unit == GIMP_UNIT_POINT) 
  {
    printf ("gimp_units_to_points(): returning value already in points: value=%lf\n", value);    
    return value;
  }

  if (unit == GIMP_UNIT_PIXEL) 
  {
    printf ("gimp_units_to_points(): returning: value * gimp_unit_get_factor (GIMP_UNIT_POINT) / resolution = %lf * ( %lf / %lf ) = %lf\n", 
            value, 
            gimp_unit_get_factor (GIMP_UNIT_POINT), 
            resolution, 
            value * (gimp_unit_get_factor (GIMP_UNIT_POINT) / resolution));

    return (value * gimp_unit_get_factor (GIMP_UNIT_POINT) / resolution);
  }

  printf ("gimp_units_to_points(): returning: value * gimp_unit_get_factor (GIMP_UNIT_POINT) / gimp_unit_get_factor (%s) = %lf * ( %lf / %lf ) = %lf\n", 
          gimpunit_type, 
          value, 
          gimp_unit_get_factor (GIMP_UNIT_POINT), 
          gimp_unit_get_factor (unit), 
          value * (gimp_unit_get_factor (GIMP_UNIT_POINT) / gimp_unit_get_factor (unit)));

  return (value *
          gimp_unit_get_factor (GIMP_UNIT_POINT) / gimp_unit_get_factor (unit));
}


cairo_surface_t *
process_transparency (cairo_surface_t *surface_text, double gamma) 
{
  cairo_format_t fmt_textsurface = cairo_image_surface_get_format(surface_text);
  
  if (fmt_textsurface != CAIRO_FORMAT_A8 && fmt_textsurface != CAIRO_FORMAT_ARGB32)
    {
      printf("process_transparency() Surface is not A8 or RGBA32 (Surface is %d)\n", fmt_textsurface);
      return NULL;
    }

  int width  = cairo_image_surface_get_width (surface_text);
  int height = cairo_image_surface_get_height (surface_text);

  cairo_surface_t *surface_output = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);

  unsigned char* p_textsurface_data   = cairo_image_surface_get_data (surface_text);
  unsigned char* p_outputsurface_data = cairo_image_surface_get_data (surface_output);

  double b_b, g_b, r_b, a_b;  // input rgba
  double b_a, g_a, r_a, a_a;  // background rgba

  //double gamma = 1.0/2.2; //0.4545.;
  double b_o, g_o, r_o, a_o;

  int pixelsize = 4;
  if (fmt_textsurface == CAIRO_FORMAT_A8)
    {
      pixelsize = 1;
      printf("\nPixel size = A8\n");
    }
  else
    {
      printf("\nPixel size = ARGB32\n");
    }
  

  for (int y = 0; y < height; y++)
    {
      //printf("y=%d", y);
      for (int x = 0; x < width; x++)
        {
            guchar* iptr = p_textsurface_data + pixelsize * ((y * width) + x);
            guchar* optr = p_outputsurface_data + 4 * ((y * width) + x);

            //printf("%02x ", *iptr);
 
            // input surface bgra

            if (fmt_textsurface == CAIRO_FORMAT_ARGB32) 
              {
                b_a = ((double)(*(iptr + 0))) / 255.0;
                g_a = ((double)(*(iptr + 1))) / 255.0;
                r_a = ((double)(*(iptr + 2))) / 255.0;
                a_a = ((double)(*(iptr + 3))) / 255.0;
                //printf("[%02lf %02lf %02lf %02lf] ", b_a, g_a, r_a, a_a);
              }
            else
              {
                b_a = 0.0;
                g_a = 0.0;
                r_a = 0.0;
                a_a = ((double)(*(iptr))) / 255.0;
                //printf("[%02lf %02lf %02lf %02lf] ", b_a, g_a, r_a, a_a);
              }

            // background bgra, assume white, opaque
            b_b = 1.0;
            g_b = 1.0;
            r_b = 1.0;
            a_b = 1.0;

            a_o = a_a + a_b * (1 - a_a);
            r_o = pow((((pow(r_a,(1/gamma)) * a_a) + (pow(r_b,(1/gamma)) * a_b * (1 - a_a))) / a_o), gamma);
            g_o = pow((((pow(g_a,(1/gamma)) * a_a) + (pow(g_b,(1/gamma)) * a_b * (1 - a_a))) / a_o), gamma);
            b_o = pow((((pow(b_a,(1/gamma)) * a_a) + (pow(b_b,(1/gamma)) * a_b * (1 - a_a))) / a_o), gamma);

            guchar ib = (guchar)(b_o * 255);
            guchar ig = (guchar)(g_o * 255);
            guchar ir = (guchar)(r_o * 255);
            guchar ia = (guchar)(a_o * 255);

            //if (ia != 255 || ir != 255 || ig != 255 || ib != 255)
              //printf("%02x %02x %02x %02x : ", ib, ig, ir, ia);

            *(optr + 0) = ib;
            *(optr + 1) = ig;
            *(optr + 2) = ir;
            *(optr + 3) = ia;
        }
        //printf("\n");
    }

    return surface_output;
}

void 
copy_to_bitmap (cairo_surface_t *surface, unsigned char *scan0, int width_pixels, int height_pixels)
{
  if (!scan0)
    return;

  cairo_format_t fmt_surface = cairo_image_surface_get_format(surface);
  
  if (fmt_surface != CAIRO_FORMAT_ARGB32)
    {
      printf("copy_to_bitmap() Source surface is not RGBA32 (Surface is %d)\n", fmt_surface);
      return;
    }
  
  printf("copy_to_bitmap() copying output surface data to windows bitmap scan0 data...");

  int x, y;
  int pixelsize = 4;  // only supports BGRA32
  unsigned char* p_surface_data = cairo_image_surface_get_data (surface);
  unsigned char* p_scan0_data = scan0;

  for (y = 0; y < height_pixels; y++)
    {
      for (x = 0; x < width_pixels; x++)
        {
            guchar* iptr = p_surface_data + pixelsize * ((y * width_pixels) + x);
            guchar* optr = p_scan0_data   + pixelsize * ((y * width_pixels) + x);
            
            *(optr + 0) = *(iptr + 0);  //B
            *(optr + 1) = *(iptr + 1);  //G
            *(optr + 2) = *(iptr + 2);  //R
            *(optr + 3) = *(iptr + 3);  //A
        }
    }
  printf("complete\n");
}

void
draw_gradient (cairo_t *cr,
               gdouble width, gdouble height)
{
  cairo_pattern_t *pat;
  pat = cairo_pattern_create_linear(0, 0, width, 0);

  cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 0);
  cairo_pattern_add_color_stop_rgba(pat, 1.0, 0, 0, 0, 1);

  cairo_rectangle(cr, 0, 0, width, height/2);
  cairo_set_source(cr, pat);
  cairo_fill(cr);  
  
  cairo_pattern_destroy(pat);
}


//void
//get_char_extents (FT_Face face,
//                  FT_UInt glyph_index,
//                  cairo_t *cr, 
//                  guint32 codepoint, 
//                  cairo_text_extents_t *extents, 
//                  const gchar* font_name, gdouble font_size, gboolean is_bold, gboolean is_italic)
//{
//  FT_Glyph  glyph; /* a handle to the glyph image */
//
//
//  //...
//  FT_Error error = FT_Load_Glyph (face, glyph_index, FT_LOAD_COMPUTE_METRICS);
//  if ( error ) { /*...*/ }
//
//  error = FT_Get_Glyph (face->glyph, &glyph);
//  if ( error ) { /*...*/ }
//
//  return face->glyph->metrics;
//
//  //https://www.cairographics.org/samples/text_extents/
//  //cairo_text_extents_t extents;
// /*
//  GString *gs_utf8 = utf8fromCodepoint (codepoint);
//  gchar *utf8 = gs_utf8->str;
//
//  double x,y;
//
//  cairo_select_font_face (cr, font_name,
//      is_italic ? CAIRO_FONT_SLANT_ITALIC : CAIRO_FONT_SLANT_NORMAL,
//      is_bold   ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
//
//  cairo_set_font_size (cr, font_size);
//  cairo_text_extents (cr, utf8, extents);
//
//  g_string_free (gs_utf8, TRUE);
//*/
///*
//  x=25.0;
//  y=150.0;
//
//  cairo_move_to (cr, x,y);
//  cairo_show_text (cr, utf8);
//*/
//  /* draw helping lines */
///*
//  cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
//  cairo_set_line_width (cr, 6.0);
//  cairo_arc (cr, x, y, 10.0, 0, 2*M_PI);
//  cairo_fill (cr);
//  cairo_move_to (cr, x,y);
//  cairo_rel_line_to (cr, 0, -extents.height);
//  cairo_rel_line_to (cr, extents.width, 0);
//  cairo_rel_line_to (cr, extents.x_bearing, -extents.y_bearing);
//  cairo_stroke (cr);
//*/
//}



/*
static void
test_advance_tt_var_nohvar (void)
{
  hb_face_t *face = hb_test_open_font_file ("fonts/SourceSansVariable-Roman-nohvar-41,C1.ttf");
  g_assert (face);
  hb_font_t *font = hb_font_create (face);
  hb_face_destroy (face);
  g_assert (font);
  hb_ot_font_set_funcs (font);

  hb_position_t x, y;
  hb_font_get_glyph_advance_for_direction(font, 2, HB_DIRECTION_LTR, &x, &y);

  g_assert_cmpint (x, ==, 520);
  g_assert_cmpint (y, ==, 0);

  hb_font_get_glyph_advance_for_direction(font, 2, HB_DIRECTION_TTB, &x, &y);

  g_assert_cmpint (x, ==, 0);
  g_assert_cmpint (y, ==, -1000);

  float coords[1] = { 500.0f };
  hb_font_set_var_coords_design (font, coords, 1);
  hb_font_get_glyph_advance_for_direction(font, 2, HB_DIRECTION_LTR, &x, &y);

  g_assert_cmpint (x, ==, 551);
  g_assert_cmpint (y, ==, 0);

  hb_font_get_glyph_advance_for_direction(font, 2, HB_DIRECTION_TTB, &x, &y);

  g_assert_cmpint (x, ==, 0);
  g_assert_cmpint (y, ==, -1000);

  hb_font_destroy (font);
}
*/

const char* getErrorMessage(FT_Error err)
{
    #undef __FTERRORS_H__
    #define FT_ERRORDEF( e, v, s )  case e: return s;
    #define FT_ERROR_START_LIST
    #define FT_ERROR_END_LIST
    switch (err) {
        #include FT_ERRORS_H
    }
    return "(Unknown error)";
}



gboolean
get_glyph_extents (gchar* text, double fontSize, PangoFont* font, cairo_t* cr, cairo_text_extents_t* extents)
{
  //PangoFont *font = pango_font_map_load_font (pangoFontmap, pangoContext, font_desc);
  
  FT_Library lib;
  //exitOnError(FT_Init_FreeType(&lib));

  // Load font
  FT_Face ft_face;
  int faceIndex = 0;
  char* font_path = "";
  //exitOnError(FT_New_Face(lib, font_path, 0, &ft_face));



  PangoFcFont *fc_font;
  //FT_Face ft_face;
  //fc_font = PANGO_FC_FONT (font);

  //ft_face = pango_fc_font_lock_face (fc_font);

  if (!ft_face) 
    {
      printf ("get_glyph_extents() Couldn't lock ft font face\n");
      return FALSE;
    }

  cairo_font_face_t* fontFace =  cairo_ft_font_face_create_for_ft_face (ft_face, 0);

  //const char* text = "Hello world";
  //int fontSize = 14;
  //cairo_font_face_t* fontFace;// = 

  // get the scaled font object
  cairo_set_font_face(cr, fontFace);
  cairo_set_font_size(cr, fontSize);
  cairo_scaled_font_t* scaled_face = cairo_get_scaled_font(cr);

  // get glyphs for the text
  cairo_glyph_t* glyphs = NULL;
  int glyph_count;
  cairo_text_cluster_t* clusters = NULL;
  int cluster_count;
  cairo_text_cluster_flags_t clusterflags;

  cairo_status_t stat = cairo_scaled_font_text_to_glyphs(scaled_face, 0, 0, text, strlen(text), 
    &glyphs, &glyph_count, &clusters, &cluster_count, &clusterflags);

  // check if conversion was successful
  if (stat == CAIRO_STATUS_SUCCESS) 
    {
      // text paints on bottom line
      //cairo_translate(cr, 0, fontSize);

      // draw each cluster
      int glyph_index = 0;
      int byte_index = 0;

      cairo_text_cluster_t* cluster = &clusters[0];
      cairo_glyph_t* clusterglyphs = &glyphs[0];

      // get extents for the glyphs in the cluster
      //cairo_text_extents_t extents;
      cairo_scaled_font_glyph_extents (scaled_face, clusterglyphs, cluster->num_glyphs, extents);
      //pango_fc_font_unlock_face (fc_font);
      return TRUE;  // return first glyph extents only
    }
  
  //pango_fc_font_unlock_face (fc_font);
  return FALSE;
}

/*
  // check if conversion was successful
  if (stat == CAIRO_STATUS_SUCCESS) 
    {
      for (int i = 0; i < cluster_count; i++) 
        {
          cairo_text_cluster_t* cluster = &clusters[i];
          cairo_glyph_t* clusterglyphs = &glyphs[glyph_index];

          // get extents for the glyphs in the cluster
          //cairo_text_extents_t extents;
          cairo_scaled_font_glyph_extents(scaled_face, clusterglyphs, cluster->num_glyphs, extents);
          return TRUE;  // return first glyph extents only
          
          
          
          // ... for later use
          
          
          // put paths for current cluster to context
          cairo_glyph_path(cr, clusterglyphs, cluster->num_glyphs);

          // draw black text with green stroke
          cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 1.0);
          cairo_fill_preserve(cr);
          cairo_set_source_rgba(cr, 0, 1, 0, 1.0);
          cairo_set_line_width(cr, 0.5);
          cairo_stroke(cr);
          
          // glyph/byte position
          glyph_index += cluster->num_glyphs;
          byte_index += cluster->num_bytes;
        }
    }
}
*/

void free_pango_item(gpointer data, gpointer user_data)
{
  pango_item_free ((PangoItem*) data);
}


int
draw_text_hb (cairo_t *cairoContext,
              gdouble  xres,
              gdouble  yres, 
              guint32  codepoint,
              gboolean b_render_glyph,
              double *LTRadvanceX, double *LTRadvanceY, double *TTBadvanceX, double *TTBadvanceY,
              double *Xbearing, double *Ybearing, double *width, double *height,
              double *ascent, double *descent, double *line_gap, double *line_height,
              const gchar* font_name, gdouble font_size, gboolean is_bold, gboolean is_italic) 
{
  //font_desc=\"Droid Serif 11\"
  
  /*
  gchar* text = "<markup><span foreground=\"black\">"
                "Let the brightness of the Lord our God be upon us and direct Thou the\n"
                "works of our hands over us; yea, the work of our hands do Thou direct.\n"
                "Alleluia."
                "</span></markup>";
  */
  /*
  gchar* text = "<markup><span foreground=\"#000000\">"
                "p p p p p p p p p p p p p"
                "</span></markup>";
  */

  gchar* lang = "en-GB";

  //FcConfig             *fontConfig;
  PangoFontMap         *pangoFontmap;
  PangoContext         *pangoContext;
  PangoFontDescription *font_desc;
  cairo_font_options_t *cairoFontOptions;


  /*
  FontConfig - get list of fonts
  */

  /*
  fontConfig = FcConfigCreate();

  gchar* workingDir  = g_get_current_dir();
  gchar* resourceDir = g_strjoin (NULL, workingDir, "/Fonts", (char*)0);
  
  gchar* sysFontsDir = "C:/Windows/Fonts";

  FcConfigAppFontAddDir (fontConfig, (const FcChar8*)sysFontsDir);
  FcConfigAppFontAddDir (fontConfig, (const FcChar8*)resourceDir);
  g_free (workingDir);
  g_free (resourceDir);
  FcConfigBuildFonts (fontConfig);
  FcConfigSetCurrent (fontConfig);
  */
  
  /*
  Fontmap and Fontdescription - Specify font to use
  */

  pangoFontmap = pango_cairo_font_map_new_for_font_type (CAIRO_FONT_TYPE_FT);
  if (! pangoFontmap)
    g_error ("You are using a Pango that has been built against a cairo "
             "that lacks the Freetype font backend");
  pango_cairo_font_map_set_resolution (PANGO_CAIRO_FONT_MAP (pangoFontmap), yres /*FONT_SIZE * PANGO_SCALE / DEVICE_DPI*/);
  
  /*
  PangoContext
  */

  pangoContext = pango_font_map_create_context (pangoFontmap);
  g_object_unref (pangoFontmap);

  cairoFontOptions = text_get_font_options ();
  pango_cairo_context_set_font_options (pangoContext, cairoFontOptions);
  cairo_font_options_destroy (cairoFontOptions);
  

  /*
  PangoLayout and Font Description
  */
  guchar* bold = is_bold ? " bold" : "";
  guchar* italic = is_italic ? " italic" : "";
  gchar* font_description_string = g_strjoin (NULL, font_name, bold, italic, (char*)0);

  printf("font description string is: [%s]\n", font_description_string);

  font_desc = pango_font_description_from_string (font_description_string);
  //font_desc = pango_font_description_from_string (font_name);
  g_return_val_if_fail (font_desc != NULL, -1);
  
  g_free(font_description_string);

  double size_gimpunits = gimp_units_to_points (font_size, GIMP_UNIT_PIXEL, yres);
  gint size = pango_units_from_double(size_gimpunits);

  /*
  gint size = pango_units_from_double (gimp_units_to_points (font_size,
                                                             GIMP_UNIT_PIXEL,
                                                             yres
                                                             ));
  */
  printf("xres=%lf, yres=%lf\ngimp_units_to_points(%lf, GIMP_UNIT_PIXEL, %lf) = %lf\nfont size(pango)=%d\n", 
    xres, yres, font_size, yres, size_gimpunits, size);                                                             

  pango_font_description_set_size (font_desc, MAX (1, size));

  GString *utf8 = utf8fromCodepoint (codepoint);
  gchar *text = utf8->str;
  printf("text:[%s]\n", text);

  //PangoContext *pangoContext = pango_cairo_font_map_create_context (pangoFontmap);

  // get measurements for character
  //get_char_extents (cairoContext, codepoint, extents, font_name, font_size, is_bold, is_italic );
  PangoFont *font = pango_font_map_load_font (pangoFontmap, pangoContext, font_desc);
  //get_glyph_extents (text, font_size, font, cairoContext, glyph_extents);


  ////PangoFcFont *fc_font;
  ////FT_Face face;
  ////fc_font = PANGO_FC_FONT (font);
  ////  
  ////face = pango_fc_font_lock_face (fc_font);
  ////if (!face) 
  ////  return 1;
    /*
  printf ("1 ");
  PangoAttrIterator *cached_iter;
  printf ("2 ");
  PangoAttrList *attrs = pango_attr_list_new ();
  printf ("3 ");
  GList *items = pango_itemize (pangoContext, text, 0, utf8->len, attrs, NULL);
  printf ("4 ");
  PangoItem *item = items->data;
  printf ("5 ");
  PangoGlyphString *glyphs = pango_glyph_string_new();
  printf ("6 ");
  pango_shape (text, utf8->len, &item->analysis, glyphs);
  printf ("7 [%d]\n", glyphs->num_glyphs);
    */

  hb_buffer_t *bufLTR;
  bufLTR = hb_buffer_create();
  hb_buffer_add_utf8(bufLTR, text, -1, 0, -1);
    
  //Set the script, language and direction of the buffer.
  hb_buffer_set_direction(bufLTR, HB_DIRECTION_LTR);
  //hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
  hb_buffer_set_language(bufLTR, hb_language_from_string("en", -1));

  hb_font_t *hb_font = pango_font_get_hb_font (font);
  hb_shape (hb_font, bufLTR, NULL, 0);

  unsigned int glyph_count;
  hb_glyph_info_t *glyph_info        = hb_buffer_get_glyph_infos (bufLTR, &glyph_count);
  hb_glyph_position_t *glyph_pos_LTR = hb_buffer_get_glyph_positions (bufLTR, &glyph_count);

  printf ("glyph count: %d\n", glyph_count);

    //if (glyphs->num_glyphs > 0) 
  if (glyph_count > 0)
    {
      hb_codepoint_t glyphid  = glyph_info[0].codepoint;
      
      hb_position_t x_advanceLTR = glyph_pos_LTR[0].x_advance;
      hb_position_t y_advanceLTR = glyph_pos_LTR[0].y_advance;
      printf("LTR: glyphid: %d\t,\tx_advance: %d\t,\ty_advance: %d\n", 
        glyphid, x_advanceLTR, y_advanceLTR);
      
      hb_buffer_destroy (bufLTR);


      hb_buffer_t *bufTTB;
      bufTTB = hb_buffer_create();
      hb_buffer_add_utf8(bufTTB, text, -1, 0, -1);
      
      hb_buffer_set_direction(bufTTB, HB_DIRECTION_TTB);
      //hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
      hb_buffer_set_language(bufTTB, hb_language_from_string("en", -1));

      //hb_font_t *hb_font = pango_font_get_hb_font (font);
      hb_shape (hb_font, bufTTB, NULL, 0);

      hb_glyph_position_t *glyph_pos_TTB = hb_buffer_get_glyph_positions (bufTTB, &glyph_count);

      hb_position_t x_advanceTTB = glyph_pos_TTB[0].x_advance;
      hb_position_t y_advanceTTB = glyph_pos_TTB[0].y_advance;
      printf("TTB: glyphid: %d\t,\tx_advance: %d\t,\ty_advance: %d\n", 
        glyphid, x_advanceTTB, y_advanceTTB);

      hb_buffer_destroy (bufTTB);

      ////hb_font_t *hb_font = pango_font_get_hb_font(font);
      //hb_font_t *hb_font = PANGO_FONT_GET_CLASS(font)->create_hb_font(font); 
  
      /*
      if (hb_font) 
        {
	        hb_face_t *hb_face = hb_font_get_face(hbf);
        }
      */
      /*
      hb_font_set_ptem (hb_font, 25);
      hb_font_set_ppem (hb_font, 25, 25);
      hb_font_set_scale (hb_font, 1024,  1024);
      */
      int hb_x_scale, hb_y_scale;
      hb_font_get_scale (hb_font, &hb_x_scale, &hb_y_scale);
      printf ("Harfbuzz font xscale:%d, yscale:%d\nPango scale:%d\n", hb_x_scale, hb_y_scale, PANGO_SCALE);
      double hb_x_scale_d = (double)hb_x_scale;
      double hb_y_scale_d = (double)hb_y_scale;
      
      //float ptem = hb_font_get_ptem (hb_font);
      //unsigned int x_ppem, y_ppem;
      //hb_font_get_ppem (hb_font, &x_ppem, &y_ppem);
      //printf ("Harfbuzz font to pixels xscale:%d, yscale:%d, points per em:%f, x px per em:%d, y px per em:%d\n", hb_x_scale, hb_y_scale, ptem, x_ppem, y_ppem);
  
      //hb_position_t x, y;
      //hb_font_get_glyph_advance_for_direction (hb_font, glyphs->glyphs[0].glyph, HB_DIRECTION_LTR, &x, &y);
      //printf ("harfbuzz LTR advances for glyph [%s] (=%d) = x: %d y: %d\n", text, glyphs->glyphs[0].glyph, x, y);
      *LTRadvanceX =  ((double)x_advanceLTR / hb_x_scale_d) * font_size;
      *LTRadvanceY =  ((double)y_advanceLTR / hb_y_scale_d) * font_size;
  
      //hb_font_get_glyph_advance_for_direction (hb_font, glyphs->glyphs[0].glyph, HB_DIRECTION_TTB, &x, &y);
      //printf ("harfbuzz TTB advances for glyph [%s] (=%d) = x: %d y: %d\n", text, glyphs->glyphs[0].glyph, x, y);
      *TTBadvanceX = ((double)x_advanceTTB / hb_x_scale_d) * font_size;
      *TTBadvanceY = ((double)y_advanceTTB / hb_y_scale_d) * font_size;
  
      hb_glyph_extents_t glyph_extents;
      hb_bool_t bresult = hb_font_get_glyph_extents (hb_font, glyphid /*glyphs->glyphs[0].glyph*/, &glyph_extents);
  
      printf ("harfbuzz glyph extents for glyph [%s] (=%d) = width: %d height: %d x_bearing: %d y_bearing: %d\n", 
        text, glyphid /*glyphs->glyphs[0].glyph*/, glyph_extents.width, glyph_extents.height, glyph_extents.x_bearing, glyph_extents.y_bearing);
  
      *Xbearing = ((double)glyph_extents.x_bearing / hb_x_scale_d) * font_size;
      *Ybearing = ((double)glyph_extents.y_bearing / hb_y_scale_d) * font_size;
      *width    = ((double)glyph_extents.width / hb_x_scale_d) * font_size;
      *height   = ((double)glyph_extents.height / hb_y_scale_d) * font_size;

      hb_font_extents_t hb_font_extents;
      hb_font_get_extents_for_direction (hb_font, HB_DIRECTION_LTR, &hb_font_extents);
      printf ("LTR: ascender %d, descender %d, line gap %d\n", hb_font_extents.ascender, hb_font_extents.descender, hb_font_extents.line_gap);
      *ascent = ((double)hb_font_extents.ascender / hb_x_scale_d) * font_size;
      *descent = ((double)hb_font_extents.descender / hb_x_scale_d) * font_size;
      *line_gap = ((double)hb_font_extents.line_gap / hb_x_scale_d) * font_size;
      *line_height = ((double)(hb_font_extents.ascender - hb_font_extents.descender + hb_font_extents.line_gap) / hb_x_scale_d) * font_size;

      hb_font_get_extents_for_direction (hb_font, HB_DIRECTION_TTB, &hb_font_extents);
      printf ("TTB: ascender %d, descender %d, line gap %d\n", hb_font_extents.ascender, hb_font_extents.descender, hb_font_extents.line_gap);
      /*
      *ascent = ((double)hb_font_extents.ascender / hb_x_scale_d) * font_size;
      *descent = ((double)hb_font_extents.descender / hb_x_scale_d) * font_size;
      *line_gap = ((double)hb_font_extents.line_gap / hb_x_scale_d) * font_size;
      *line_height = ((double)(hb_font_extents.ascender - hb_font_extents.descender) / hb_x_scale_d) * font_size;
      */


      //debug/testing
      /*
      double dh = (double) (glyph_extents.height / hb_y_scale) * font_size;
      double dw = (double) (glyph_extents.width / hb_x_scale) * font_size;  
      printf("Test: pts Height = %lf, Width = %lf\n", dh, dw);
      */
      /*
      PangoRectangle r;
      r.x = 0;
      r.y = 0;
      r.width = glyph_extents.width;
      r.height = glyph_extents.height;
      printf("x,y,w,h: %d, %d, %d, %d\n", r.x, r.y, r.width, r.height);
  
      pango_extents_to_pixels (&r, NULL);
  
      printf("x,y,w,h: %d, %d, %d, %d\n", r.x, r.y, r.width, r.height);
      */
      // debug/testing ^^
  
  
      ////hb_font_destroy (hb_font);

      //FT_UInt glyph_index = glyphs->glyphs[0].glyph;
      //
      //FT_Glyph ft_glyph; /* a handle to the glyph image */
      //
      //FT_Error error = FT_Load_Glyph (face, glyph_index, FT_LOAD_COMPUTE_METRICS);
      //if (!error) 
      //  {
      //    error = FT_Get_Glyph (face->glyph, &ft_glyph);
      //    if (!error) 
      //      {
      //        *glyph_metrics = face->glyph->metrics;
      //      }
      //  }
      //
      //pango_fc_font_unlock_face(fc_font);    
      ////return face->glyph->metrics;
    }

    hb_font_destroy (hb_font);

  //else 
  //  {
  //    printf ("glyphs->num_glyphs == 0\n");
  //  }
  //
  //pango_glyph_string_free (glyphs);
  //
  //g_list_foreach (items, free_pango_item, NULL);
  //g_list_free (items);
  //
  //pango_attr_list_unref(attrs);
  
  ////
  // get measurements for character
  //PangoFontMetrics *font_metrics;
  //pango_font_get_metrics();
  //PangoContext *pangoContext = pango_cairo_font_map_create_context (pangoFontmap);
  
  /*  
  PangoFont *font = pango_font_map_load_font (pangoFontmap,
                                              pangoContext,
                                              font_desc);
  */
  //PangoFcFont *fc_font;
  //FT_Face face;
  //fc_font = PANGO_FC_FONT (font);
  /*
  PangoGlyph glyph = pango_fc_font_get_glyph (PANGO_FC_FONT (font), codepoint);
  PangoRectangle *ink_rect;
  PangoRectangle *logical_rect;
  pango_font_get_glyph_extents (font, glyph, ink_rect, logical_rect);
  */
  //face = pango_fc_font_lock_face (fc_font);
  //if (!face)
  //  return;

  /*
  n_glyphs  = indic_ot_reorder (wc_in, utf8_offsets, n_chars, indic_info->classTable, wc_out, indices, tags, &mprefixups);
  
  pango_glyph_string_set_size (glyphs, n_glyphs);
  buffer = pango_ot_buffer_new (fc_font);

  set_glyphs(font, wc_out, tags, n_glyphs, buffer,
	     (indic_info->classTable->scriptFlags & SF_PROCESS_ZWJ) != 0);
  */



  //int font_ascent = pango_font_metrics_get_ascent();
  //

  if (b_render_glyph)
    {
      PangoLayout* pangoLayout = pango_layout_new (pangoContext);
      pango_layout_set_wrap (pangoLayout, PANGO_WRAP_WORD_CHAR);
      pango_layout_set_font_description (pangoLayout, font_desc);


      pango_font_description_free (font_desc);


      pango_context_set_language (pangoContext,
                                    pango_language_from_string (lang));

      pango_context_set_base_dir (pangoContext, PANGO_DIRECTION_LTR);
      pango_context_set_gravity_hint (pangoContext, PANGO_GRAVITY_HINT_NATURAL);
      pango_context_set_base_gravity (pangoContext, PANGO_GRAVITY_SOUTH);

      pango_layout_set_width (pangoLayout,
                              pango_units_from_double(xres));

      pango_layout_set_height (pangoLayout,
                              pango_units_from_double(yres));

      //pango_layout_set_width (pangoLayout, xres * PANGO_SCALE);
      //pango_layout_set_height (pangoLayout, yres * PANGO_SCALE);
      pango_layout_set_markup (pangoLayout, text, -1);
      //pango_layout_set_markup (pangoLayout, "<markup><span foreground=\"black\">test</span></markup>", -1);


      /*
      Show Layout (draw text to cairo context)
      */
      cairo_move_to (cairoContext, 0, 0); //0.5, 0.5);  // 0.5px offset plus gamma correction (1/2.2) is the way gimp does it!
      pango_cairo_update_layout (cairoContext, pangoLayout);
      
      int iPixelWidth = 0, iPixelHeight = 0;
      pango_layout_get_pixel_size (pangoLayout, &iPixelWidth, &iPixelHeight);
      printf ("pango_layout_get_pixel_size: width=%d, height=%d\n", iPixelWidth, iPixelHeight);

      pango_cairo_show_layout (cairoContext, pangoLayout);
    }

  //FcConfigDestroy (fontConfig);
  g_string_free (utf8, TRUE);
  
  return 0;
}



int
draw_text (cairo_t *cairoContext,
           gdouble  xres,
           gdouble  yres, 
           guint32  codepoint,
           gboolean b_render_glyph,
           cairo_text_extents_t* glyph_extents,
           const gchar* font_name, gdouble font_size, gboolean is_bold, gboolean is_italic) 
{
  gchar* lang = "en-GB";

  PangoFontMap         *pangoFontmap;
  PangoContext         *pangoContext;
  PangoFontDescription *font_desc;
  cairo_font_options_t *cairoFontOptions;


  /*
    Fontmap and Fontdescription - Specify font to use
  */

  pangoFontmap = pango_cairo_font_map_new_for_font_type (CAIRO_FONT_TYPE_FT);
  if (! pangoFontmap)
    g_error ("You are using a Pango that has been built against a cairo "
             "that lacks the Freetype font backend");
  pango_cairo_font_map_set_resolution (PANGO_CAIRO_FONT_MAP (pangoFontmap), yres /*FONT_SIZE * PANGO_SCALE / DEVICE_DPI*/);
  
  /*
    PangoContext
  */

  pangoContext = pango_font_map_create_context (pangoFontmap);
  g_object_unref (pangoFontmap);

  cairoFontOptions = text_get_font_options ();
  pango_cairo_context_set_font_options (pangoContext, cairoFontOptions);
  cairo_font_options_destroy (cairoFontOptions);
  

  /*
    PangoLayout and Font Description
  */

  guchar* bold = is_bold ? " bold" : "";
  guchar* italic = is_italic ? " italic" : "";
  gchar* font_description_string = g_strjoin (NULL, font_name, bold, italic, (char*)0);

  printf("font description string is: [%s]\n", font_description_string);

  font_desc = pango_font_description_from_string (font_description_string);
  g_return_val_if_fail (font_desc != NULL, -1);
  
  g_free(font_description_string);

  gint size = pango_units_from_double (gimp_units_to_points (font_size,
                                                             GIMP_UNIT_PIXEL,
                                                             yres
                                                             ));
  
  printf("xres=%lf, yres=%lf\nfont size(input)=%lf\nfont size(pango)=%d\n", xres, yres, font_size, size);                                                             

  pango_font_description_set_size (font_desc, MAX (1, size));

  GString *utf8 = utf8fromCodepoint (codepoint);
  gchar *text = utf8->str;

  
  /*
    Get measurements for character
  */

  PangoFont *font = pango_font_map_load_font (pangoFontmap, pangoContext, font_desc);
  get_glyph_extents (text, font_size, font, cairoContext, glyph_extents);


  /*
    Render glyph to surface
  */
 
  if (b_render_glyph)
    {
      PangoLayout* pangoLayout = pango_layout_new (pangoContext);
      pango_layout_set_wrap (pangoLayout, PANGO_WRAP_WORD_CHAR);
      pango_layout_set_font_description (pangoLayout, font_desc);


      pango_font_description_free (font_desc);


      pango_context_set_language (pangoContext,
                                    pango_language_from_string (lang));

      pango_context_set_base_dir (pangoContext, PANGO_DIRECTION_LTR);
      pango_context_set_gravity_hint (pangoContext, PANGO_GRAVITY_HINT_NATURAL);
      pango_context_set_base_gravity (pangoContext, PANGO_GRAVITY_SOUTH);

      pango_layout_set_width (pangoLayout,
                              pango_units_from_double(xres));

      pango_layout_set_height (pangoLayout,
                              pango_units_from_double(yres));

      pango_layout_set_markup (pangoLayout, text, -1);


      /*
        Show Layout (draw text to cairo context)
      */

      cairo_move_to (cairoContext, 0, 0); //0.5, 0.5);  // 0.5px offset plus gamma correction (1/2.2) is the way gimp does it!
      pango_cairo_update_layout (cairoContext, pangoLayout);
      
      int iPixelWidth = 0, iPixelHeight = 0;
      pango_layout_get_pixel_size (pangoLayout, &iPixelWidth, &iPixelHeight);
      printf ("pango_layout_get_pixel_size: width=%d, height=%d\n", iPixelWidth, iPixelHeight);

      pango_cairo_show_layout (cairoContext, pangoLayout);
    }

  g_string_free (utf8, TRUE);
  
  return 0;
}













int 
DOTFACTORYDLLCALL getGlyph (const char    *font_name,    double  font_size,      // in
                            int            is_bold,      int     is_italic,      // 0 == false, true otherwise
                            unsigned int   codepoint,    double  gamma,          // invert monitor gamma (2.2) = 0.45454545.
                            unsigned char *scan0,                                // pointer to bitmap data for calling windows function (ARGB32 format for image)
                            int            bm_width,     int      bm_height,

                            double        *width,        double  *height,        // glyph metrics (out)
                            double        *advanceWidth, double  *advanceHeight,
                            double        *bearingX,     double  *bearingY,
                            double        *ascent,       double  *descent,
                            double        *line_gap,     double  *line_height
                           )
{ 
  #define B_DEBUG TRUE

  if (B_DEBUG)
    printf("Debugging - will output glyph png from DLL\n");

  double xres = (double) bm_width;
  double yres = (double) bm_height;

  printf ("font_name: %s font_size: %lf is_bold: %d is_italic: %d\ncodepoint: %d\ngamma: %lf\nscan0: %0lx\nxres: %lf yres: %lf\n", 
    font_name, font_size, is_bold, is_italic, codepoint, gamma, scan0, xres, yres);

  cairo_status_t     status;
  cairo_surface_t   *surface_text; 

  //if (hdc)
  //  {
  //    printf ("Creating surface for DC\n");
  //    surface_text = cairo_win32_surface_create_with_ddb (hdc, CAIRO_FORMAT_ARGB32, xres, yres);
  //  }
  //else
  //  {
  //    printf("HDC is null, Creating surface with CAIRO_FORMAT_ARGB32, %lf x %lf\n", xres, yres);
  surface_text = cairo_image_surface_create (/*CAIRO_FORMAT_A8*/CAIRO_FORMAT_ARGB32, xres, yres);
  //  }

  gboolean b_is_bold   = (is_bold   != 0);
  gboolean b_is_italic = (is_italic != 0);

  //cairo_surface_t   *surface_text = cairo_image_surface_create (CAIRO_FORMAT_A8, xres, yres);
  cairo_t           *cr_text = cairo_create (surface_text);
 
  //cairo_text_extents_t extents;
  //FT_Glyph_Metrics glyph_metrics;

  double x_bearing, y_bearing;
  double x_advance, y_advance;
  double x_advance_ttb, y_advance_ttb;
  double glyph_width, glyph_height;
  double font_ascent, font_descent, font_linegap, font_lineheight;

  printf("codepoint=%d\n", codepoint);
  
  gboolean b_render_glyph = TRUE;

  if (!scan0)
    {
      b_render_glyph = FALSE || B_DEBUG;   // get glyph metrics only, don't render bitmap if scan0 is null
      printf("Getting glyph metrics only\n");
    }

  /*
  cairo_text_extents_t glyph_extents;
  int has_font = draw_text (cr_text, xres, yres, codepoint, b_render_glyph,
                            &glyph_extents, 
                            font_name, font_size, b_is_bold, b_is_italic);
  */

  int has_font = draw_text_hb (cr_text, xres, yres, codepoint, b_render_glyph,
                               &x_advance, &y_advance, &x_advance_ttb, &y_advance_ttb, 
                               &x_bearing, &y_bearing, &glyph_width, &glyph_height,
                               &font_ascent, &font_descent, &font_linegap, &font_lineheight,
                               font_name, font_size, b_is_bold, b_is_italic);
  
  printf ("font is %s\n", has_font == 0 ? "available" : "unavailable, returning -1");

  //if (scan0) 
  //    printf ("Copying glyph output to windows bitmap data surface scan0\n");
  //else
      //printf ("Writing glyph output to PNG file glyph.png\n");
  
  if (has_font == 0 && (b_render_glyph || B_DEBUG))
    {
      printf ("Getting glyph bitmap and metrics\n");
      cairo_surface_t *blended_surface = process_transparency (surface_text, gamma);
      //cairo_t *cr_blended = cairo_create (blended_surface);
      copy_to_bitmap (blended_surface, scan0, bm_width, bm_height);    
      if (B_DEBUG)
        status = cairo_surface_write_to_png (blended_surface, "glyph.png");
      cairo_surface_destroy (blended_surface);
    }
  //else
    //status = cairo_surface_write_to_png (blended_surface, "glyph.png");

  //cairo_destroy (cr_blended);
  
  if (has_font == 0) 
    {
      /*
      *bearingX = glyph_extents.x_bearing;
      *bearingY = glyph_extents.y_bearing; 
 
      *width  = glyph_extents.width;
      *height = glyph_extents.height;
  
      *advanceWidth  = glyph_extents.x_advance;
      *advanceHeight = glyph_extents.y_advance;
      */

      *bearingX = x_bearing;
      *bearingY = y_bearing; 
 
      *width  = glyph_width;
      *height = glyph_height;
  
      *advanceWidth  = x_advance;
      *advanceHeight = y_advance_ttb;

      *ascent = font_ascent;
      *descent = font_descent;
      *line_gap = font_linegap;
      *line_height = font_lineheight;

      g_print ("width:\t%lf\t,\theight:\t%lf\nbearingX:\t%lf\t,\tbearingY:\t%lf\nadvanceWidth:\t%lf\t,\tadvanceHeight:\t%lf\n"
               "ascent:\t%lf\t,\tdescent:\t%lf\t,\tline_gap:\t%lf\t,\tline_height:\t%lf\n", 
               *width, *height, *bearingX, *bearingY, *advanceWidth, *advanceHeight, *ascent, *descent, *line_gap, *line_height);
    }
 
  /*
  if (!scan0 && status != CAIRO_STATUS_SUCCESS)
    {
      g_printerr ("Could not save file glyph.png'\n");
      return 1;
    }
  */

  return has_font;
}



//boolean DOTFACTORYDLLCALL getGlyph (const char *font_name,    double  font_size,
//                                    boolean     is_bold,      boolean is_italic,
//                                    uint32_t    codepoint, 
//                                    double     *width,        double  *height,
//                                    double     *advanceWidth, double  *advanceHeight,
//                                    double     *bearingX,     double  *bearingY,
//                                    HBITMAP    *bmWinBitmap)
//{
///*
//  cairo_t         *cr;
//  cairo_t         *cr_source;
//  cairo_t         *cr_mask;
//  gchar           *filename;
//*/
//  cairo_status_t   status;
///*
//  cairo_surface_t *surface;
//  cairo_surface_t *surface_source;
//  cairo_surface_t *surface_mask;
//  gdouble          xres = 300;
//  gdouble          yres = 300;
//*/
//
///*
//  if (argc != 2)
//    {
//      g_printerr ("Usage: cairosimple OUTPUT_FILENAME\n");
//      return 1;
//    }
//
//     filename = argv[1];
//*/
//  /*
//  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
//                                        2 * RADIUS, 2 * RADIUS);
//  */
//  /*
//  surface_source = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, xres, yres); 
//  cr_source = cairo_create (surface_source);
//  */
///*
//  surface_mask = cairo_image_surface_create (CAIRO_FORMAT_A8, xres, yres);
//  cr_mask = cairo_create (surface_mask);
//
//  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, xres, yres);
//  cr = cairo_create (surface);
//*/
//
//  //cairo_set_source_rgba (cr, 1, 1, 1, 1.0);
//  //cairo_paint (cr);
//
//  //cairo_scale(cr, 1 * TWEAKABLE_SCALE, 1 * TWEAKABLE_SCALE);
//
//  //cairo_set_source_rgba (cr, 1, 1, 1, 1.0);
//  //cairo_rectangle(cr, 0, 0, 300, 300);
//  //cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
//  //cairo_paint (cr);
//  //cairo_set_source_rgba (cr, 0, 0, 0, 0.5);
//
//  //cairo_set_source_rgba (cr_mask, 0, 0, 0, 0.5);
//  //cairo_rectangle (cr_mask, 100,100,100,100);
//  //cairo_fill(cr_mask);
//  //cairo_surface_flush (surface_mask);
//  
//  /*
//  cairo_set_source_rgba (cr_source, 0.0, 0.0, 0.0, 1.0);
//  cairo_paint (cr_source);
//  */
//  
//  //cairo_set_source_rgba (cr_mask, 1, 1, 1, 1);
//  //cairo_paint (cr_mask);
//  
//  /*
//  draw_text (cr_mask, xres, yres);
//  */
//  
//  //draw_text (cr_source, xres, yres);
//
//  // Mask and Source surfaces *do not combine in real time in response to graphics functions* - 
//  // they are applied *once*. The call to cairo_set_source_surface needs to be made *before* the
//  // call to cairo_mask_surface, and the surfaces need to be in their completed state before both
//  // calls
//  /*
//  cairo_set_source_surface (cr, surface_source, 0, 0);
//  cairo_mask_surface (cr, surface_mask, 0, 0);
//  */
//
//  // Gamma text test
//  /*
//  cairo_surface_t *blended_surface = process_transparency(surface_mask);
//  cairo_t         *cr_blended = cairo_create (surface_mask);
//  //status = cairo_surface_write_to_png (blended_surface, "blended.png");
//  cairo_destroy (cr_blended);
//  cairo_surface_destroy (blended_surface);
//  */
//
//  cairo_surface_t   *surface_text = cairo_image_surface_create (CAIRO_FORMAT_A8, xres, yres);
//  cairo_t           *cr_text = cairo_create (surface_text);
// 
//  //draw_text (cr_text, xres, yres);
///*
//    double x_bearing;
//    double y_bearing;
//    double width;
//    double height;
//    double x_advance;
//    double y_advance;
//*/
//
//  cairo_text_extents_t extents;
//  draw_text (cr_text, xres, yres, codepoint, &extents, font_name, font_size, is_bold, is_italic);
//
//  cairo_surface_t   *blended_surface = process_transparency (surface_text);
//  cairo_t           *cr_blended = cairo_create (blended_surface/*surface_mask*/);
//  status = cairo_surface_write_to_png (blended_surface, "blended.png");
//  cairo_destroy (cr_blended);
//  cairo_surface_destroy (blended_surface);
//
//  if (status != CAIRO_STATUS_SUCCESS)
//    {
//      g_printerr ("Could not save file blended.png'\n");
//      return 1;
//    }
//
//  return 0;
//
//  // Gamma gradient test
//  /*
//  cairo_surface_t *gradient_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, xres, yres);
//  cairo_t         *cr_gradient = cairo_create (gradient_surface);
//  draw_gradient(cr_gradient, xres, yres);
//  cairo_surface_t *blended_gradient_surface = process_transparency(gradient_surface);
//  status = cairo_surface_write_to_png (gradient_surface, "gradient.png");
//  status = cairo_surface_write_to_png (blended_gradient_surface, "blended_gradient.png");
//  cairo_destroy (cr_gradient);
//  cairo_surface_destroy (gradient_surface);
//  cairo_surface_destroy (blended_gradient_surface);
//  */
//
//  // Output various text test images
//  /*
//  status = cairo_surface_write_to_png (surface, "output.png");
//  status = cairo_surface_write_to_png (surface_mask, "mask.png");
//  status = cairo_surface_write_to_png (surface_source, "source.png");
//  */
//  /*
//  cairo_destroy (cr);
//  cairo_destroy (cr_mask);
//  cairo_destroy (cr_source);
//  cairo_surface_destroy (surface);
//  cairo_surface_destroy (surface_mask);
//  cairo_surface_destroy (surface_source);
//  */
//  /*
//  if (status != CAIRO_STATUS_SUCCESS)
//    {
//      g_printerr ("Could not save png to '%s'\n", filename);
//      return 1;
//    }
//  */
//  //return 0;
//}



//public bool GetFontAscentAndDescent(ref double ascent, ref double descent, ref double linespacing, ref double baseline)
//{
//    ascent = 1.0;
//    descent = 1.0;
//    linespacing = 1.0;
//
//    double toPixels = (96 / 72) * g_wpf_fontsize;
//
//    linespacing = g_wpf_typeface.FontFamily.LineSpacing * toPixels;
//    baseline = g_wpf_typeface.FontFamily.Baseline * toPixels;
//
//    return true;
//}
//
//public bool GetCharAdvanceWidthAndHeight(int charindex, ref double width, ref double height)
//{
//    try
//    {
//        var typeface = g_wpf_typeface;
//
//        GlyphTypeface glyph;
//        typeface.TryGetGlyphTypeface(out glyph);
//
//        double w = 0;
//        double h = 0;
//
//        if (charindex < 65536)
//        {
//            var character = (char)charindex;
//            var glyphIndex = glyph.CharacterToGlyphMap[character];
//
//            w = glyph.AdvanceWidths[glyphIndex];
//            h = glyph.Height - glyph.TopSideBearings[glyphIndex]
//                                            - glyph.BottomSideBearings[glyphIndex];
//
//            width = w * (/*g.DpiX*/ 96 / 72) * g_wpf_fontsize; // g_font.Size;
//            height = h * (/*g.DpiY*/ 96 / 72) * g_wpf_fontsize; // g_font.Size;
//        }
//        else
//        {
//            getTextAdvance(Char.ConvertFromUtf32(charindex), ref width, ref height);
//        }
//
//
//        return true;
//    }
//
//    catch (Exception e)
//    {
//        System.Windows.Forms.MessageBox.Show(e.Message);
//        return false;
//    }
//}
//
//private void getTextAdvance(string text, ref double width, ref double height)
//{
//    var formattedText = new FormattedText(
//        text,
//        CultureInfo.CurrentCulture,
//        System.Windows.FlowDirection.LeftToRight,
//        g_wpf_typeface, /*new Typeface(this.textBlock.FontFamily, this.textBlock.FontStyle, this.textBlock.FontWeight, this.textBlock.FontStretch),*/
//        g_wpf_fontsize, /*this.textBlock.FontSize,*/
//        System.Windows.Media.Brushes.Black,
//        1.0
//    );
//
//    width = formattedText.Width;
//    height = formattedText.Extent + formattedText.OverhangAfter; // formattedText.Height + formattedText.OverhangAfter;
//
//    return;
//}
