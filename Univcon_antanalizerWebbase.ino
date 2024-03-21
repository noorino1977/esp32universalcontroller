#include "si5351.h"
#include "Wire.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "soc/syscon_reg.h"
#include "soc/syscon_struct.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <TFT_eSPI.h>  
#include <SPI.h>

const char *ssid = "ANTENA_ANALYZER_YD1GSE";
const char *password = "noorino1977";


TFT_eSPI tft = TFT_eSPI();  

#define TFT_GREY 0xBDF7

Si5351 si5351;
int_fast32_t freq_manual =  7000000;
int_fast32_t freq_send =  0;
bool onoff=false;

  float REV; float analogVoltsref;  
  float FWD; float analogVoltsfwd; 
  int calb=150;
  float VSWR,minVSWR;

WiFiServer server(80);
WiFiClient client = server.available();


bool disable_5351 = true; // true untul test
byte modelnya = 3;   
float valueTest = 1.0;

void setup(){
  tft.init(); tft.setRotation(2); tft.fillScreen(TFT_BLACK);

  adc1_config_width(ADC_WIDTH_BIT_10); //ADC_WIDTH_BIT_9 ADC_WIDTH_BIT_10 ADC_WIDTH_BIT_11 ADC_WIDTH_BIT_12
  //adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_0);//ADC_ATTEN_DB_0 ADC_ATTEN_DB_2_5 ADC_ATTEN_DB_6 ADC_ATTEN_DB_11
  //adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0);
  //analogReadResolution(12);
  //analogReference(INTERNAL);  
  
  Serial.begin(115200);

  if (!WiFi.softAP(ssid, password)) {log_e("Soft AP creation failed."); while(1);  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");  Serial.println(myIP);
  server.begin();  Serial.println("Server started");

  if  (disable_5351==false){
        bool i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
        if(!i2c_found){ Serial.println("Device not found on I2C bus!");}
        
        si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
        //si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);
        //si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_4MA);
      
        si5351.set_phase(SI5351_CLK0, 0);
  }
  update_freq();
  
  if  (disable_5351==false) si5351.update_status();
  
  delay(500);
  Serial.println(freq_manual);
}

void loop(){
  client = server.available(); 
   if (client) {  
    Serial.println("oke");
     while (client.connected()) {
      if (client.available()) {
             if (modelnya == 1) lineGraphSWR();
             else if (modelnya == 2) barGraphSWR();
             else if (modelnya == 3) smithChartSWR();  
      }
     }
   }

  readAdc(); 
  tampilLCD();
  bacaserial();

}

void tampilLCD(){
  char munculFreq[20];char adcr[20];char adcf[20];char a[20];
  sprintf(munculFreq, "%d",freq_manual); 
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); tft.setTextSize(2); tft.drawString(munculFreq, 0, 0, 2); 
  sprintf(adcr, "adcF %f",FWD); 
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); tft.setTextSize(2); tft.drawString(adcr, 0, 30, 2);
  sprintf(adcr, "adcR %f ",REV); 
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); tft.setTextSize(2); tft.drawString(adcr, 0, 60, 2);  
  sprintf(a, "VSWR %f ",VSWR); 
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); tft.setTextSize(2); tft.drawString(a, 0, 120, 2);    
  //delay(500);
};
  
void readAdc(){
  int repeat = 3;
  for (int i = 0 ; i < repeat ;i++){
    
    //REV = analogRead(36);   
    //FWD = analogRead(39);
    REV = analogReadMilliVolts(37);
    FWD = analogReadMilliVolts(32);
   
    REV+=REV;
    FWD+=FWD;
    //delay(20);
  }
    REV/=repeat;
    FWD/=repeat;
  if (REV>=FWD) {VSWR = 999;}  else {    VSWR=(FWD+REV)/(FWD-REV); }
  //Serial.print(REV); Serial.print("|");Serial.println(FWD);
  
  //untuk test!!
  valueTest +=0.1; if (valueTest>5.0) valueTest = 1.0;
  VSWR = valueTest; 
  //Serial.println(VSWR);
}


void update_freq(){
     if  (disable_5351==false){
          si5351.set_freq(freq_manual * 100, SI5351_CLK0);
          //si5351.set_freq(freq_manual * 100, SI5351_CLK1);
          //si5351.set_freq(freq_manual * 100, SI5351_CLK2);
          //si5351.set_phase(SI5351_CLK0, 0); 
          //si5351.set_phase(SI5351_CLK1, 90); 
          si5351.update_status();
          Serial.println(freq_manual);
     }
      
 
      delay(50); 
}


