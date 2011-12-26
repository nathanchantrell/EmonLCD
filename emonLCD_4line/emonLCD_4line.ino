//--------------------------------------------------------------------------------------------------------
// LCD Energy Monitor Display using a 4 line LCD display connected with Sparkfun SerLCD Serial Backpack
// By Nathan Chantrell. http://zorg.org/
// Builds on emonGLCD example by Trystan Lea and Glyn Hudson at OpenEnergyMonitor.org
//
// Features:
//  Receive power reading from emonTX via RF
//  Receive remote temperature reading via RF
//  Read room temperature from DS18B20 sensor
//--------------------------------------------------------------------------------------------------------

#include <OneWire.h> // http://www.pjrc.com/teensy/arduino_libraries/OneWire.zip
#include <DallasTemperature.h> // http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_371Beta.zip
#include <JeeLib.h> // https://github.com/jcw/jeelib

// Using SoftwareSerial pin 3 = rx, pin 4 = tx
#include <SoftwareSerial.h>
SoftwareSerial mySerial(3, 4);

// fixed RF12 settings
#define MYNODE 28            //node ID, 30 is reserved for base station
#define freq RF12_433MHZ     //frequency
#define group 210            //network group 

// DS18B20 Temperature sensor on pin D5
#define ONE_WIRE_BUS 5     

//########################################################################################################################
//Data Structure to be received 
//########################################################################################################################
typedef struct {
  	  int ct;		// current transformer
	  int supplyV;		// emontx voltage
	} Payload;
	Payload emontx;

int emontx_nodeID;    //node ID of emon tx, extracted from RF datapacket. Not transmitted as part of structure
//########################################################################################################################

//variables
unsigned long last; // last update from rf
unsigned long lastTemp; // last local temp reading

double temp=0; // local temp
double cval; // ct reading from emonTX

char str[50]; // general char array

//DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup () {

 rf12_initialize(MYNODE, freq,group); // Initialise the RFM12B
    
 last = millis();
    
 sensors.begin(); //start up temp sensor

 //Serial LCD    
 mySerial.begin(9600);
 clearLCD();
 backlightOn();
  
 //get inital temperature reading
 sensors.requestTemperatures();
 temp=(sensors.getTempCByIndex(0));
}

 void loop () {

//########################################################################################################################
//Receive data from RFM12
//########################################################################################################################
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

//########################################################################################################################
//Display data
//########################################################################################################################

  selectLineOne();
  delay(100);
  mySerial.write("OpenEnergyMonitor");

 // Power value
  selectLineTwo();
  delay(100);
  mySerial.write("Power: ");
  itoa((int)emontx.ct,str,10);
  strcat(str,"W   ");
  mySerial.write(str);

 // Room temperature 
  selectLineThree();
  delay(100);
  mySerial.write("Room Temp: ");
  dtostrf(temp,0,1,str); 
  strcat(str,"C ");
  mySerial.write(str);

 // Last updated   
  selectLineFour();
  delay(100);
  mySerial.write("Last update: ");
  int seconds = (int)((millis()-last)/1000.0);
  itoa(seconds,str,10);
  strcat(str,"s ago ");
  mySerial.write(str);
   
}

//########################################################################################################################
//SerialLCD Functions
//########################################################################################################################

   void selectLineOne(){  //puts the cursor at line 0 char 0.
      mySerial.write(0xFE);   //command flag
      mySerial.write(128);    //position
   }
   void selectLineTwo(){  //puts the cursor at line 2 char 0.
      mySerial.write(0xFE);   //command flag
      mySerial.write(192);    //position
   }
   void selectLineThree(){  //puts the cursor at line 3 char 0.
      mySerial.write(0xFE);   //command flag
      mySerial.write(148);    //position
   }
   void selectLineFour(){  //puts the cursor at line 4 char 0.
      mySerial.write(0xFE);   //command flag
      mySerial.write(212);    //position
   }
   void goTo(int position) { //position = line 1: 0-19, line 2: 20-39, etc, 79+ defaults back to 0
   if (position<20){ mySerial.write(0xFE);   //command flag
                 mySerial.write(position+128);    //position
   }else if (position<40){mySerial.write(0xFE);   //command flag
                 mySerial.write(position+128+64-20);    //position 
   }else if (position<60){mySerial.write(0xFE);   //command flag
                 mySerial.write(position+128+20-40);    //position
   }else if (position<80){mySerial.write(0xFE);   //command flag
                 mySerial.write(position+128+84-60);    //position              
   } else { goTo(0); }
   }
   void clearLCD(){
      mySerial.write(0xFE);   //command flag
      mySerial.write(0x01);   //clear command.
   }
   void backlightOn(){  //turns on the backlight
       mySerial.write(0x7C);   //command flag for backlight stuff
       mySerial.write(157);    //light level.
   }
   void backlightOff(){  //turns off the backlight
       mySerial.write(0x7C);   //command flag for backlight stuff
       mySerial.write(128);     //light level for off.
   }
   void backlight50(){  //sets the backlight at 50% brightness
       mySerial.write(0x7C);   //command flag for backlight stuff
       mySerial.write(143);     //light level for off.
   }
   void serCommand(){   //a general function to call the command flag for issuing all other commands   
     mySerial.write(0xFE);
   }
