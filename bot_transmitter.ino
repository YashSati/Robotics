#include "I2Cdev.h"
#include "MPU6050.h"
#include <MCUFRIEND_kbv.h>
#include <SPI.h> 
#include <nRF24L01.h>
#include <RF24.h>

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif


MCUFRIEND_kbv tft;       // hard-wired for UNO shields anyway.
#include <TouchScreen.h>

bool gyro=0,btn2=0,btn3=0,btn4=1,btn5=0,tch=0; //0=off
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

const uint64_t my_radio_pipe = 0xE8E8F0F0E1LL;
RF24 radio(15, 14);  //Set CE and CSN pins

// The sizeof this struct should not exceed 32 bytes
struct Data_to_be_sent {
  byte ch1;
  byte ch2;
  byte ch3;
  byte ch4;
  byte ch5;
  byte ch6;
  byte ch7;
};

//Create a variable with the structure above and name it sent_data
Data_to_be_sent sent_data;


int c=50,Xaxis,Yaxis;
long a7=0;
float a8=0;

const int XP=8,XM=A2,YP=A3,YM=9; //320x480 ID=0x9486
const int TS_LEFT=118,TS_RT=928,TS_TOP=957,TS_BOT=89;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
TSPoint tp;

#define MINPRESSURE 1
#define MAXPRESSURE 1000

uint16_t ID;
   //PORTRAIT=0

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

void setup()
{ 
  
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
  
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    
    tft.reset();
    ID = tft.readID();
    tft.begin(ID);
    tft.setRotation(3);
    tft.fillScreen(BLACK);

    tft.drawRect(0,0,480,320,WHITE);
    for(int i=1;i<4;i++) 
     tft.drawFastHLine(0,80*i,480,WHITE);

    for(int i=1;i<6;i++) 
     tft.drawFastVLine(80*i,80,240,WHITE); 

     tft.drawFastHLine(320,40,160,WHITE);
     tft.drawFastVLine(320,0,80,WHITE);
     tft.drawFastVLine(160,0,80,WHITE);
     
     tft.drawFastHLine(321,160,158,BLACK);
     tft.drawFastVLine(400,81,159,BLACK);
     tft.drawFastVLine(80,161,79,BLACK);
     tft.drawFastVLine(160,161,79,BLACK);


     tft.setCursor(330,10);
     tft.setTextColor(GREEN);
     tft.setTextSize(3);
     tft.print("My:");
     tft.setCursor(330,50);
     tft.print("Bot:");
     tft.setCursor(10,10);
     tft.print("GYRO:");
     tft.setCursor(170,10);
     tft.print("SPEED");
     tft.setCursor(30,190);
     tft.print("SPEED");

     tft.setCursor(100,10);
     tft.setTextColor(CYAN);
     tft.print("OFF");
     tft.setCursor(165,190);
     tft.print("SLOW");

     tft.setTextColor(YELLOW);
     tft.setCursor(5,110);
     tft.print("GYRO");
     tft.setCursor(95,110);
     tft.print("LED");
     tft.setCursor(165,110);
     tft.print("DPAD");

     tft.setTextColor(RED);
     tft.setCursor(245,110);
     tft.print("LOCK");

     radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(my_radio_pipe);

  //Reset each channel value
  sent_data.ch1 = 127;
  sent_data.ch2 = 127;
  sent_data.ch3 = 127;
  sent_data.ch4 = 127;
  sent_data.ch5 = 0;
  sent_data.ch6 = 0;
  sent_data.ch7 = 0;

  while(btn4)
    {
      uint16_t xpos, ypos;  //screen coordinates
      tp = ts.getPoint();
      xpos = map(tp.y, TS_BOT, TS_TOP, 0, tft.width());
      ypos = map(tp.x, TS_LEFT, TS_RT, 0, tft.height());

      xpos=xpos/80;
      ypos=ypos/80;

      pinMode(XM, OUTPUT);
      pinMode(YP, OUTPUT);
      
      if(xpos==3 && ypos==1)
       button4();
    }
    

  
}

void gyros()
  {
    int16_t ax, ay, az;
   
    accelgyro.initialize();
    
    accelgyro.getAcceleration(&ax, &ay, &az);
    Xaxis=(ax/1600)*12-24, Yaxis=(-ay/1600)*12;
   
    tft.drawCircle(Yaxis/2+400,Xaxis/1.7+160,10,WHITE);
    delay(50);
    tft.drawCircle(Yaxis/2+400,Xaxis/1.7+160,10,BLACK);

    Yaxis=Yaxis/2+400;
    Xaxis=Xaxis/1.7+160;
  }

