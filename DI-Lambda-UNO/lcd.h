
#include <SPI.h>

//#include <Adafruit_ST7735.h>


#include <PDQ_GFX.h>    // Core graphics library
#include "PDQ_ST7735_config.h"

#include <PDQ_ST7735.h> // Hardware-specific library


#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>

/*
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
*/

#define TFT_BACKLIGHT 10

/*
#define ST7735_CS_PIN    A5          // <= /CS pin (chip-select, LOW to get attention of ST7735, HIGH and it ignores SPI bus)
#define ST7735_DC_PIN    A3          // <= DC pin (1=data or 0=command indicator line) also called RS
#define ST7735_RST_PIN   A4          // <= RESET pin (active low)
*/

//Adafruit_ST7735 tft = Adafruit_ST7735(ST7735_CS_PIN,  ST7735_DC_PIN, ST7735_RST_PIN);

PDQ_ST7735 tft;

struct point {
  uint8_t x;
  uint8_t y;
};

point wavelength = { 5, 20 };
point mode_label = { 5, 37 };
point data_label =  { 10, 85 };

#define FONT_HEIGHT 18

typedef uint16_t color_t;

#define BACKGROUND_COLOR ST7735_BLACK
#define WL_COLOR ST7735_YELLOW
#define LABEL_COLOR ST7735_WHITE
#define A_COLOR ST7735_WHITE
#define T_COLOR ST7735_WHITE
#define B_COLOR ST7735_GREEN
#define FL_COLOR ST7735_GREEN

typedef enum {
  MODE_FIRST = 1,
  MODE_ABS = MODE_FIRST, // Absorbance 
  display_mode_tR = 2,  // Transmittance
  MODE_FL = 3,  // Fluorescence
  MODE_RAW = 4, // Raw data
  MODE_INFO = 5,
  MODE_LAST = MODE_FL
} display_mode_t;

display_mode_t mode;

char txt_buffer[16];

void display_clear(void) {
    tft.fillScreen(BACKGROUND_COLOR);
}

void display_wl(uint16_t wl) {
  static char last_value[16] = {0, };
  int16_t x, y, w, h;
  if (wl) {
    snprintf(txt_buffer, sizeof(txt_buffer), "%dnm", wl);
  } else {
    txt_buffer[0] = 0;
  }

  if (strcmp(last_value, txt_buffer)) {
    tft.setFont(&FreeSans12pt7b);
    tft.setCursor(wavelength.x, wavelength.y);
    tft.setTextColor(BACKGROUND_COLOR);
    tft.print(last_value);
    tft.setCursor(wavelength.x, wavelength.y);

    tft.setTextColor(WL_COLOR);
    tft.print(txt_buffer);
    memcpy(last_value, txt_buffer, 16);
  }
}

void dislay_mode_label(char *label)
{
  static char last_value[16] = {0, };

  if (label) {
    strncpy(txt_buffer, label, 16);
  } else {
    txt_buffer[0] = 0;
  }

  if (strcmp(last_value, txt_buffer)) {
    tft.setFont(&FreeSans9pt7b);
    tft.setCursor(mode_label.x, mode_label.y);
    tft.setTextColor(BACKGROUND_COLOR);
    tft.print(last_value);
    tft.setCursor(mode_label.x, mode_label.y);

    tft.setTextColor(LABEL_COLOR);
    tft.print(txt_buffer);
    memcpy(last_value, txt_buffer, 16);
  }
}

void show_large_data(char *data, color_t color) {
  static char last_value[16] = {0, };
  if (data) {
    strncpy(txt_buffer, data, 16);
  } else {
    txt_buffer[0] = 0;
  }
  
  tft.setFont(&FreeSans18pt7b);
  if (strcmp(last_value, txt_buffer)) {
    tft.setCursor(data_label.x, data_label.y);
    tft.setTextColor(BACKGROUND_COLOR);
    tft.print(last_value);
    memcpy(last_value, txt_buffer, 16);

  }

  tft.setCursor(data_label.x, data_label.y);
  tft.setTextColor(color);
  tft.print(txt_buffer);
  
}

