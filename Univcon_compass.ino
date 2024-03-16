#include <Wire.h>
#include <TFT_eSPI.h> 
#include "compas.h"
#include "position.h"
#include "HMC5883mumet.h"

MechaQMC5883 qmc;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite bck = TFT_eSprite(&tft);
TFT_eSprite sprite = TFT_eSprite(&tft);
TFT_eSprite sprite1 = TFT_eSprite(&tft);
TFT_eSprite data = TFT_eSprite(&tft);

#define gray 0x39C7
#define background1 TFT_BLACK //0x29CC
#define background2 0x1928

int angle=0;
int angle2=359;
int sx=60;
int sy=12; 
int jarumtetaputara;int jarumtetaputara1;
  int x, y, z;
  
  int azimuth;
void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  tft.init();  tft.fillScreen(background1);  
  tft.setSwapBytes(true);
  tft.setRotation(0);
  tft.drawString("LOCATION",10,400);
  tft.setTextColor(TFT_ORANGE, background1);
  //tft.loadFont(NotoSansBold15);
  tft.drawString("43768554'",10,360);
  //tft.loadFont(NotoSansBold15);
  tft.drawString("23758554'",10,360);
  tft.pushImage(100,360,48,48,position);
  
  data.createSprite(140,60);
  //data.loadFont(NotoSansMonoSCB20);
  data.setTextColor(TFT_WHITE,background1);

  bck.createSprite(200,200);
  bck.setSwapBytes(true);
  
  sprite.createSprite(200,200);
  sprite.setSwapBytes(true);
 
    
  sprite1.createSprite(121,25);
  sprite.setPivot(100,100);
  sprite.pushImage(1,1,199,199,COMPASS);
  qmc.init();
  qmc.setMode(Mode_Continuous,ODR_200Hz,RNG_2G,OSR_256);
  
}


void drawData(){
  data.fillSprite(background1);
  if (azimuth<10){
          data.drawString("00"+String(azimuth),10,8,7);data.fillSmoothCircle(115, 10, 3, TFT_WHITE);
  }
  else if (azimuth>=10 && azimuth<100){
          data.drawString("0"+String(azimuth),10,8,7);data.fillSmoothCircle(115, 10, 3, TFT_WHITE);
  }
  else if (azimuth>=100){
          data.drawString(    String(azimuth),10,8,7);data.fillSmoothCircle(115, 10, 3, TFT_WHITE);
  }
  data.pushSprite(105,255);
}

void drawCompas(){ 
  bck.fillSprite(background1);
  //bck.fillSmoothCircle(152, 5, 5, TFT_ORANGE);
  
  sprite1.drawWedgeLine(6, sy, sx, sy, 1, 10, TFT_DARKGREY);
  //sprite1.drawWedgeLine(sx, sy, 115, sy, 10, 1, TFT_YELLOW);
  sprite1.fillSmoothCircle(sx, sy, 12, gray);
  sprite1.fillSmoothCircle(sx, sy, 6, TFT_WHITE);

  sprite.pushRotated(&bck,jarumtetaputara1,background1);
  sprite1.pushRotated(&bck,jarumtetaputara,background1);
  bck.pushSprite(59,37);
}

void loop() { 

  qmc.read(&x, &y, &z,&azimuth); 
  angle=azimuth+90; if(angle>360)angle=angle-360;
  jarumtetaputara = angle-azimuth; //360-angle ;
  angle2=azimuth; if(angle2<0)angle2=359;
  jarumtetaputara1 = 360-angle2;
  
Serial.print(" Azimuth:"); Serial.println(azimuth);
drawCompas();
drawData();
delay(10);
}
