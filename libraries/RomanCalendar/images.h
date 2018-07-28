typedef struct
{
	unsigned char* bitmap_black;
	unsigned char* bitmap_red;
	int bitmap_bytecount;
} EPD_DISPLAY_IMAGE;	

extern EPD_DISPLAY_IMAGE battery_recharge_image;

extern EPD_DISPLAY_IMAGE connect_power_image;

extern EPD_DISPLAY_IMAGE wps_connect_image;

extern EPD_DISPLAY_IMAGE clock_not_set_image;

extern EPD_DISPLAY_IMAGE sd_card_not_inserted_image;

extern EPD_DISPLAY_IMAGE replace_cr2032_image;