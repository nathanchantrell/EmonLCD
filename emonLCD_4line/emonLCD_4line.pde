//--------------------------------------------------------------------------------------------------------
// LCD Energy Monitor Display using a 4 line LCD display connected with Sparkfun SerLCD Serial Backpack
//
// Based on OpenEnergyMonitor.org emonGLCD example by Trystan Lea and Glyn Hudson at OpenEnergyMonitor.org
// Adapted for 4 Line Serial LCD by Nathan Chantrell Hardware details at http://zorg.org/yeg
//--------------------------------------------------------------------------------------------------------

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Ports.h>
#include <RF12.h> // needed to avoid a linker error :(
#include <avr/pgmspace.h>

// fixed RF12 settings
#define MYNODE 28            //node ID 30 reserved for base station
#define freq RF12_433MHZ     //frequency
#define group 210            //network group 

#define ONE_WIRE_BUS 3      //temperature sensor connection 

//########################################################################################################################
//Data Structure to be received 
//########################################################################################################################
typedef struct {
  	  int ct;		// current transformer
	  int supplyV;		// emontx voltage
	} Payload;
	Payload emontx;

int emontx_nodeID;    //node ID of emon tx, extracted from RF datapacket. Not transmitted as part of structure
//###############################################################

unsigned long last;
unsigned long lastTemp;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
double temp=0;

double cval;

void setup () {
    rf12_initialize(MYNODE, freq,group);
    
    last = millis();
    
    sensors.begin(); //start up temp sensor

    //Serial LCD    
    Serial.begin(9600);
    clearLCD();
    backlightOn();
  
 //get inital temperature reading
 sensors.requestTemperatures();
 temp=(sensors.getTempCByIndex(0));
}

void loop () {

   //--------------------------------------------------------------------
    // 1) Receive data from RFM12
    //--------------------------------------------------------------------
    if (rf12_recvDone() && rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0) {
        emontx=*(Payload*) rf12_data;   
       // emontx_nodeID=rf12_hdr & 0x1F;   //extract node ID from received packet 
        last = millis();
    }
    
   //get data from temp sensor every 10s
   if (millis()>lastTemp+10000){
   sensors.requestTemperatures();
   temp=(sensors.getTempCByIndex(0));
   lastTemp=millis();
   }


  selectLineOne();
  delay(100);
  Serial.print("OpenEnergyMonitor");

  selectLineTwo();
  delay(100);
  Serial.print("Power: ");

  char str[50];

  //print power value on LCD
  itoa((int)emontx.ct,str,10);
  strcat(str,"W   ");
  Serial.print(str);
   
  selectLineThree();
  delay(100);
  Serial.print("Room Temp: ");
  dtostrf(temp,0,1,str); 
  strcat(str,"C ");
  Serial.print(str);

    //last updated   
  selectLineFour();
  delay(100);
  Serial.print("Last update: ");
  int seconds = (int)((millis()-last)/1000.0);
  itoa(seconds,str,10);
  strcat(str,"s ago ");
  Serial.print(str);
   
}

   //SerialLCD Functions
   void selectLineOne(){  //puts the cursor at line 0 char 0.
      Serial.print(0xFE, BYTE);   //command flag
      Serial.print(128, BYTE);    //position
   }
   void selectLineTwo(){  //puts the cursor at line 2 char 0.
      Serial.print(0xFE, BYTE);   //command flag
      Serial.print(192, BYTE);    //position
   }
   void selectLineThree(){  //puts the cursor at line 3 char 0.
      Serial.print(0xFE, BYTE);   //command flag
      Serial.print(148, BYTE);    //position
   }
   void selectLineFour(){  //puts the cursor at line 4 char 0.
      Serial.print(0xFE, BYTE);   //command flag
      Serial.print(212, BYTE);    //position
   }
   void goTo(int position) { //position = line 1: 0-19, line 2: 20-39, etc, 79+ defaults back to 0
   if (position<20){ Serial.print(0xFE, BYTE);   //command flag
                 Serial.print((position+128), BYTE);    //position
   }else if (position<40){Serial.print(0xFE, BYTE);   //command flag
                 Serial.print((position+128+64-20), BYTE);    //position 
   }else if (position<60){Serial.print(0xFE, BYTE);   //command flag
                 Serial.print((position+128+20-40), BYTE);    //position
   }else if (position<80){Serial.print(0xFE, BYTE);   //command flag
                 Serial.print((position+128+84-60), BYTE);    //position              
   } else { goTo(0); }
   }
   void clearLCD(){
      Serial.print(0xFE, BYTE);   //command flag
      Serial.print(0x01, BYTE);   //clear command.
   }
   void backlightOn(){  //turns on the backlight
       Serial.print(0x7C, BYTE);   //command flag for backlight stuff
       Serial.print(157, BYTE);    //light level.
   }
   void backlightOff(){  //turns off the backlight
       Serial.print(0x7C, BYTE);   //command flag for backlight stuff
       Serial.print(128, BYTE);     //light level for off.
   }
   void backlight50(){  //sets the backlight at 50% brightness
       Serial.print(0x7C, BYTE);   //command flag for backlight stuff
       Serial.print(143, BYTE);     //light level for off.
   }
   void serCommand(){   //a general function to call the command flag for issuing all other commands   
     Serial.print(0xFE, BYTE);
   }