void display_transmittance() {
  if (blank > 0 && last_sample >= 0) {          
    float f = ((float)last_sample / blank) * 100;
    dtostrf(f, 2, 1, txt_buffer);
    if (strlen(txt_buffer) > 4) {
      dtostrf(f, 2, 0, txt_buffer);
    }

    snprintf(txt_buffer, sizeof(txt_buffer), "%s%%", txt_buffer);
    show_large_data(txt_buffer, T_COLOR);
          
  } else {
    show_large_data("N/A", T_COLOR);
  }
}

void display_absorbance() {
  if (blank > 0 && last_sample >= 0) {          
    float f = log10((float)blank / last_sample);
    if (f < 0 && f > -0.01)
      f = 0;
    
    dtostrf(f, 1, 2, txt_buffer);
    show_large_data(txt_buffer, T_COLOR);
          
  } else {
    show_large_data("N/A", T_COLOR);
  }
}

void display_fluorescence() {
  static char last_value[16] = {0, };
  
  if (blank > 0 && last_sample >= 0) {          
    float f = ((float)last_sample / blank);
    dtostrf(f, 1, 2, txt_buffer);
    if (strlen(txt_buffer) > 4) {
      dtostrf(f, 2, 0, txt_buffer);
    }

    snprintf(txt_buffer, sizeof(txt_buffer), "%s", txt_buffer);
          
  } else {
    strcpy(txt_buffer, "N/A");
  }

  if (strcmp(last_value, txt_buffer)) {
    tft.setFont(&FreeSans18pt7b);
    tft.setCursor(data_label.x, data_label.y);
    tft.setTextColor(BACKGROUND_COLOR);
    tft.print(last_value);
    tft.setFont(&FreeSans9pt7b);
    tft.print(" a.u.");
    memcpy(last_value, txt_buffer, 16);
  }

  tft.setFont(&FreeSans18pt7b);
  tft.setCursor(data_label.x, data_label.y);
  tft.setTextColor(FL_COLOR);
  tft.print(txt_buffer);
  tft.setFont(&FreeSans9pt7b);
    
  if (blank > 0 && last_sample >= 0){
    tft.print(" a.u.");
  }
}

void display_raw() {
}


void display_mode(display_mode_t mode)
{
  //display_clear();

  if(mode != MODE_INFO) {
      display_wl(led_wl);
      
      tft.drawRoundRect(5, 44, 118, 60, 5, LABEL_COLOR);
      tft.fillRoundRect(6, 45, 116, 58, 5, BACKGROUND_COLOR);
  }

  if (mode == display_mode_tR) {
    dislay_mode_label("Transmittance");
    display_transmittance();
  }

  if (mode == MODE_ABS) {

    dislay_mode_label("Absorbance");
    display_absorbance();
  }

  if (mode == MODE_FL) {

    dislay_mode_label("Fluorescence");
    display_fluorescence(); 
  }
/*
  if (mode == MODE_RAW) {
    dislay_mode_label("Raw data");
  }  
*/
}

void lcd_new_data()
{
  if (mode == display_mode_tR) {
    display_transmittance();
  }

  if (mode == MODE_ABS) {
    display_absorbance();
  }

  if(mode == MODE_FL) {
    display_fluorescence();
  }
}

void display_name(void) {
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ST7735_WHITE);  
  tft.setCursor(5, 20);
  tft.println("DI-L UNO v1.5");
  tft.setCursor(15, 120);

  tft.println("Koi Science");
}

void display_start(void) {
  
 // reset like Adafruit does
 
  FastPin<ST7735_RST_PIN>::setOutput();
  FastPin<ST7735_RST_PIN>::hi();
  FastPin<ST7735_RST_PIN>::lo();
  delay(1);
  FastPin<ST7735_RST_PIN>::hi();

  Serial.println("display_start");
  
  //tft.initR(INITR_144GREENTAB);
  //SPI.setClockDivider(SPI_CLOCK_DIV2);

  
  Serial.println("display_start 2");

  tft.begin();
  display_name();
  
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, LOW);
}



