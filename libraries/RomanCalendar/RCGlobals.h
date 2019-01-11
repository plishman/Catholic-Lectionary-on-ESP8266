#ifndef _RCGLOBALS42_H
#define _RCGLOBALS42_H
	#define LECT_VER "0.7"
    #define MAX_DEEPSLEEP_SECONDS 4294
	#define DEBUG_PRT I2CSerial	//Serial
	
	#define EPAPER_GxGDEW042Z15
//	#define EPAPER_GxGDEW027C44
	
	#ifdef EPAPER_GxGDEW027C44
		#define EPD_ROTATION 1

		#define PAGE_HEIGHT GxGDEW027C44_PAGE_HEIGHT
		#define PAGE_WIDTH GxGDEW027C44_WIDTH
		#define PAGE_COUNT GxGDEW027C44_PAGES

		#define DISPLAY_PORTRAIT true

		#define _PSIZE_X GxGDEW027C44_WIDTH
		#define _PSIZE_Y GxGDEW027C44_HEIGHT

		#if DISPLAY_PORTRAIT == true
			#define PANEL_SIZE_Y _PSIZE_X
			#define PANEL_SIZE_X _PSIZE_Y
		#else
			#define PANEL_SIZE_X _PSIZE_X
			#define PANEL_SIZE_Y _PSIZE_Y
		#endif

		//#include <GxEPD.h>
		//#include <GxGDEW027C44/GxGDEW027C44.h>      // 2.7" b/w/r 176x264 GxGDEW027C44/GxGDEW027C44.cpp
	#endif
	
	#ifdef EPAPER_GxGDEW042Z15
		#define EPD_ROTATION 0
	
		#define PAGE_HEIGHT GxGDEW042Z15_PAGE_HEIGHT
		#define PAGE_WIDTH GxGDEW042Z15_WIDTH
		#define PAGE_COUNT GxGDEW042Z15_PAGES

		#define DISPLAY_PORTRAIT false

		#define _PSIZE_X GxGDEW042Z15_WIDTH
		#define _PSIZE_Y GxGDEW042Z15_HEIGHT

		#if DISPLAY_PORTRAIT == true
			#define PANEL_SIZE_Y _PSIZE_X
			#define PANEL_SIZE_X _PSIZE_Y
		#else
			#define PANEL_SIZE_X _PSIZE_X
			#define PANEL_SIZE_Y _PSIZE_Y
		#endif

		//#include <GxEPD.h>
		//#include <GxGDEW042Z15/GxGDEW042Z15.h>      // 4.2" b/w/r
	#endif
#endif