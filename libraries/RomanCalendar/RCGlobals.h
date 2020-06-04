#ifndef _RCGLOBALS42_H
#define _RCGLOBALS42_H
	#define LECT_VER "0.7"
    #define MAX_DEEPSLEEP_SECONDS 4294
	#define MAX_MEM_BIBLE_REFS 4096

// select debug port (comment out next line to default to Serial)
	#define DEBUG_PRT Debug_Prt
	
	#ifndef DEBUG_PRT
		#define DEBUG_PRT Serial
		#define DEBUGPRT_BEGIN Serial.begin(115200);
		#define DEBUGPRT_END
	#else
		#define DEBUGPRT_BEGIN Debug_Prt.begin("/html/DEBUGLOG.TXT", 1, 3, 8);
		#define DEBUGPRT_END Debug_Prt.end();
	#endif

	#define CRASHFILEPATH "/html/CRASHDMP.TXT"

	#define USE_SPI_RAM_FRAMEBUFFER

	#ifdef USE_SPI_RAM_FRAMEBUFFER
		#define FB_EPAPER FrameBuffer&  // use SPI ram frame buffer for faster rendering if present
		#define FB_EPAPER_PTR FrameBuffer*  // use SPI ram frame buffer for faster rendering if present
		#define MAX_MEM_BIBLE_REFS 4096
	#else
		#define FB_EPAPER GxEPD_Class&	// render direct to ePaper display
		#define FB_EPAPER_PTR GxEPD_Class*	// render direct to ePaper display
		#define MAX_MEM_BIBLE_REFS 1536
	#endif

// select e-paper display:	
	#define EPAPER_GxGDEW042Z15
//	#define EPAPER_GxGDEW027C44
	
	#ifdef EPAPER_GxGDEW027C44	// portrait orientation framebuffer
		#define EPD_ROTATION 1

		#define PAGE_HEIGHT GxGDEW027C44_PAGE_HEIGHT
		#define PAGE_WIDTH GxGDEW027C44_WIDTH
		#define PAGE_COUNT GxGDEW027C44_PAGES

		#define PANEL_SIZE_X GxGDEW027C44_WIDTH
		#define PANEL_SIZE_Y GxGDEW027C44_HEIGHT

		//#include <GxEPD.h>
		//#include <GxGDEW027C44/GxGDEW027C44.h>      // 2.7" b/w/r 176x264 GxGDEW027C44/GxGDEW027C44.cpp
	#endif
	
	#ifdef EPAPER_GxGDEW042Z15	// landscape orientation framebuffer
		#define EPD_ROTATION 0

		#define PAGE_HEIGHT GxGDEW042Z15_PAGE_HEIGHT
		#define PAGE_WIDTH GxGDEW042Z15_WIDTH
		#define PAGE_COUNT GxGDEW042Z15_PAGES

		#define PANEL_SIZE_X GxGDEW042Z15_WIDTH
		#define PANEL_SIZE_Y GxGDEW042Z15_HEIGHT

		//#include <GxEPD.h>
		//#include <GxGDEW042Z15/GxGDEW042Z15.h>      // 4.2" b/w/r
	#endif
#endif