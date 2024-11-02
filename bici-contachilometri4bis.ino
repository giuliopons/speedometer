//
// Speedometer with interupt
// and smooth number variation
//

// arduino nano wiring
// ------------------------------------------------------------------
// 
//   5V --- BUT1 --- D7
//   5V --- BUT2 --- D5
//   
//   Reed Sensor: D2, 5V, GND
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
unsigned int count = 0;       // counter of wheel loops in time deltat seconds
byte button1_status = IS_NOT_PRESSED; // button status
byte button2_status = IS_NOT_PRESSED; // button status

volatile int flag = 0;  // used in interrupt

float v1 = 0;       // current speed value
float v0 = 0;       // previous speed value
float m1 = 0;       // current meters
float m0 = 0;       // previous meters

//
// Store 2 values (m0 and m1, v0 and v1) to gradually move numbers from previous speed to current speed
// to have a better smooth display movement
//

float r = (0.711/2.0) ;       // wheel radius in m
float p = 2.0*r*3.1415;       // wheel circumference in m
int deltat = 2; //sec         // time window of refresh of the counter

unsigned long w = 0;          // timer for velocity display from previous speed to the current one
unsigned long t_mag = 0;      // timer to show the magnet passage
unsigned long t_dis = 0;     // timer for button1, when > 0 display is in use to show message/label
unsigned long timepass;       // timer for tempo

byte function = 0;             // 0 =show km/h   1= show km     2= show giri    3 = show elapsed time
byte mode = 0;                 // 0/1/2...  show variation of each function


unsigned int spin = 0;         // max 65.535 spins => max 146km 
float inc;
unsigned long sec;

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

  showFunction();

  // attach an interrupt to the change of the digital pin of the reed sensor
  // everytime the reed changes the interrupt function is called
  attachInterrupt(digitalPinToInterrupt(reedSensor), magnetPassage, CHANGE);
  timepass = millis();
}


// function called every time the reed sensor value changes (the magnet has passed)
void magnetPassage(){
  byte a=digitalRead(reedSensor);
  if(a==0 && flag==0) {
    flag++;
  }
  EIFR = 0x01;    // EIFR (External Interrupt Flag Register) reset interrupt?
}





// display the function and set the timer to reset display
// function displayed is determined by function and mode
void showFunction() {
  String f = "";
  if (function == 0) { if(mode==0) f=" KPH"; if (mode==1) f=" MPS"; if (mode==2) f="mean";}
  if (function == 1) { if(mode==0) f="dist"; else f= m1 > 9999 ? "km  " : "metr"; }
  if (function == 2) { if(mode==0) f="giri"; else f=" RPM";}
  if (function == 3) f="time";
  ledprint(f.c_str(),&alpha4,0);
  t_dis = millis() + 1000; // display is in use, showing function for a second
}

float slideNumber( float from, float to, float inc, int pos, String u ) {

  if(millis() > w ) {
   w =millis() + 20; // next timer
   if( ( to > from && from+inc < to) || (to < from && from + inc > 0) ) from = from+inc;
   
  if(t_dis==0) {
    String s = (String)int(from);

    s = s + u;

    if(from < 1000 && pos == 0 && u == "") s = " "+s; // left pad
    if(from < 100) s = " "+s; // left pad     
    if(from < 10) s = " "+s;  // left pad     
  
    ledprint(s.c_str(),&alpha4, pos );        
  }
 }

 return from;
  
}





