EPD_DISPLAY_IMAGE* epd_image = NULL;
String epaper_messagetext = "";
uint16_t epaper_messagetext_color = GxEPD_BLACK;

void updateDisplay(DISPLAY_UPDATE_TYPE d) {
  updateDisplay(d, "", GxEPD_BLACK);
}

void updateDisplay(DISPLAY_UPDATE_TYPE d, String messagetext, uint16_t messagecolor) {
  DEBUG_PRT.println("Updating Display");
  ePaper.init(); // disable diagnostic output on Serial (use 115200 as parameter to enable diagnostic output on Serial)
  DEBUG_PRT.println("Init Display");

  if (d == display_reading) {
    DEBUG_PRT.println("display_reading");
    ePaper.drawPaged(epaperUpdate);
    return;    
  }

  epaper_messagetext = messagetext;
  epaper_messagetext_color = messagecolor;
  
  switch(d) {
  case battery_recharge:
    DEBUG_PRT.println("display battery_recharge image");
    epd_image = &battery_recharge_image;
    break;

  case connect_power: 
    DEBUG_PRT.println("display connect_power image");
    epd_image = &connect_power_image;
    break;

  case wps_connect:
    DEBUG_PRT.println("display wps_connect image");
    epd_image = &wps_connect_image;
    break;

  case clock_not_set:
    DEBUG_PRT.println("display clock_not_set image");
    epd_image = &clock_not_set_image;
    break;

  case sd_card_not_inserted:
    DEBUG_PRT.println("display sd_card_not_inserted image");
    epd_image = &sd_card_not_inserted_image;
    break;
  }
  
  ePaper.drawPaged(epaperDisplayImage);
}

void epaperUpdate() {
  wdt_reset();
  ePaper.eraseDisplay();
  ePaper.setRotation(1); //90 degrees 
  
  tb.render(ePaper, diskfont);
  
  int charheight = diskfont._FontHeader.charheight;
  ePaper.drawFastHLine(0, charheight, PANEL_SIZE_X, GxEPD_BLACK);
  
  DEBUG_PRT.print(".");
}

void epaperDisplayImage() {
  if (epd_image == NULL) return;
  
  wdt_reset();
  ePaper.setRotation(0); //90 degrees       
  ePaper.eraseDisplay();
  if (epd_image->bitmap_black != NULL) ePaper.drawBitmap(epd_image->bitmap_black, 0, 0, PANEL_SIZE_Y, PANEL_SIZE_X, GxEPD_BLACK, GxEPD::bm_transparent); 
  if (epd_image->bitmap_red   != NULL) ePaper.drawBitmap(epd_image->bitmap_red,   0, 0, PANEL_SIZE_Y, PANEL_SIZE_X, GxEPD_RED,   GxEPD::bm_transparent); 

  if (epaper_messagetext != "") {
    int strwidth = diskfont.GetTextWidth(epaper_messagetext);
    ePaper.setRotation(1); //90 degrees       
    diskfont.DrawStringAt(PANEL_SIZE_X - strwidth, 0, epaper_messagetext, ePaper, epaper_messagetext_color, false);
  }
  DEBUG_PRT.print(".");
}