void barGraphSWR() {
                    client.println("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
                    client.println("<!DOCTYPE HTML>");
                    client.println("<head>");           
                    client.println("<title>Bar Chart</title>");          // XXXXXXXXXXXXXXXX  judule   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                    client.println("<meta http-equiv=Content-Type content=text/html; charset=utf-8/>"); 
                    client.println("</head>");       // page background color
                    client.println("<h1 style=background-color:#EBFFD6>");   
                    client.println("<h5 style=background-color:#EBFFD6>");     
                    client.println("<text style=background-color:#EBFFD6>");    
                    //client.println("<br><br>"); 
                    client.println("<hr><hr>");    
                    client.println("<h1 style='text-align: center;'><FONT COLOR=#009900><i>Web Server Bar Chart Antenna Analyzer</i></FONT></h1>");
                    client.println("<hr>");
                    client.println("<br>"); 
                    //client.println("<h3 style='text-align: center;'><FONT COLOR=#000000>This is a simple web server that generates bar graphs to anyone connecting displaying; Voltage Standing Wave Ratio, resistance, reactance and complex impedance (magnitude and phase). It generates random values to demonstrate dynamic bar movement and color changes. The graphs are inside a HTML 5 Canvas area 1000 Wide X 700 High. Everything is presented relative to the canvas positions. Each bar is 30 pixels wide and up to 500 pixels tall. There are three functions; 1) Plot the blank graph, 2) Convert the five variables into pixels and colors, 3) Plot the results and print the value in text under the appropriate bar. Step is a setup and steps 2 & 3 are normally inside a loop. </FONT></h2>");                  
                    client.println("<br>"); 
                    client.println("<hr>");                                     
                    client.println("<br><br>");  
    
              
                     // Start of STATIC BAR Grapgh            // Start of STATIC BAR Grapgh            // Start of STATIC BAR Grapgh            // Start of STATIC BAR Grapgh            // Start of STATIC BAR Grapgh 
             
                    client.print(F("<CENTER><canvas id=\"MANUAL_Canvas\" width=\"1000\" height=\"700\" style=\"border:0px solid #000000;\">"));       // zero pixels to hide border on canvas   
                    client.print(F("<br><br><br>Sorry graphic content will not display, please update your browser to the lastest version; Minimum requiremnt for browsers:<br>Internet Explorer 9, Chrome 3, Firefox 3, Safari 3, Opera 10 and Android 3<br>XP OS users should use latest version of Chrome<br><br><br><br></canvas>"));      // old browser html5 not supported error message     
                    client.print(F("<script>"));
                    client.print(F("var c=document.getElementById(\"MANUAL_Canvas\");"));
                    client.print(F("var ctx=c.getContext(\"2d\");"));
                                       
                   // Top Title Text Line
                    client.print(F("ctx.fillStyle=\"#0000FF\";"));
                    client.print(F("ctx.font=\"32px Arial\";"));
                    client.print(F("ctx.fillText(\"Bar Graph ANtenna Analyzer Uhuy !!e\",225,30);"));
              
                  // Static VSWR manual bar chart text labels
                    client.print(F("ctx.fillStyle=\"#000000\";"));    // Blk
                    client.print(F("ctx.font=\"bold 32px Arial\";"));
                    client.print(F("ctx.fillText(\"VSWR\",65,635);"));  // ( text , left, ht )
                    client.print(F("ctx.fillStyle=\"#FFA500\";"));
                    client.print(F("ctx.font=\"20px Arial\";"));
                    client.print(F("ctx.fillText(\"+\",70,105);"));    
                    client.print(F("ctx.fillText(\"10\",63,155);"));  
                    client.print(F("ctx.fillText(\"9\",70,205);"));      
                    client.print(F("ctx.fillText(\"8\",70,255);"));      
                    client.print(F("ctx.fillText(\"7\",70,305);"));             
                    client.print(F("ctx.fillText(\"6\",70,355);"));    
                    client.print(F("ctx.fillText(\"5\",70,405);"));             
                    client.print(F("ctx.fillText(\"4\",70,455);")); 
                    client.print(F("ctx.fillText(\"3\",70,505);"));             
                    client.print(F("ctx.fillText(\"2\",70,555);"));             
                    client.print(F("ctx.fillText(\"1\",70,605);"));          
              
                  // Static R manual bar chart text labels
                    client.print(F("ctx.fillStyle=\"#000000\";"));    // Blk
                    client.print(F("ctx.font=\"bold 32px Arial\";"));
                    client.print(F("ctx.fillText(\"R\",265,635);"));  // ( text , left, ht )
                    client.print(F("ctx.fillStyle=\"#FFA500\";"));
                    client.print(F("ctx.font=\"20px Arial\";"));
                    client.print(F("ctx.fillText(\"500\",220,105);"));    
                    client.print(F("ctx.fillText(\"450\",220,155);"));   
                    client.print(F("ctx.fillText(\"400\",220,205);"));      
                    client.print(F("ctx.fillText(\"350\",220,255);"));      
                    client.print(F("ctx.fillText(\"300\",220,305);"));             
                    client.print(F("ctx.fillText(\"250\",220,355);"));    
                    client.print(F("ctx.fillText(\"200\",220,405);"));             
                    client.print(F("ctx.fillText(\"150\",220,455);")); 
                    client.print(F("ctx.fillText(\"100\",220,505);"));             
                    client.print(F("ctx.fillText(\"50\",231,555);"));             
                    client.print(F("ctx.fillText(\"0\",237,605);"));  
                     
                  // Static X manual bar chart text labels
                    client.print(F("ctx.fillStyle=\"#000000\";"));    // Blk
                    client.print(F("ctx.font=\"bold 32px Arial\";"));
                    client.print(F("ctx.fillText(\"X\",432,635);"));  // ( text , left, ht )
                    client.print(F("ctx.fillStyle=\"#FFA500\";"));
                    client.print(F("ctx.font=\"20px Arial\";"));
                    client.print(F("ctx.fillText(\"500\",387,105);"));    
                    client.print(F("ctx.fillText(\"450\",387,155);"));   
                    client.print(F("ctx.fillText(\"400\",387,205);"));      
                    client.print(F("ctx.fillText(\"350\",387,255);"));      
                    client.print(F("ctx.fillText(\"300\",387,305);"));             
                    client.print(F("ctx.fillText(\"250\",387,355);"));    
                    client.print(F("ctx.fillText(\"200\",387,405);"));             
                    client.print(F("ctx.fillText(\"150\",387,455);")); 
                    client.print(F("ctx.fillText(\"100\",387,505);"));             
                    client.print(F("ctx.fillText(\"50\",398,555);"));             
                    client.print(F("ctx.fillText(\"0\",404,605);"));  
                    
                  // Static Z manual bar chart text labels
                    client.print(F("ctx.fillStyle=\"#000000\";"));    // Blk
                    client.print(F("ctx.font=\"bold 32px Arial\";"));
                    client.print(F("ctx.fillText(\"Z\",599,635);"));  // ( text , left, ht )
                    client.print(F("ctx.fillStyle=\"#FFA500\";"));
                    client.print(F("ctx.font=\"20px Arial\";"));
                    client.print(F("ctx.fillText(\"500\",554,105);"));    
                    client.print(F("ctx.fillText(\"450\",554,155);"));   
                    client.print(F("ctx.fillText(\"400\",554,205);"));      
                    client.print(F("ctx.fillText(\"350\",554,255);"));      
                    client.print(F("ctx.fillText(\"300\",554,305);"));             
                    client.print(F("ctx.fillText(\"250\",554,355);"));    
                    client.print(F("ctx.fillText(\"200\",554,405);"));             
                    client.print(F("ctx.fillText(\"150\",554,455);")); 
                    client.print(F("ctx.fillText(\"100\",554,505);"));             
                    client.print(F("ctx.fillText(\"50\",565,555);"));             
                    client.print(F("ctx.fillText(\"0\",571,605);"));  
          
                  // Static ANG manual bar chart text labels
                    client.print(F("ctx.fillStyle=\"#000000\";"));    // Blk
                    client.print(F("ctx.font=\"italic bold 32px Arial\";"));
                    client.print(F("ctx.fillText(\"O\",766,635);"));  // ( text , left, ht )
                    client.print(F("ctx.fillStyle=\"#FFA500\";"));
                    client.print(F("ctx.font=\"20px Arial\";"));
                    client.print(F("ctx.fillText(\"90\",732,105);"));   
                    client.print(F("ctx.fillText(\"80\",732,161);"));      
                    client.print(F("ctx.fillText(\"70\",732,216);"));      
                    client.print(F("ctx.fillText(\"60\",732,271);"));             
                    client.print(F("ctx.fillText(\"50\",732,327);"));    
                    client.print(F("ctx.fillText(\"40\",732,382);"));             
                    client.print(F("ctx.fillText(\"30\",732,438);")); 
                    client.print(F("ctx.fillText(\"20\",732,494);"));             
                    client.print(F("ctx.fillText(\"10\",732,549);"));             
                    client.print(F("ctx.fillText(\"0\",738,605);"));            
          
                   // stroke line for theta symbol made from bold italic letter O     
                    client.print(F("ctx.beginPath();"));
                    client.print(F("ctx.rect(770,622,20,3);"));         
                    client.print(F("ctx.closePath();"));
                    client.print(F("ctx.lineWidth = 0;"));   //  zero is off // use  one for alignment of rect
                    client.print(F("ctx.fillStyle =\"#000000\";")); // blk
                    client.print(F("ctx.fill();"));
                    client.print(F("ctx.strokeStyle =\"#000000\";")); // blk
                    client.print(F("ctx.stroke();"));
                    client.print(F("</script>"));
                    
                     // END of STATIC BAR Grapgh                              // END of STATIC BAR Grapgh                              // END of STATIC BAR Grapgh                              // END of STATIC BAR Grapgh                              // END of STATIC BAR Grapgh         
      
 /////////////////////////////////////////////////////////////////////////////////////////
 // Math to calculate values goes here        
        //begin loop to draw plot
         float SWR;
         float R;
         float X;
         float Z;
         float A;
         char SIGN = '+';    
          
            //for (int slowdown = 1; slowdown < 10; slowdown = slowdown++ ) {     // loop for demonstration   
                delay (100);
                readAdc();tampilLCD();
                SWR = VSWR;                //( random(100, 999) ) / 100;  
                R = map(REV,0,4096,0,499); //random(0, 499);   
                X = map(FWD,0,4096,0,499); //random(0, 499);   
                Z = random(0, 499);  
                A = random(0, 85);   
          
 ////////////////////////////////////////////////////////////////////////////////////////                   
 
                      // Start Dynamic Canvas Plots             
                      client.print(F("<script>"));
                      client.print(F("var c=document.getElementById(\"MANUAL_Canvas\");"));
                      client.print(F("var ctx=c.getContext(\"2d\");"));
                      
                    // Dynamic convert SWR calc value to bar ht and color
                      // Dynamic SWR color of vertical bar
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(95,100,30,500);"));        // Rectangle ( Left, Down, Width, HT )    
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));
                          
                      int MBAR = 200 - ((SWR - 1) * 50); // changes SWR ( 1 to 9.99 ) to ( 500 to 0 )  //  should be "int MBAR = 500 - something" offset to make color reflect RF impact not true rainbow                     
                    // SWR color bar generaor  // converts # 500 to 0 into green to red // 500 is green and 0 is red
                      int RCOLOR = 250 - MBAR;
                      int ABGRN = MBAR - 250;
                      int GCOLOR = 250 - abs(ABGRN);
                      int BCOLOR = MBAR - 350;
                      RCOLOR = constrain(RCOLOR, 0, 255);
                      GCOLOR = constrain(GCOLOR, 0, 255);
                      BCOLOR = constrain(BCOLOR, 0, 255);
                      client.print(F("ctx.fillStyle=\"rgb("));
                      client.print(RCOLOR);
                      client.print(F(","));
                      client.print(GCOLOR);
                      client.print(F(",")); 
                      client.print(BCOLOR); 
                      client.print(F(")\";"));
                      client.print(F("ctx.fill();"));
                      client.print(F("ctx.strokeStyle = '000000';"));
                      client.print(F("ctx.stroke();"));
                      
                      // Dynamic clears VSWR top of vertical bar to show value in ht // note this is inverted
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(95,100,30,"));
                      MBAR = MBAR + 300; //  this corrects for the should be "int MBAR = 500 - something" offset to make bar ht correct to scale
                      MBAR = constrain(MBAR, 0, 600);  // limits bar size
                      client.print(MBAR);
                      client.print(F(");"));         
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));   //  zero is off // use  one for alignment of rect
                      client.print(F("ctx.fillStyle = '#FFFFFF';"));
                      client.print(F("ctx.fill();"));
                      client.print(F("ctx.strokeStyle = '#FFFFFF';"));
                      client.print(F("ctx.stroke();"));
            
                     //   Dynamic clears VSWR text value
                      // clears previous text
                      client.print("ctx.fillStyle=\"#ddffff\";");   // light blue // same as all pages
                      client.print("ctx.clearRect(70,646,80,25);");  // very important must clear rectangle area of last text write
                      
                   // Dynamic writes new VSWR text
                      client.print(F("ctx.fillStyle=\"#000000\";"));
                      client.print("ctx.font=\"20px Arial\";");
                      client.print("ctx.fillText(\"");
                      client.print(SWR);    // VSWR      
                      client.print("\",90,665);");
                      // Dynamic R color of vertical bar
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(262,100,30,500);"));        // Rectangle ( Left, Down, Width, HT )    
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));
            
                    // Dynamic convert R calc value to bar ht and color
                      // Dynamic R color of vertical bar
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(262,100,30,500);"));        // Rectangle ( Left, Down, Width, HT )    
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));
                       
                      MBAR = 300 - R; // changes R ( 0 to 500 ) to ( 500 to 0 )  //  should be "int MBAR = 500 - something" offset to make color reflect RF impact not true rainbow
                      
                    // R color bar generaor  // converts # 500 to 0 into green to red // 500 is green and 0 is red
                      RCOLOR = 250 - MBAR;
                      ABGRN = MBAR - 250;
                      GCOLOR = 250 - abs(ABGRN);
                      BCOLOR = MBAR - 250;
                      RCOLOR = constrain(RCOLOR, 0, 255);
                      GCOLOR = constrain(GCOLOR, 0, 255);
                      BCOLOR = constrain(BCOLOR, 0, 255);
                      client.print(F("ctx.fillStyle=\"rgb("));
                      client.print(RCOLOR);
                      client.print(F(","));
                      client.print(GCOLOR);
                      client.print(F(",")); 
                      client.print(BCOLOR); 
                      client.print(F(")\";"));
                      client.print(F("ctx.fill();"));
                      client.print(F("ctx.strokeStyle = '000000';"));
                      client.print(F("ctx.stroke();"));
            
                      // Dynamic clears R top of vertical bar to show value in ht // note this is inverted
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(262,100,30,"));
                      MBAR = MBAR + 200;   //  this corrects for the should be "int MBAR = 500 - something" offset to make bar ht correct to scale
                      MBAR = constrain(MBAR, 0, 600);  // limits bar size
                      client.print(MBAR);
                      client.print(F(");"));         
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));   //  zero is off // use  one for alignment of rect
                      client.print(F("ctx.fillStyle = '#FFFFFF';"));
                      client.print(F("ctx.fill();"));
                      client.print(F("ctx.strokeStyle = '#FFFFFF';"));
                      client.print(F("ctx.stroke();"));
            
                     //   Dynamic clears R text value
                      // clears previous text
                      client.print("ctx.fillStyle=\"#ddffff\";");   // light blue // same as all pages
                      client.print("ctx.clearRect(237,646,80,25);");  // very important must clear rectangle area of last text write
                      
                   // Dynamic writes new R text
                      client.print(F("ctx.fillStyle=\"#000000\";"));
                      client.print("ctx.font=\"20px Arial\";");
                      client.print("ctx.fillText(\"");
                      client.print(R,0);    // R      
                      client.print("\",265,665);");
   
                    // Dynamic convert X calc value to bar ht and color
                      // Dynamic X color of vertical bar
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(429,100,30,500);"));        // Rectangle ( Left, Down, Width, HT )    
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));
                                 
                      MBAR = 200 - X; // changes X ( 0 to 500 ) to ( 500 to 0 )  //  should be "int MBAR = 500 - something" offset to make color reflect RF impact not true rainbow
                      
                    // X color bar generaor  // converts # 500 to 0 into green to red // 500 is green and 0 is red
                      RCOLOR = 250 - MBAR;
                      ABGRN = MBAR - 250;
                      GCOLOR = 250 - abs(ABGRN);
                      BCOLOR = MBAR - 250;
                      RCOLOR = constrain(RCOLOR, 0, 255);
                      GCOLOR = constrain(GCOLOR, 0, 255);
                      BCOLOR = constrain(BCOLOR, 0, 255);
                      client.print(F("ctx.fillStyle=\"rgb("));
                      client.print(RCOLOR);
                      client.print(F(","));
                      client.print(GCOLOR);
                      client.print(F(",")); 
                      client.print(BCOLOR); 
                      client.print(F(")\";"));
                      client.print(F("ctx.fill();"));
                      client.print(F("ctx.strokeStyle = '000000';"));
                      client.print(F("ctx.stroke();"));
            
                      // Dynamic clears X top of vertical bar to show value in ht // note this is inverted
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(429,100,30,"));
                      MBAR = MBAR + 300;   //  this corrects for the should be "int MBAR = 500 - something" offset to make bar ht correct to scale
                      MBAR = constrain(MBAR, 0, 600);  // limits bar size
                      client.print(MBAR);
                      client.print(F(");"));         
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));   //  zero is off // use  one for alignment of rect
                      client.print(F("ctx.fillStyle = '#FFFFFF';"));
                      client.print(F("ctx.fill();"));
                      client.print(F("ctx.strokeStyle = '#FFFFFF';"));
                      client.print(F("ctx.stroke();"));
            
                     //   Dynamic clears X text value
                      // clears previous text
                      client.print("ctx.fillStyle=\"#ddffff\";");   // light blue // same as all pages
                      client.print("ctx.clearRect(404,646,80,25);");  // very important must clear rectangle area of last text write
                      
                   // Dynamic writes new X text
                      client.print(F("ctx.fillStyle=\"#000000\";"));
                      client.print("ctx.font=\"20px Arial\";");
                      client.print("ctx.fillText(\"");
                      client.print(X,0);    // X      
                      client.print("\",432,665);");
                      
                    // Dynamic convert Z calc value to bar ht and color
                      // Dynamic Z color of vertical bar
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(596,100,30,500);"));        // Rectangle ( Left, Down, Width, HT )    
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));
                                 
                      MBAR = 300 - Z; // changes X ( 0 to 500 ) to ( 500 to 0 )  //  should be "int MBAR = 500 - something" offset to make color reflect RF impact not true rainbow
                      
                    // Z color bar generaor  // converts # 500 to 0 into green to red // 500 is green and 0 is red
                      RCOLOR = 250 - MBAR;
                      ABGRN = MBAR - 250;
                      GCOLOR = 250 - abs(ABGRN);
                      BCOLOR = MBAR - 250;
                      RCOLOR = constrain(RCOLOR, 0, 255);
                      GCOLOR = constrain(GCOLOR, 0, 255);
                      BCOLOR = constrain(BCOLOR, 0, 255);
                      client.print(F("ctx.fillStyle=\"rgb("));
                      client.print(RCOLOR);
                      client.print(F(","));
                      client.print(GCOLOR);
                      client.print(F(",")); 
                      client.print(BCOLOR); 
                      client.print(F(")\";"));
                      client.print(F("ctx.fill();"));
                      client.print(F("ctx.strokeStyle = '000000';"));
                      client.print(F("ctx.stroke();"));
            
                      // Dynamic clears Z top of vertical bar to show value in ht // note this is inverted
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(596,100,30,"));
                      MBAR = MBAR + 200;   //  this corrects for the should be "int MBAR = 500 - something" offset to make bar ht correct to scale
                      MBAR = constrain(MBAR, 0, 600);  // limits bar size
                      client.print(MBAR);
                      client.print(F(");"));         
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));   //  zero is off // use  one for alignment of rect
                      client.print(F("ctx.fillStyle = '#FFFFFF';"));
                      client.print(F("ctx.fill();"));
                      client.print(F("ctx.strokeStyle = '#FFFFFF';"));
                      client.print(F("ctx.stroke();"));
            
                     //   Dynamic clears Z text value
                      // clears previous text
                      client.print("ctx.fillStyle=\"#ddffff\";");   // light blue // same as all pages
                      client.print("ctx.clearRect(571,646,80,25);");  // very important must clear rectangle area of last text write
                      
                   // Dynamic writes new Z text
                      client.print(F("ctx.fillStyle=\"#000000\";"));
                      client.print("ctx.font=\"20px Arial\";");
                      client.print("ctx.fillText(\"");
                      client.print(Z,0);    // Z      
                      client.print("\",599,665);");
                      
                    // Dynamic convert ANG calc value to bar ht and color
                      // Dynamic ANG color of vertical bar
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(763,100,30,500);"));        // Rectangle ( Left, Down, Width, HT )    
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));
                                
                      MBAR = 200 - (A * 5.55); // changes ANG ( 0 to 90 ) to ( 500 to 0 )  //  should be "int MBAR = 500 - something" offset to make color reflect RF impact not true rainbow
                      
                    // ANG color bar generaor  // converts # 500 to 0 into green to red // 500 is green and 0 is red
                      RCOLOR = 250 - MBAR;
                      ABGRN = MBAR - 250;
                      GCOLOR = 250 - abs(ABGRN);
                      BCOLOR = MBAR - 250;
                      RCOLOR = constrain(RCOLOR, 0, 255);
                      GCOLOR = constrain(GCOLOR, 0, 255);
                      BCOLOR = constrain(BCOLOR, 0, 255);
                      client.print(F("ctx.fillStyle=\"rgb("));
                      client.print(RCOLOR);
                      client.print(F(","));
                      client.print(GCOLOR);
                      client.print(F(",")); 
                      client.print(BCOLOR); 
                      client.print(F(")\";"));
                      client.print(F("ctx.fill();"));
                      client.print(F("ctx.strokeStyle = '000000';"));
                      client.print(F("ctx.stroke();"));
            
                      // Dynamic clears ANG top of vertical bar to show value in ht // note this is inverted
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.rect(763,100,30,"));
                      MBAR = MBAR + 300;   //  this corrects for the should be "int MBAR = 500 - something" offset to make bar ht correct to scale
                      client.print(MBAR);
                      client.print(F(");"));         
                      client.print(F("ctx.closePath();"));
                      client.print(F("ctx.lineWidth = 0;"));   //  zero is off // use  one for alignment of rect
                      client.print(F("ctx.fillStyle = '#FFFFFF';"));
                      client.print(F("ctx.fill();"));
                      client.print(F("ctx.strokeStyle = '#FFFFFF';"));
                      client.print(F("ctx.stroke();"));
            
                     //   Dynamic clears ANG text value
                      // clears previous text
                      client.print("ctx.fillStyle=\"#ddffff\";");   // light blue // same as all pages
                      client.print("ctx.clearRect(744,646,80,25);");  // very important must clear rectangle area of last text write
                      
                   // Dynamic writes new ANG text
                      client.print(F("ctx.fillStyle=\"#000000\";"));
                      client.print("ctx.font=\"20px Arial\";");
                      client.print("ctx.fillText(\"");
                      client.print(A,0);    // ANG      
                      client.print("\",772,665);");          
            
                   // Dynamic writes new sign for phase angle text
                      client.print(F("ctx.fillStyle=\"#000000\";"));
                      client.print("ctx.font=\"20px Arial\";");
                      client.print("ctx.fillText(\"");
                      client.print(SIGN);    // pos or neg symbol      
                      client.print("\",750,665);");          
                        
                      client.print(F("</script>"));
                
                      // END of Dynamic BAR Grapgh Single Frequency Mode         // END of STATIC BAR Grapgh Single Frequency Mode        
                     
            
          // End Dynamic Canvas Plots       // End Dynamic Canvas Plots        // End Dynamic Canvas Plots        // End Dynamic Canvas Plots        // End Dynamic Canvas Plots                 
                   
          //}   //end loop to draw plot          

          // Bottom Info Text Line
          client.println(F("<br><br>"));
          client.println("The Voltage Standing Wave Ratio, resistance, reactance and complex impedance (magnitude and phase) antenna.");   
          //client.println(F("<br><br><br><br>"));
          //client.stop();
}