void Dpad()
  {
    Xaxis=analogRead(A9)-494, Yaxis=analogRead(A10)-518;
    
    tft.drawCircle(Yaxis/8+400,-Xaxis/8+160,10,WHITE);
    delay(50);
    tft.drawCircle(Yaxis/8+400,-Xaxis/8+160,10,BLACK);
    Yaxis=Yaxis/8+400;
    Xaxis=-Xaxis/8+160;
  }  

void button1()
  {
     if(gyro==0)
      {
        tft.setTextColor(BLACK);
        tft.setCursor(100,10);
        tft.print("OFF");

        tft.setTextColor(RED);
        tft.setCursor(100,10);
        tft.print("ON");

        btn3=0;
        tft.setTextColor(YELLOW);
        tft.setCursor(165,110);
        tft.print("DPAD");
      }
     else
      {
        tft.setTextColor(BLACK);
        tft.setCursor(100,10);
        tft.print("ON");

        tft.setTextColor(CYAN);
        tft.setCursor(100,10);
        tft.print("OFF"); 
      }
     gyro=!gyro;
     tch=1;
  }
  
void button2()
  {
    if(btn2==0)
     tft.setTextColor(RED);
    else
     tft.setTextColor(YELLOW);
      
    tft.setCursor(95,110);
    tft.print("LED");
    btn2=!btn2;
    tch=1; 
  } 

void button3()
  {
    if(btn3==0)
     {
      gyro=0;
      tft.setTextColor(BLACK);
      tft.setCursor(100,10);
      tft.print("ON");

      tft.setTextColor(CYAN);
      tft.setCursor(100,10);
      tft.print("OFF");

      tft.setTextColor(RED);  
     }
    else
     tft.setTextColor(YELLOW);
      
    tft.setCursor(165,110);
    tft.print("DPAD");
    btn3=!btn3;
    tch=1;   
  }

void button4()
  {
    if(btn4==0)
     tft.setTextColor(RED);
    else
     tft.setTextColor(YELLOW);
      
    tft.setCursor(245,110);
    tft.print("LOCK");
    btn4=!btn4;
    tch=1; 
  }   

void button5()
  {
    tft.setCursor(165,190);
    if(btn5==0)
     {
       tft.setTextColor(BLACK);
       tft.print("SLOW");
       tft.setCursor(165,190);
       tft.setTextColor(RED);
       tft.print("FAST");
     }
    else
     {
       tft.setTextColor(BLACK);
       tft.print("FAST");
       tft.setCursor(165,190);
       tft.setTextColor(CYAN);
       tft.print("SLOW");
     }
    btn5=!btn5;
    tch=1; 
  }   

void button(int x,int y)
  {
    if(x==0 && y==1)
      button1();  

    else if(x==1 && y==1)
      button2();

    else if((x==2 && y==1) || analogRead(A6)==0)
      button3();

    else if(x>=0 && x<3 && y==2)
      button5(); 
  }

void touch()
  {
    uint16_t xpos, ypos;  //screen coordinates
    tp = ts.getPoint();
    xpos = map(tp.y, TS_BOT, TS_TOP, 0, tft.width());
    ypos = map(tp.x, TS_LEFT, TS_RT, 0, tft.height());

    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    button(xpos/80,ypos/80);
  }

void nrf()
  {
    if(gyro==1)
     gyros();
    else if(btn3==1)
     Dpad();
      
    sent_data.ch1 = Xaxis;
    sent_data.ch2 = Yaxis;
    sent_data.ch3 = gyro;
    sent_data.ch4 = btn2;
    sent_data.ch5 = btn3;
    sent_data.ch6 = btn5;
    sent_data.ch7 = a7;

    radio.write(&sent_data, sizeof(Data_to_be_sent));
  }

void data()
  {
      tft.setTextColor(BLACK);
      tft.setCursor(265,10);
      tft.print(a7);
      a7=map(analogRead(A7),0,1023,0,100);
      tft.setTextColor(CYAN);
      tft.setCursor(265,10);
      tft.print(a7);

      tft.setTextColor(BLACK);
      tft.setCursor(385,10);
      tft.print(a8);
      a8=(analogRead(A8)*15)/1024;
      tft.setTextColor(CYAN);
      tft.setCursor(385,10);
      tft.print(a8);  
  }
  
void loop()
{ 
  touch();
  nrf();

  if(tch==1)
    {
      delay(200);
      tch=0;
    }
    
  if(c==50)
    {
      data();
      c=0;
    }

  delay(10);  
  c++;
}
