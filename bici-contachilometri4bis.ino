//
// Speedometer with interupt
// smooth numbers
//

// arduino nano wiring
// ------------------------------------------------------------------
// 
//   5V --- BUT1 --- D7
//   5V --- BUT2 --- D5
//   
//   Reed Sensor D2, 5V, GND
//
//   Display
//    SCL  --- A4
//    SDA  --- A5
//    GND  --- GND
//    VCC  --- 5V
//    I2C  --- 5V
//    
// ------------------------------------------------------------------


#include <Wire.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// ------------------------------------------------
// function to print to display starting from startpos
void ledprint(char* str, Adafruit_AlphaNum4 *alphanum, int startpos) {
  int pos = 0;
  for (int index = 0; (index < strlen(str) && pos < 4); index++) {
    if ('.' == str[index+1]) {
      alphanum->writeDigitAscii(pos+ startpos, str[index], true);
      index++;                                 
    }  else {
      alphanum->writeDigitAscii(pos+startpos, str[index]); 
    }
    pos++;
  } 
  alphanum->writeDisplay();
}

const byte IS_PRESSED = 0;
const byte IS_NOT_PRESSED = 1;

int buttonPin1 = 7;
int buttonPin2 = 5;

int reedSensor = 2;  // reed sensor
int count = 0;       // counter of wheel loops in time deltat seconds
byte button1_status = IS_NOT_PRESSED; // button status

volatile int flag = 0;  // used in interrupt

float v1 = 0;        // current speed value
float v0 = 0;       // previous speed value
float m = 0;        // current meters

//
// I store 2 values to gradually move numbers from previous speed to current speed
// to have a better smooth display movement
//

float r = (0.711/2.0) ;       // wheel radius in m
float p = 2.0*r*3.1415;       // wheel circumference in m
int deltat = 2; //sec         // time window of refresh of the counter

unsigned long w = 0;          // timer for velocity display from previous speed to the current one
unsigned long t_mag = 0;      // timer to show the magnet passage
unsigned long t_but1 = 0;     // timer for button1
unsigned long timepass;       // timer for tempo

int function = 0;             // 0 =show km/h   1= show km     2= show giri    3 = show elapsed time

int spin = 0;

void setup() {
  Serial.begin(9600);
  pinMode (reedSensor, INPUT);
  pinMode (buttonPin1, INPUT_PULLUP);
  pinMode (buttonPin2, INPUT_PULLUP);

  Serial.println("Start");  
  alpha4.begin(0x70);  // pass in the address
  alpha4.clear();
  alpha4.writeDisplay();
  alpha4.clear();
  alpha4.writeDisplay();

  String s = "km/h";
  ledprint(s.c_str(),&alpha4,0);
  delay(500);
  alpha4.clear();
  alpha4.writeDisplay();

  // attach an interrupt to the change of the digital pin of the reed sensor
  // everytime the reed changes the interrupt function is called
  attachInterrupt(digitalPinToInterrupt(reedSensor), magnetPassage, CHANGE);
  timepass = millis();
}


// function called every time the reed sensor value changes (the magnet has passed)
void magnetPassage(){
  byte a=digitalRead(reedSensor);
  
  if(a==0 && flag==0) { flag++; Serial.print(a); }
  EIFR = 0x01;    // EIFR (External Interrupt Flag Register) reset interrupt?
}



float val0=0;
float val1=0;
float inc;
unsigned long sec;
bool asteriskmode;