void smithChartSWR(){            
                    client.println("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
                    client.println("<!DOCTYPE HTML>");
                    client.println("<head>");           
                    client.println("<title>Smith Chart</title>");          // XXXXXXXXXXXXXXXX  Title in browser tab   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                    client.println("<meta http-equiv=Content-Type content=text/html; charset=utf-8/>"); 
                    client.println("</head>");       // page background color
                    client.println("<h1 style=background-color:#EBFFD6>");   
                    client.println("<h5 style=background-color:#EBFFD6>");     
                    client.println("<text style=background-color:#EBFFD6>");    
                    client.println("<br><br>"); 
                    client.println("<hr><hr>");    
                    client.println("<h1 style='text-align: center;'><FONT COLOR=#009900><i>ESP32 Web Server Smith Chart and Plotter</i></FONT></h1>");
                    client.println("<hr>");
                    client.println("<br>"); 
                    client.println("<h3 style='text-align: center;'><FONT COLOR=#000000>This is a simple web server that generates a Smith Chart to anyone connecting. It includes the calculations to Plot Smith Chart Positions from inputs of R, X & SIGN. The demonstration loop may seem slow as it draws an arc from 0 to +25 X in 1/100 Ohm steps with a constant R = 25 Ohms. Each position is plotted with a 3 X 3 pixel dot, it seems small but it works. When using real sweeps having 2,500 or more data points like this demonstration is common.</FONT></h2>");                  
                    client.println("<br>"); 
                    client.println("<hr>");                                     
                    client.println("<br><br>");  
                   
                    //  Start of Static Smith Chart for Single Frequency        
  
                          // Plot area 1000 Wide X 700 High
                          client.print(F("<CENTER><canvas id=\"SMTHCHRT_Canvas\" width=\"1000\" height=\"700\" style=\"border:0px solid #000000;\">"));       // zero pixels to hide border on canvas   
                          client.print(F("<br><br><br>Sorry graphic content will not display, please update your browser to the lastest version; Minimum requiremnt for browsers:<br>Internet Explorer 9, Chrome 3, Firefox 3, Safari 3, Opera 10 and Android 3<br>XP OS users should use latest version of Chrome<br><br><br><br></canvas>"));      // old browser html5 not supported error message     
                          client.print(F("<script>"));
                          client.print(F("var c=document.getElementById(\"SMTHCHRT_Canvas\");"));
                          client.print(F("var ctx=c.getContext(\"2d\");"));
                 
                          // Outside cirlce // center of plot 500, 350 // circle radis 350 or diameter 700
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth = 3;"));
                          client.print(F("ctx.strokeStyle = '#FFA500';"));  // orange
                          client.print(F("ctx.arc(500,350,325,0,2*Math.PI);"));
                          client.print(F("ctx.fillStyle = '#F0F0B2';"));  // oscope green  // fill only used on main circle
                          client.print(F("ctx.fill();"));  // fill only used on main circle
                          client.print(F("ctx.stroke();"));             
                          client.print(F("ctx.closePath();"));
                     
                          // R 10 Ohm cirlce (0.2)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.arc(554,350,271,0,2*Math.PI);"));
                          client.print(F("ctx.closePath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#808000';"));  // olive
                          client.print(F("ctx.stroke();"));
                       
                          // R 25 Ohm cirlce (0.5)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.arc(608,350,217,0,2*Math.PI);"));
                          client.print(F("ctx.closePath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#808000';"));  // olive
                          client.print(F("ctx.stroke();"));
                       
                          // R 50 Ohm cirlce (1.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.arc(662,350,162,0,2*Math.PI);"));
                          client.print(F("ctx.closePath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#808000';"));  // olive
                          client.print(F("ctx.stroke();")); 
                                
                          // R 100 Ohm cirlce (2.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.arc(717,350,108,0,2*Math.PI);"));
                          client.print(F("ctx.closePath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#808000';"));  // olive
                          client.print(F("ctx.stroke();"));
                                 
                          // R 150 Ohm cirlce (3.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.arc(744,350,81,0,2*Math.PI);"));
                          client.print(F("ctx.closePath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#808000';"));  // olive
                          client.print(F("ctx.stroke();"));
                             
                          // R 200 Ohm cirlce (4.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.arc(760,350,65,0,2*Math.PI);"));
                          client.print(F("ctx.closePath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#808000';"));  // olive
                          client.print(F("ctx.stroke();"));
                            
                          // R 300 Ohm cirlce (6.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.arc(779,350,46,0,2*Math.PI);"));
                          client.print(F("ctx.closePath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#808000';"));  // olive
                          client.print(F("ctx.stroke();"));
                               
                          // R 500 Ohm cirlce (10.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.arc(796,350,29,0,2*Math.PI);"));
                          client.print(F("ctx.closePath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#808000';"));  // olive
                          client.print(F("ctx.stroke();"));
                
                           // Resistive Line (Horz Line)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#000000\";")); // black
                          client.print(F("ctx.moveTo(150,350);"));
                          client.print(F("ctx.lineTo(850,350);"));
                          client.print(F("ctx.closePath();"));
                          client.print(F("ctx.stroke();"));         
                      
                          // X +10 Ohm Arc (+0.2)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,-1275,1625, 0.5*Math.PI, 0.626*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          // the center of the circle is negative because it is outside the canvas however it displays the correct arc  
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));              
                   
                          // X +25 Ohm Arc (+0.5)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,-300,650, 0.5*Math.PI, 0.795*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          // the center of the circle is negative because it is outside the canvas however it displays the correct arc 
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));            
                    
                          // X +50 Ohm Arc (+1.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,25,325, 0.5*Math.PI, Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));             
                   
                          // X +100 Ohm Arc (+2.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,188,162, Math.PI/2, 1.2*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));             
                  
                          // X +150 Ohm Arc (+3.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,242,108, Math.PI/2, 1.3*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));             
                 
                          // X +200 Ohm Arc (+4.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,269,81, Math.PI/2, 1.34*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));         
                      
                          // X -10 Ohm Arc (-0.2)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,1975,1625, 1.374*Math.PI, 1.5*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          // the center of the circle is greater than 700 because it is outside the canvas however it displays the correct arc  
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));               
                    
                          // X -25 Ohm Arc (-0.5)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,1000,650, 1.205*Math.PI, 1.5*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          // the center of the circle is greater than 700 because it is outside the canvas however it displays the correct arc  
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));
                      
                           // X -50 Ohm Arc (-1.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,675,325, Math.PI, 1.5*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));             
                         
                          // X -100 Ohm Arc (-2.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,512,162, 0.8*Math.PI, 1.5*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));             
                         
                          // X -150 Ohm Arc (-3.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,458,108, 0.7*Math.PI, 1.5*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));             
                                
                          // X -200 Ohm Arc (-4.0)
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth=1;"));
                          client.print(F("ctx.strokeStyle=\"#FF8080\";")); // Light red
                          client.print(F("ctx.arc(825,431,81, 0.66*Math.PI, 1.5*Math.PI, false);"));  //  center xxx, center yyy, radius, start (radians 0=3PM, 0.5=6PM, 1=9PM, 1.5=12PM), stop (radians), clockwise = false or counter clockwise = true
                          client.print(F("ctx.stroke();")); 
                          client.print(F("ctx.closePath();"));
                 
                          // 2:1 VSWR Circle // center of plot 500, 350
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#6666FF';"));  // light blue
                          client.print(F("ctx.setLineDash([3,6,1,2]);")); // dashed = first dash is 3 pixels, then a space of 6 pixels, then a dash of 1 pixels, then a space of 2 pixels, 
                          client.print(F("ctx.arc(500,350,108,0,2*Math.PI);"));  // 
                          client.print(F("ctx.stroke();"));             
                          client.print(F("ctx.closePath();"));
                  
                          // 3:1 VSWR Circle // center of plot 500, 350
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#6666FF';"));  // light blue
                          client.print(F("ctx.setLineDash([3,6,1,2]);")); // dashed = first dash is 3 pixels, then a space of 6 pixels, then a dash of 1 pixels, then a space of 2 pixels, 
                          client.print(F("ctx.arc(500,350,162,0,2*Math.PI);"));  // 
                          client.print(F("ctx.stroke();"));             
                          client.print(F("ctx.closePath();"));
                 
                          // 5:1 VSWR Circle // center of plot 500, 350
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#6666FF';"));  // light blue
                          client.print(F("ctx.setLineDash([3,6,1,2]);")); // dashed = first dash is 3 pixels, then a space of 6 pixels, then a dash of 1 pixels, then a space of 2 pixels, 
                          client.print(F("ctx.arc(500,350,216,0,2*Math.PI);"));  // 
                          client.print(F("ctx.stroke();"));             
                          client.print(F("ctx.closePath();"));
                 
                          // 10:1 VSWR Circle // center of plot 500, 350
                          client.print(F("ctx.beginPath();"));
                          client.print(F("ctx.lineWidth = 1;"));
                          client.print(F("ctx.strokeStyle = '#6666FF';"));  // light blue
                          client.print(F("ctx.setLineDash([3,6,1,2]);")); // dashed = first dash is 3 pixels, then a space of 6 pixels, then a dash of 1 pixels, then a space of 2 pixels, 
                          client.print(F("ctx.arc(500,350,267,0,2*Math.PI);"));  // 
                          client.print(F("ctx.stroke();"));             
                          client.print(F("ctx.closePath();"));
                      
                          // text is added last to ensure text is written on top           // text is added last to ensure text is written on top                             
                          // Orange text 
                          client.print(F("ctx.fillStyle=\"#FFA500\";")); // orange
                          client.print(F("ctx.font=\"20px Arial\";"));
                          client.print(F("ctx.fillText(\"Shorted\",60,355);"));
                          client.print(F("ctx.fillText(\"Open\",870,355);"));     
                          client.print(F("ctx.fillText(\"Ind\",505,55);"));     
                          client.print(F("ctx.fillText(\"Cap\",505,660);"));
                               
                          // Resistance text RRRRRRRR
                          client.print(F("ctx.fillStyle=\"#000000\";")); // black
                          client.print(F("ctx.font=\"10px Arial\";"));
                          client.print(F("ctx.fillText(\"10 R\",297,362);"));
                          client.print(F("ctx.fillText(\"25 R\",395,345);"));   
                          client.print(F("ctx.fillText(\"50 R\",505,362);"));  
                          client.print(F("ctx.fillText(\"100 R\",615,345);"));
                          client.print(F("ctx.fillText(\"150 R\",665,362);"));
                          client.print(F("ctx.fillText(\"200 R\",700,345);"));
                          client.print(F("ctx.fillText(\"300 R\",738,362);"));
                          client.print(F("ctx.fillText(\"500 R\",775,345);"));
                               
                          // Reactance text XXXXXXXX
                          client.print(F("ctx.fillStyle=\"#FF0000\";")); // red
                          client.print(F("ctx.font=\"10px Arial\";"));
                          client.print(F("ctx.fillText(\"+10 X\",240,225);"));
                          client.print(F("ctx.fillText(\"-10 X\",240,480);"));
                          client.print(F("ctx.fillText(\"+25 X\",340,115);"));
                          client.print(F("ctx.fillText(\"-25 X\",340,595);"));
                          client.print(F("ctx.fillText(\"+50 X\",467,50);"));
                          client.print(F("ctx.fillText(\"-50 X\",467,660);"));
                          client.print(F("ctx.fillText(\"+100 X\",635,125);"));
                          client.print(F("ctx.fillText(\"-100 X\",635,585);"));
                          client.print(F("ctx.fillText(\"+150 X\",695,185);"));
                          client.print(F("ctx.fillText(\"-150 X\",695,522);"));
                          client.print(F("ctx.fillText(\"+200 X\",755,245);"));
                          client.print(F("ctx.fillText(\"-200 X\",755,463);"));                
                               
                          // VSWR text SWR SWR SWR
                          client.print(F("ctx.fillStyle=\"#0000FF\";")); // blue
                          client.print(F("ctx.font=\"10px Arial\";"));
                          client.print(F("ctx.fillText(\"2:1\",485,237);"));
                          client.print(F("ctx.fillText(\"3:1\",484,185);"));   
                          client.print(F("ctx.fillText(\"5:1\",482,130);"));   
                          client.print(F("ctx.fillText(\"10:1\",477,79);"));   
                          
                          client.print(F("</script>"));
                          client.println(F("<br>"));
                          
                         // End of Static Smith Chart Canvas          
                          // End Static Canvas Setup           
                    
 /////////////////////////////////////////////////////////////////////////////////////////
 // Math to calculate values goes here  
        
        //begin loop to draw plot
          for (float X = 0; X < 25; X = X + 0.01 ){      // Reactance range 0 to 25 for demonstration        
            float R = 25;      // R Resistance value 25 for demonstration
            char SIGN = '+';  // Reactance sign = "-" or "+"  // characters have single quotes
            
 ////////////////////////////////////////////////////////////////////////////////////////                   
 
 
            
            //  Start of Dynamic Smith Chart for Sweep         //  Start of Dynamic Smith Chart for Sweep         //  Start of Dynamic Smith Chart for Sweep         
                   
                           // Solving for Smith Chart Plot      // Solving for Smith Chart Plot        // Solving for Smith Chart Plot        // Solving for Smith Chart Plot
                          //   Smith Chart Plot  Postion is determined from  R, X & SIGN 
                      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                      
                      // Solve for Smith Chart Position from R and X using intersection of circles
                      
                      // RRR = R radius = [ 1 / (1+r) X 325 ] were r = Resistance / 50
                      float RRR = 325 / ( 1 + (R/50));
                      // XXR = X radius = [ 1 / (x) X 325 ] were (x) = Reactance / 50
                      float XXR = 325 / (X/50);
                      // RRH = R circle center horz point = [ 325 X (r) / (1+(r)) ] + 500 were (r) = Resistance / 50
                      float RRH = ((325 * (R/50)) / (1 + (R/50))) + 500;
                      // XXV = X circle center vert point = [ 325 / (x) ] + 350 were (x) = Reactance / 50
                      // when SIGN negative X is below the center (350) or down, which is a higher number in html canvas
                      float XXV;
                      if ( SIGN == '-' )   //  determines top or bottom of chart
                          {
                      XXV = 350 + (325 / (X/50));
                          }
                      else
                          {
                      XXV = 350 - (325 / (X/50));
                          }
                      // DSQ = distance between centers or d^2 = (825-RRH)^2  +  (XXV-350)^2
                      float DSQ = ((825-RRH)*(825-RRH))  +  ((XXV-350)*(XXV-350));
                      // ATC = (1/4)sqrt(((RRR+XXR)^2-DSQ)(DSQ-(RRR-XXR)^2))
                      float ATC = 0.25 * sqrt( (((RRR+XXR)*(RRR+XXR))-DSQ) * (DSQ-((RRR-XXR)*(RRR-XXR))) );
                      // SMTHORZ = canvas horizontal value
                      // Resistance circle center vert point = 350
                      // Reactance circle center horz point = 825
                      // There is one horizontal solution, the second is not needed as both circle always touch at infinity (825, 350)
                      // SMTHORZ = [ (1/2)(825+RRH) ] + [ (1/2)(825-RRH)(RRR^2-XXR^2) / DSQ ] + [ 2(XXV-350)ATC/DSQ ]
                      // SMTVERT = canvas vertical value
                      // There are two vertical solutions, add the third equation for negative "SIGN", else subtract as shown here
                      // Resistance circle center vert point = 350
                      // Reactance circle center horz point = 825
                      // SMTVERT = [ (1/2)(XXV+350) ] + [ (1/2)(XXV-350)(RRR^2-XXR^2) / DSQ ] - [ 2(825-RRH)ATC/DSQ ]
                      float SMTHORZ;
                      float SMTVERT;
                      if ( SIGN == '-' )   //  determines top or bottom of chart
                          {
                      SMTHORZ = ( 0.5 * (825+RRH) ) + ( 0.5 * (825-RRH) * ((RRR*RRR) -(XXR*XXR)) / DSQ ) - ( 2 * (XXV-350) * ATC / DSQ );
                      SMTVERT = ( 0.5 *(XXV+350) ) + ( 0.5 * (XXV-350) * ((RRR*RRR) - (XXR*XXR)) / DSQ ) + ( 2 * (825-RRH) * ATC / DSQ );
                          }
                      else
                          {
                      SMTHORZ = ( 0.5 * (825+RRH) ) + ( 0.5 * (825-RRH) * ((RRR*RRR) -(XXR*XXR)) / DSQ ) + ( 2 * (XXV-350) * ATC / DSQ );
                      SMTVERT = ( 0.5 *(XXV+350) ) + ( 0.5 * (XXV-350) * ((RRR*RRR) - (XXR*XXR)) / DSQ ) - ( 2 * (825-RRH) * ATC / DSQ );
                          }
                      
                      // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
            
                      
                  //   Smith Chart Dynamic scatter plot graph of data
                        client.print(F("<script>"));
                        client.print(F("var c=document.getElementById(\"SMTHCHRT_Canvas\");"));
                        client.print(F("var ctx=c.getContext(\"2d\");"));
                        client.print("ctx.beginPath();");
                        client.print("ctx.lineWidth=1;");
                        client.print("ctx.strokeStyle=\"#00FF00\";"); // green
                        client.print("ctx.rect("); 
                        client.print(SMTHORZ); 
                        client.print(",");
                        client.print(SMTVERT);
                        client.print(",3,3);");  // nine pixel sq
                        client.print("ctx.stroke();");            
                        client.print("</script>");                 
                      //  End of Dynamic Smith Chart for Sweep                 
            // End Dynamic Canvas Plots     
          //end loop to draw plot           
          }           
                    client.println("<br><br><br><br><br><br><br><br>");   
                    client.stop();            
}

