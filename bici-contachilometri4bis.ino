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


int buttonPin1 = 7;
int buttonPin2 = 5;

int reedSensor = 2;  // reed sensor
int count = 0;       // counter of wheel loops
byte bv = 2;         // prev button value

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

unsigned long w = 0;          // timer for velocity from previous speed to the current one
unsigned long wa = 0;         // timer for visualizzare passaggio magnete
unsigned long wp = 0;         // timer for button1
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
  Serial.println("attachInterrupt");  
  timepass = millis();
}


// function called every time the reed sensor value changes (the magnet has passed)
void magnetPassage(){
  Serial.println("OK");
  byte a=digitalRead(reedSensor);
  if(a==0) flag++;
  EIFR = 0x01;    // EIFR (External Interrupt Flag Register) reset interrupt?
}



/*

 aggiungere 1 bottone funzione
 che cambia il dato mostrato nel display
 1 prima pressione : switch a tempo passato hh.mm
 2 seconda pressione: switch a km percorsi
 0 terza pressione: switch a velocità
 pressione lunga: azzera contatore tempo e km
 
 * 
 */

float val0=0;
float val1=0;
float inc;
unsigned long sec;
bool asteriskmode;

void loop() {

  unsigned long t = millis() + deltat * 1000;
  
  String s0="";
  String s="";



  // ogni deltat secondi
  while(t>millis()) {

    // controllo la pressione del bottone
    // e cambio la modalità
    // (aggiungere modalità velocità media e velocità massima)

    // --------------------------------------------------------------
    // button
    int b = digitalRead(buttonPin1);
    b = b==1 ? 0 : 1 ;
    //Serial.println("b:"+(String)b+" bv:"+String(bv));
    if(b==1 && bv ==2) {
      //
      // button has been pressed
      bv = 1;
    }
    if(b==0 && bv==1) {
      // button has been released 
      function++;
      if (function>3) function = 0;
      Serial.println("Function is " + (String)function);

      // display changed function
      String f = "";
      if (function == 0) f="km/h";
      if (function == 1) f="dist";
      if (function == 2) f="giri";
      if (function == 3) f="time";
      ledprint(f.c_str(),&alpha4,0);

      wp = millis() + 1000;

      bv = 2;
    }
    if(bv==2 && millis()>wp && wp > 0) {
      // terminate showing changed function
      alpha4.clear(); 
      //alpha4.writeDisplay();
      Serial.println("cleared");

      wp = 0;
      //bv = 2; // ready to work
    }
    // --------------------------------------------------------------

    delay(50);



    v1 = v1;      // velocità istantanea calcolata ogni deltat secondi
    m = p * spin; // distanza in m
    spin = spin;  // giri da interrupt
    sec = ( millis() - timepass ) / 1000; // seconds from beginning
    

    if (function == 0) {
      val1 = v1;
      inc = (v1-v0) / 5.0;
      val0 = val0 + inc;
      asteriskmode = true;
    }
    if (function==1 && bv==2) {
      val1 = m;
      asteriskmode = false;
    }
    if (function==2 && bv==2) {
      val1 = spin;
      asteriskmode = false;
    }
    if (function==3 && bv==2) {
      val1 = sec;
      asteriskmode = false;
    }
   
 


    if(false && millis() > w ) {
                                
      w =millis() + 200; // next time

      // stampa il nuovo val1
    
    }

 
    if (function==0) {
Serial.println("xxx "+(String)w);
            // if enaugh time has passed print speed, show variation gradually
            if(millis() > w ) {
                            
              w =millis() + 200; // next timer
        
              v0 = v0 + inc; // add increment
        
              s = (String)round(v0);
              Serial.println(s);
              if(v0 < 100) s = " "+s; // left pad     
              if(v0 < 10) s = " "+s;  // left pad     

        
              if(s0!=s) {
                //alpha4.clear();
                if(bv==2 && wp==0) {
                  alpha4.writeDigitAscii(1,' '); alpha4.writeDigitAscii(2,' '); alpha4.writeDigitAscii(3,' ');
                  ledprint(s.c_str(),&alpha4,1);        
                }
                s0=s;
              }
        
            }


     



    }


    // m e km
    if (function==1 && bv==2) {

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
        
        if(  wp==0 ) ledprint(u.c_str(),&alpha4,0);        
        
    }



    if (function==2 && bv==2  && wp==0) {
        ledprint(String(spin).c_str(),&alpha4,0);        
        
    }


    if (function==3 && bv==2) {
        //int minu = s / 60;   // 2
        //int seco = s - 60*minu;   //1
        
        int ore = (sec / 60) / 60;   // 0
        int minu = (sec / 60) % 60;
        int seco = sec % 60;

        char ss[2];
        sprintf(ss,"%02d", seco);
        char mm[2];
        sprintf(mm,"%02d", minu);
        char oo[2];
        sprintf(oo,"%02d", ore);

        if( wp==0) {
          if (ore > 0) {
            ledprint(strcat(strcat(oo,"."),mm),&alpha4,0);        
          } else {
            ledprint(strcat(strcat(mm,"."),ss),&alpha4,0);        
          }      
        }
    }
    



    

    if(flag>0) {
            if(function==0) {
                if(bv==2 && wp==0) {
                  alpha4.writeDigitAscii(0,'*');alpha4.writeDisplay(); // show magnet passage
                  Serial.println("*");
                }
                wa =millis() + 100; // next timer
            }
      flag=0;
      count++;
      spin++;
    }

    if(function==0 && bv==2) {
      if (millis()>wa  && wp==0) {
        alpha4.writeDigitAscii(0,' ');alpha4.writeDisplay(); 
      } // clear show magnet passage
    }

     
   }

  
   // store previous speed
   v0=v1;
   // calculate new speed (km/h) (kph)
   v1 = round ( (count*p / deltat) * 3.6 * 10.0 ) / 10.0;
    
   count=0;

}