void loop() {

  unsigned long t = millis() + deltat * 1000;

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
      //
      button1_status = IS_PRESSED;
    }
    if(b1 == IS_NOT_PRESSED && button1_status == IS_PRESSED) {
      //
      // button has been released 
      // change function
      //
      function++;
      mode =0;
      if (function>3) function = 0;
      
      // display changed function
      showFunction();

      button1_status = IS_NOT_PRESSED;
    }
    if(millis()>t_dis && t_dis > 0) {
      //
      // terminate showing changed function
      //
      alpha4.clear(); 
      t_dis = 0; // now display can be used

    }
    // --------------------------------------------------------------

  
    if(b2 == IS_PRESSED && button2_status == IS_NOT_PRESSED) {
     button2_status = IS_PRESSED;
    }
    if(b2 == IS_NOT_PRESSED && button2_status == IS_PRESSED) {
      mode++;
      if(function ==0 && mode==3) mode=0;
      if(function ==1 && mode==2) mode=0;
      if(function ==2 && mode==2) mode=0;
      if(function ==3) mode=0;
      
     button2_status = IS_NOT_PRESSED;
      // display changed function
      showFunction();     
    }


 
    if (function==0) {
        // if enaugh time has passed print speed, show variation gradually
        inc = (v1-v0) > 0 ? .2 : -.2;
        v0 = slideNumber( v0, v1, inc, 1, "" );
        
    }


    // m e km
    if (function==1) {
        String u = "m";
        String sm ="";
        float d  = m1;

        if(mode ==0) {

          if(d > 999) {
            u =  "km";
            d = d/1000;
            if(d>9.9) sm = String(int(d)); else {
              sm = String(round(d * 10.0) / 10.0);
              sm.remove(sm.length()-1,1);
              sm = sm + u;
              if(  t_dis==0) ledprint(sm.c_str(),&alpha4,0); 
            }
          } else {
            m0 = slideNumber(m0,m1,.1,0, u);       
          }
           
          
        } else {
          if(d > 9999) {
              d = d/1000;
              sm = String(int(d * 10.0) / 10.0);
              if(d < 100) sm = " "+sm; // left pad  
              sm.remove(sm.length()-1,1);
              sm = sm + u;
              if(  t_dis==0) ledprint(sm.c_str(),&alpha4,0); 
          } else {
            m0 = slideNumber(m0,m1,.1,0, "");
          }
        }
        
    }



    if (function==2) {
        float g = spin;

        if (mode == 0) {
          String sm = String(int(g));
          if (g>9999) {
            g = g/1000;
            sm = String(g);
            sm.remove(sm.length()-1,1);
            sm = sm + "k";
          }
          if( t_dis == 0 ) {
            ledprint(sm.c_str(),&alpha4,0);
          }
        } else {
          // rpm
          String sm = String(int( 60.0 * spin / sec ));
          if( t_dis == 0 ) {
            ledprint(sm.c_str(),&alpha4,0);
          }
          
        }
    }


    if (function==3) {
        
        int hours = (sec / 60) / 60;
        int minutes = (sec / 60) % 60;
        int seconds = sec % 60;

        char ss[2];
        sprintf(ss,"%02d", seconds);
        char mm[2];
        sprintf(mm,"%02d", minutes);
        char oo[2];
        sprintf(oo,"%02d", hours);

        if( t_dis == 0) {
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
      //
      // interrupt has been called, flag is > 0, I have to update counters and show magnet passage
      //
      if(function==0) {
          if(t_dis==0) {
            alpha4.writeDigitAscii(0,'*');alpha4.writeDisplay(); // show magnet passage
          }
          t_mag =millis() + 100; // next timer
      }
      flag=0;
      count++;    // counter for spins in deltat seconds
      spin++;     // total count of spins

      // status:
      spin = spin;  // spins of the wheel from interrupt
      v1 = v1;      // speed calculated every deltat
      m0=m1;         // store previous
      m1 = p * spin; // distance in meters
      
  
      //Serial.println("giri:" + (String)spin + " v1:" + String(v1) + " m:" + String(m1) + " sec:"+String(sec));

    } 

    if(function==0 ) {
      if (millis()>t_mag  && t_dis==0) {
        alpha4.writeDigitAscii(0,' ');alpha4.writeDisplay(); 
      } // clear show magnet passage
    }

     
   } //deltat

  

   // count seconds
   sec = ( millis() - timepass ) / 1000; // seconds from beginning

   if( function == 0) {
     // store previous speed
     v0=v1;

     // calculate new speed (km/h) (kph)
     if( mode == 0) {
        v1 = round ( (count*p / deltat) * 3.6 * 10.0 ) / 10.0; 
     }
     if(mode==1)  // mps
     {
        v1 = round ( (count*p / deltat) * 10.0 ) / 10.0; 
     }
     if(mode==2) {   //kph  mean value
        v1 = round ( (spin*p / sec) * 3.6 * 10.0 ) / 10.0;
     }

   }
  
   count=0; // reset counter for spins in deltat seconds

}