void lineGraphSWR(){                  
                    client.println("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
                    client.println("<!DOCTYPE HTML>");
                    client.println("<head>");           
                    client.println("<title>VSWR Plotter</title>");          // XXXXXXXXXXXXXXXX  Title in browser tab   XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                    client.println("<meta http-equiv=Content-Type content=text/html; charset=utf-8/>"); 
                    client.println("</head>");       // page background color
                    client.println("<h1 style=background-color:#EBFFD6>");   
                    client.println("<h5 style=background-color:#EBFFD6>");     
                    client.println("<text style=background-color:#EBFFD6>");    
                    client.println("<br><br>"); 
                    client.println("<hr><hr>");    
                    client.println("<h1 style='text-align: center;'><FONT COLOR=#009900><i>ESP32 Web Server VSWR Plotter</i></FONT></h1>");
                    client.println("<hr>");
                    client.println("<br>"); 
                    client.println("<h3 style='text-align: center;'><FONT COLOR=#000000>This is a simple web server that generates a VSWR plot vs. Frequency to anyone connecting. It includes the calculations to Plot VSWR Positions from inputs of; VSWR, sweep start frequency, sweep stop frequency, sweep step size. The VSWR Plot is inside a HTML 5 Canvas area 1000 Wide X 200 High. Everything is presented relative to the canvas positions. The demonstration loop draws a random plot with a 1:1 ratio at 20M mid-band 14,175 KHz. Each position is plotted with a 1 pixel wide line, connecting point to point. There are three functions; 1) Plot the blank graph, 2) Convert start and stop frequencies in chart labels, 3) Plot the results. Steps 1 & 2 are setups and 3 is normally inside a loop.</FONT></h2>");                  
                    client.println("<br>"); 
                    client.println("<hr>");                                     
                    client.println("<br><br>");  
   
    
              // Simulated input values for demonstration sweep setup           
                   
                   float centerF = 7100;   // center khz  
                   float low  = centerF - 2000; // Sweep Start 
                   float high = centerF + 2000; // Sweep end  
                   
            // Start Static Canvas Setup                         
                  // var for static sweep scale          
                    float swp1qtr = low + ((high-low)/4);       // Scale Freq
                    float swp2qtr = low + ((high-low)/2);       // Scale Freq
                    float swp3qtr = low + ((3*(high-low))/4);   // Scale Freq
                    float KHZDIV= (high-low)/40;                // KHz per Div 
          
                //   VSWR Static Canvas begin  
                    client.print(F("<CENTER><canvas id=\"SWR_Canvas\" width=\"1000\" height=\"200\" style=\"border:0px solid #000000;\">"));       // zero pixels to hide border on canvas   
                    client.print(F("Sorry digital storage oscilloscope will not display, please update your browser</canvas>"));      // old browser html5 not supported error message                   
                    client.print(F("<script>"));
                    client.print(F("var c=document.getElementById(\"SWR_Canvas\");"));
                    client.print(F("var ctx=c.getContext(\"2d\");"));
          
                    // Static HORZIZONTAL hollow rectangle // WHITE BACKGROUND
                    client.print(F("ctx.beginPath();"));
                    client.print(F("ctx.rect(90,5,800,180);"));      // Rectangle Left 0, Down 20, Width 300, HT 50   
                    client.print(F("ctx.closePath();"));
                    client.print(F("ctx.lineWidth = 1;"));
                    client.print(F("ctx.fillStyle = '#FFFFFF';"));  // white
                    client.print(F("ctx.fill();"));
                    client.print(F("ctx.strokeStyle = '##808000';")); // olive
                    client.print(F("ctx.stroke();"));

//   Static KHz per Div text value in SWR canvas         
                    client.print(F("ctx.fillStyle=\"#000000\";"));
                    client.print(F("ctx.font=\"12px Arial\";"));
                    client.print(F("ctx.fillText(\""));
                    client.print(KHZDIV);   //  KHz per Div     
                    client.print(F("\",10,110);"));
                    
                 // VSWR osciloscope Static text labels
                    client.print(F("ctx.fillStyle=\"#00FF00\";"));
                    client.print(F("ctx.font=\"20px Arial\";"));
                    client.print(F("ctx.fillText(\"VSWR\",5,20);"));
                    client.print(F("ctx.fillStyle=\"#FFA500\";"));
                    client.print(F("ctx.font=\"16px Arial\";"));
                    client.print(F("ctx.fillText(\"Ratio\",10,40);"));
                    client.print(F("ctx.fillText(\"KHz/Div\",10,90);"));
                    client.print(F("ctx.fillText(\"Freq\",10,140);"));
                    client.print(F("ctx.fillText(\"KHz\",10,180);"));
                    client.print(F("ctx.font=\"12px Arial\";"));
                    client.print(F("ctx.fillText(\"+\",77,10);"));    
                    client.print(F("ctx.fillText(\"9\",77,28);"));      
                    client.print(F("ctx.fillText(\"8\",77,48);"));      
                    client.print(F("ctx.fillText(\"7\",77,68);"));             
                    client.print(F("ctx.fillText(\"6\",77,88);"));    
                    client.print(F("ctx.fillText(\"5\",77,108);"));             
                    client.print(F("ctx.fillText(\"4\",77,128);")); 
                    client.print(F("ctx.fillText(\"3\",77,148);"));             
                    client.print(F("ctx.fillText(\"2\",77,168);"));             
                    client.print(F("ctx.fillText(\"1\",77,188);"));                      
                    
               // Static frequency freq labels below canvas
                    client.print(F("ctx.fillText(\"KHz\",125,200);"));  
                    client.print(F("ctx.fillText(\"KHz\",330,200);"));  
                    client.print(F("ctx.fillText(\"KHz\",530,200);"));  
                    client.print(F("ctx.fillText(\"KHz\",730,200);"));  
                    client.print(F("ctx.fillText(\"KHz\",925,200);"));  
                    client.print(F("ctx.fillText(\""));
                    client.print(low);   // sweep start freq      
                    client.print(F("\",72,200);"));
                    client.print(F("ctx.fillText(\""));
                    client.print(swp1qtr);   // sweep 1/4 freq      
                    client.print(F("\",271,200);"));
                    client.print(F("ctx.fillText(\""));
                    client.print(swp2qtr);   // sweep 1/2 freq      
                    client.print(F("\",470,200);"));
                    client.print(F("ctx.fillText(\""));
                    client.print(swp3qtr);   // sweep 3/4 freq      
                    client.print(F("\",669,200);"));
                    client.print(F("ctx.fillText(\""));
                    client.print(high);   // sweep stop freq      
                    client.print(F("\",867,200);"));
                    
                    // olive osciloscope Static grid of rectangles
                    // olive osciloscope Static grid VERTICAL
                    for (int tekgrdv = 90; tekgrdv < 860; tekgrdv = tekgrdv + 20 ){  
                        client.print(F("ctx.beginPath();"));
                        client.print(F("ctx.lineWidth=1;"));
                        client.print(F("ctx.strokeStyle=\"#808000\";")); // olive
                        client.print(F("ctx.rect("));
                        client.print(tekgrdv);
                        client.print(F(",5,20,180);"));         
                        client.print(F("ctx.stroke();"));
                    }
                    // orn freq Static vetical maker line
                    client.print(F("ctx.beginPath();"));
                    client.print(F("ctx.lineWidth=1;"));
                    client.print(F("ctx.strokeStyle=\"#FFA500\";")); // orn 1/4 Sweep
                    client.print(F("ctx.rect(290,5,1,180);"));         
                    client.print(F("ctx.stroke();"));
                    client.print(F("ctx.beginPath();"));
                    client.print(F("ctx.lineWidth=1;"));
                    client.print(F("ctx.strokeStyle=\"#FFA500\";")); // orn 1/2 Sweep
                    client.print(F("ctx.rect(490,5,1,180);"));         
                    client.print(F("ctx.stroke();"));
                    client.print(F("ctx.beginPath();"));
                    client.print(F("ctx.lineWidth=1;"));
                    client.print(F("ctx.strokeStyle=\"#FFA500\";")); // orn 3/4 Sweep
                    client.print(F("ctx.rect(690,5,1,180);"));         
                    client.print(F("ctx.stroke();"));
                    
                    // olive osciloscope Static grid HORZIZONTAL
                    for (int tekgrdh = 5; tekgrdh < 180; tekgrdh = tekgrdh + 20 ){  
                      client.print(F("ctx.beginPath();"));
                      client.print(F("ctx.lineWidth=1;"));
                      client.print(F("ctx.strokeStyle=\"#808000\";")); // olive
                      client.print(F("ctx.rect(90,"));
                      client.print(tekgrdh);
                      client.print(F(",800,20);"));         
                      client.print(F("ctx.stroke();"));
                    }
          
                  // Intialize VSWR  Line Plot
                    client.print(F("ctx.beginPath();"));
                    int pointXS = 91;
                    int pointYS = 185;
                    client.print(F("ctx.moveTo("));
                    client.print(pointXS);
                    client.print(",");
                    client.print(pointYS);
                    client.print(F(");"));         
                    client.print(F("</script>"));
                    client.print(F("</body>"));
                    client.print(F("<hr>"));
          
              //   VSWR Static Canvas end
              // End Static Canvas Setup           
                    
                    
 /////////////////////////////////////////////////////////////////////////////////////////
 // Math to calculate VSWR values goes here  
        
        //begin loop to draw plot
         float SWR;        
         for (float FREQ = low; FREQ < high ; FREQ = FREQ + 2 ) {    // 1 KHz step size                        
                freq_manual = FREQ * 1000; update_freq(); tampilLCD(); delay(100);
                readAdc();
                SWR = VSWR;           
     

//   VSWR dynamic line graph of data from CSV data output
          client.print("<script>");
          client.print("var c=document.getElementById(\"SWR_Canvas\");");  // swr canvas 
          client.print("var ctx=c.getContext(\"2d\");");
          client.print("ctx.lineWidth=1;");
          client.print("ctx.strokeStyle=\"#00FF00\";");  // green
          pointXS = (((FREQ-low)/(high-low)) * 800) + 89 ; // 89 is left side of plot  // 800 steps left to right  // freq - low is relative position // scale is high - low
          pointYS = 185 - ((SWR * 20)-20);   // 185 is bottom of plot  // 5 starts at top of plot // SWR is 1 to 9.99 on scale of 0 to 180 so times 20 and neg 20 is because SWR starts at 1.00 not zero
          client.print("ctx.lineTo(");
          client.print(pointXS);
          client.print(",");
          client.print(pointYS);
          client.print(");");
          client.print("ctx.stroke();");            
          client.print("</script>");
          
 //   Insert dynamic VSWR text value in canvas from CSV data output 
          client.print("<script>");
          client.print("var c=document.getElementById(\"SWR_Canvas\");");   // SWR canvas 
          client.print("var ctx=c.getContext(\"2d\");");
          client.print("ctx.fillStyle=\"#000000\";");
          client.print("ctx.clearRect(10,45,55,20);");  // very important must clear rectangle area of last text write
          client.print("ctx.font=\"16px Arial\";");
          client.print("ctx.fillText(\"");
          client.print(SWR);   //  VSWR      
          client.print("\",10,60);");
          client.print("</script>");
          
 //   Insert dynamic FREQUENCY text value in canvas from CSV data output
          client.print("<script>");
          client.print("var c=document.getElementById(\"SWR_Canvas\");");   // SWR canvas 
          client.print("var ctx=c.getContext(\"2d\");");
          client.print("ctx.fillStyle=\"#000000\";");
          client.print("ctx.clearRect(10,150,55,10);");  // very important must clear rectangle area of last text write
          client.print("ctx.font=\"12px Arial\";");
          client.print("ctx.fillText(\"");
          client.print(FREQ);    // this number needs to be in KHz      
          client.print("\",10,160);");
          client.print("</script>");
            
             //end loop to draw plot     
          //client.flush();      
     }           
          client.println("<br><br><br><br><br><br><br><br>");   
         //client.stop();
              
}





void bacaserial(){
  if (Serial.available() > 0)  {
    char key = Serial.read();
    switch (key){
    case 'a':
      freq_manual+=10;update_freq();
      break;
    case 'z':
      freq_manual-=10;update_freq();    
      break;  
    case 's':
      freq_manual+=100;update_freq();      
      break;
    case 'x':
      freq_manual-=100; update_freq();     
      break;      
    case 'd':
      freq_manual+=1000;update_freq();      
      break;
    case 'c':
      freq_manual-=1000; update_freq(); 
      break; 
    case 'f':
      freq_manual+=10000; update_freq();  
      break;
    case 'v':
      freq_manual-=10000;update_freq();
      break; 
    case 'g':
      freq_manual+=100000; update_freq();  
      break;
    case 'b':
      freq_manual-=100000;update_freq();
      break;  
    case 'h':
      freq_manual+=1000000; update_freq();  
      break;
    case 'n':
      freq_manual-=1000000;update_freq();
      break;       
    case 'y':
      //onoff=!onoff;digitalWrite(txrx,onoff);
      break;       
    case 't':
      si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
      update_freq();
      break; 
    case 'm':
      modelnya++;
      if (modelnya>3) modelnya=1;
       Serial.println(modelnya);   
      break;      

    }
  }  
}


 
