
#define ST7735
#define POLOLU    // define this to use pololu library which saves flash memory
// #define LONG_RANGE

#ifdef POLOLU
#include <Wire.h>
#include <VL53L0X.h>
VL53L0X sensor;
#else
#include <Adafruit_VL53L0X.h>
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
#endif

#ifdef ST7735
#include <TFT_ST7735.h> 
TFT_ST7735 tft = TFT_ST7735();       // Invoke custom library
#define BLACK   TFT_BLACK
#define WHITE   TFT_WHITE
#define YELLOW  TFT_YELLOW
#define GREEN   TFT_GREEN
#else
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#define TFT_CS        10 
#define TFT_RST        9
#define TFT_DC         8
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
#define BLACK   ST77XX_BLACK
#define WHITE   ST77XX_WHITE
#define YELLOW  ST77XX_YELLOW
#define GREEN   ST77XX_GREEN
#endif

#define UP     3
#define RIGHT  2
#define DOWN  17

#define TITLE "Distance"


void distance_str(char *str, int32_t val) {
  // 64-(tmp_str.length()*12)/2

  if(val < 0) {
    strcpy(str, "./.");
    return;
  }

  // convert integer to string
  ltoa(val, str, 10);

  // move last digit one positon back and insert decimal dot
  char l = strlen(str);
  str[l+1] = 0;         // termination char
  str[l] = str[l-1];    // move digit
  str[l-1] = '.';       // set dot

  // append "cm"
  strcpy(str+l+1, "cm"); 
}

void draw_centered(char y, char *str) {
  
  // erase previous text area
  tft.fillRect(10, y, 140, 8, BLACK);

  tft.setCursor(80-(6*strlen(str))/2, y);
  tft.setTextSize(0);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextWrap(false);
  
  tft.print(str);  
}

void draw_mem(char y, int16_t val) {
  int yd[] = { 60, 70, 80 };
  char str[32];

  strcpy(str, "X: ");
  str[0] += y;
  distance_str(str+strlen(str), val);

  draw_centered(yd[y], str);
}

void draw_square(int32_t val) {
  char str[32];

  strcpy(str, "X*Y: ");
  distance_str(str+strlen(str), val);
  if(val >= 0) strcat(str, "^2");
  draw_centered(100, str);
}

void draw_cube(int32_t val) {
  char str[32];

  strcpy(str, "X*Y*Z: ");
  distance_str(str+strlen(str), val);
  if(val >= 0) strcat(str, "^3");
  draw_centered(110, str);
}

void setup(void) { 
#ifdef POLOLU
  Wire.begin();
  sensor.init();
  sensor.setTimeout(500);

#ifdef LONG_RANGE
  // ---- enable long range mode, increases sensivity and work better in dark environments ----
  // lower the return signal rate limit (default is 0.25 MCPS)
  sensor.setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif
#else
  lox.begin();
#endif
  
#ifdef ST7735
  tft.init();
#else
  tft.initR(INITR_REDTAB); // Init ST7735R chip, green tab
#endif

  tft.setRotation(1);
  
  // clear screen
  tft.fillScreen(BLACK);

  // setup screen mask
  tft.drawRoundRect(5, 5, 150, 118, 5, YELLOW);
  tft.setCursor(80-(strlen(TITLE)*12)/2, 10);
  tft.setTextSize(2);
  tft.setTextColor(GREEN, BLACK);
  tft.setTextWrap(false);
  tft.print(TITLE); 

  draw_mem(0, -1);
  draw_mem(1, -1);
  draw_mem(2, -1);
  draw_square(-1);
  draw_cube(-1);

  // configure UP/RIGHT/DOWN buttons as inputs
  pinMode(UP, INPUT_PULLUP);
  pinMode(RIGHT, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
}

void draw_distance(int16_t val) {
  static int16_t cval = -1;  
  char str[32];

  if(cval == val) 
    return;

  cval = val;

  // draw main distance 
  if(val < 0) strcpy(str, "out of range");
  else        distance_str(str, val);
  
  // erase previous text area
  tft.fillRect(6, 30, 148, 16, BLACK);

  tft.setCursor(80-(12*strlen(str))/2, 30);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextWrap(false);
  
  tft.print(str);
}

void loop() {
  static int16_t x=-1,y=-1,z=-1;
  
#ifdef POLOLU
  int16_t distance = sensor.readRangeSingleMillimeters();
  if(sensor.timeoutOccurred())
    distance = -1;
#else
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  int16_t distance = measure.RangeMilliMeter;
  if(measure.RangeStatus == 4)
    distance = -1;
#endif

  draw_distance(distance);
  
  if(!digitalRead(UP)) {
     x = distance;
     draw_mem(0, x);

     if(y >= 0)
        draw_square((int32_t)x*(int32_t)y);
     if((y >= 0) && (z >= 0))
        draw_cube((int32_t)x*(int32_t)y*(int32_t)z);
  }
  
  if(!digitalRead(RIGHT)) {
     y = distance;
     draw_mem(1, y);
     
     if(x >= 0)
        draw_square((int32_t)x*(int32_t)y);
     if((x >= 0) && (z >= 0))
        draw_cube((int32_t)x*(int32_t)y*(int32_t)z);
  }

  if(!digitalRead(DOWN)) {
     z = distance;
     draw_mem(2, z);
     if((x >= 0) && (y >= 0))
        draw_cube((int32_t)x*(int32_t)y*(int32_t)z);
  }
  
  delay(100);
}