void loop() {

  unsigned long t = millis() + deltat * 1000;
  
  String s0="";
  String s="";

  

  // every deltat seconds
  while(t>millis()) {

    // check buttons and perform actions
    // --------------------------------------------------------------
    // button
    byte b1 = digitalRead(buttonPin1);    // 0 pressed, 1 not pressed
    byte b2 = digitalRead(buttonPin2);    // 0 pressed, 1 not pressed
    
    if(b1 == IS_PRESSED && button1_status == IS_NOT_PRESSED) {
      //
      // button has been pressed
      button1_status = IS_PRESSED;
    }
    if(b1 == IS_NOT_PRESSED && button1_status == IS_PRESSED) {
      // button has been released 
      function++;
      if (function>3) function = 0;
      
      // display changed function
      String f = "";
      if (function == 0) f="km/h";
      if (function == 1) f="dist";
      if (function == 2) f="giri";
      if (function == 3) f="time";
      ledprint(f.c_str(),&alpha4,0);

      t_but1 = millis() + 1000; // display is showing something for a second

      button1_status = IS_NOT_PRESSED;
    }
    if(button1_status==IS_NOT_PRESSED && millis()>t_but1 && t_but1 > 0) {
      // terminate showing changed function
      alpha4.clear(); 
      t_but1 = 0; // now display can be used

    }
    // --------------------------------------------------------------







    //delay(50);


    // status:
    spin = spin;  // spins of the wheel from interrupt
    v1 = v1;      // speed calculated every deltat
    m = p * spin; // distance in meters
    sec = ( millis() - timepass ) / 1000; // seconds from beginning
    

    // functions

    if (function == 0) {    // km/h mode
      val1 = v1;
      inc = (v1-v0) / 10.0;  // gradually change from one speed to next in 5 steps
      val0 = val0 + inc;
      asteriskmode = true;
    }
    if (function==1 && button1_status==IS_NOT_PRESSED) {
      val1 = m;
      asteriskmode = false;
    }
    if (function==2 && button1_status==IS_NOT_PRESSED) {
      val1 = spin;
      asteriskmode = false;
    }
    if (function==3 && button1_status==IS_NOT_PRESSED) {
      val1 = sec;
      asteriskmode = false;
    }
   
 
 
    if (function==0) {
        // if enaugh time has passed print speed, show variation gradually
        if(millis() > w ) {
          w =millis() + 20; // next timer
          //v0 = v0 + inc; // add increment
          if( ( v1 > v0 && v0+inc < v1) || (v1 < v0 && v0 + inc > 0) ) v0 = v0+inc;
            
          s = (String)round(v0);
          if(v0 < 100) s = " "+s; // left pad     
          if(v0 < 10) s = " "+s;  // left pad     
          if(s0!=s) {
            //alpha4.clear();
            if(button1_status==IS_NOT_PRESSED && t_but1==0) {
              alpha4.writeDigitAscii(1,' '); alpha4.writeDigitAscii(2,' '); alpha4.writeDigitAscii(3,' ');
              ledprint(s.c_str(),&alpha4,1);        
            }
            s0=s;
          }        
        }
    }


    // m e km
    if (function==1 && button1_status==IS_NOT_PRESSED) {

        String u = "m";

        String sm ="";
        if(m > 999) {
          u =  "km";
          m = m/1000;
          if(m>9.9) sm = String(round(m)); else {
            sm = String(round(m * 10.0) / 10.0);
            sm.remove(sm.length()-1,1);
          }
        } else {
          sm = String(round(m));
          if(m < 100) sm = " "+sm; // left pad     
          if(m < 10) sm = " "+sm;  // left pad               
        }

        u = sm + u;
        
        if(  t_but1==0 ) ledprint(u.c_str(),&alpha4,0);        
        
    }



    if (function==2 && button1_status==IS_NOT_PRESSED  && t_but1 == 0) {
        ledprint(String(spin).c_str(),&alpha4,0);        
        
    }


    if (function==3 && button1_status==IS_NOT_PRESSED) {
        
        int hours = (sec / 60) / 60;
        int minutes = (sec / 60) % 60;
        int seconds = sec % 60;

        char ss[2];
        sprintf(ss,"%02d", seconds);
        char mm[2];
        sprintf(mm,"%02d", minutes);
        char oo[2];
        sprintf(oo,"%02d", hours);

        if( t_but1 == 0) {
          if (hours > 0) {
            // write hours and minutes
            ledprint(strcat(strcat(oo,"."),mm),&alpha4,0);        
          } else {
            // write minutes and seconds
            ledprint(strcat(strcat(mm,"."),ss),&alpha4,0);        
          }      
        }
    }

    if(flag>0) {
      // interrupt has been called, I have to update counters and show magnet passage
      if(function==0) {
          if(button1_status==IS_NOT_PRESSED && t_but1==0) {
            alpha4.writeDigitAscii(0,'*');alpha4.writeDisplay(); // show magnet passage
          }
          t_mag =millis() + 100; // next timer
      }
      flag=0;
      count++;    // counter for spins in deltat seconds
      spin++;     // total count of spins
    } 

    if(function==0 && button1_status==IS_NOT_PRESSED) {
      if (millis()>t_mag  && t_but1==0) {
        alpha4.writeDigitAscii(0,' ');alpha4.writeDisplay(); 
      } // clear show magnet passage
    }

     
   }

  
   // store previous speed
   v0=v1;
   // calculate new speed (km/h) (kph)
   v1 = round ( (count*p / deltat) * 3.6 * 10.0 ) / 10.0;
   Serial.println("\nv1:" + (String)v1 + " count:"+(String)count);
   count=0;

}
