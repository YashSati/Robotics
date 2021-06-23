/* Receiver code for the Arduino Radio control with PWM output
 * Install the NRF24 library to your IDE
 * Upload this code to the Arduino UNO, NANO, Pro mini (5V,16MHz)
 * Connect a NRF24 module to it:
 
    Module // Arduino UNO,NANO
    
    GND    ->   GND
    Vcc    ->   3.3V
    CE     ->   D9
    CSN    ->   D10
    CLK    ->   D13
    MOSI   ->   D11
    MISO   ->   D12

This code receive 7 channels and create a PWM output for each one on D2, D3, D4, D5, D6, D7and D8
Please, like share and subscribe : https://www.youtube.com/c/ELECTRONOOBS
*/


#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


const uint64_t pipeIn = 0xE8E8F0F0E1LL;     //Remember that this code is the same as in the transmitter
RF24 radio(9, 10);  //CSN and CE pins

// The sizeof this struct should not exceed 32 bytes
struct Received_data {
  byte ch1;
  byte ch2;
  byte ch3;
  byte ch4;
  byte ch5;
  byte ch6;
  byte ch7;
};

Received_data received_data;

int Xaxis = 0;
int Yaxis = 0;
int gyro = 0;
int LED = 0;
int DPAD = 0;
int FAST = 1;
int SPEED = 0;

void reset_the_Data() 
{
  // 'safe' values to use when NO radio input is detected
  received_data.ch1 = 0;      //Throttle (channel 1) to 0
  received_data.ch2 = 0;
  received_data.ch3 = 0;
  received_data.ch4 = 0;
  received_data.ch5 = 0;
  received_data.ch6 = 1;
  received_data.ch7 = 0;
}



/**************************************************/

void setup()
{
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  Serial.begin(9600);
  //We reset the received values
  reset_the_Data();

  //Once again, begin and radio configuration
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);  
  radio.openReadingPipe(1,pipeIn);
  
  //We start the radio comunication
  radio.startListening();

}

/**************************************************/

unsigned long lastRecvTime = 0;

//We create the function that will read the data each certain time
void receive_the_data()
{
  while ( radio.available() ) {
  radio.read(&received_data, sizeof(Received_data));
  lastRecvTime = millis(); //Here we receive the data
}
}

void motion()
  {
    if(DPAD==1 || gyro==1)
      {
        if(Xaxis<110 && Yaxis<180 && Yaxis>100) //UP
         {
          digitalWrite(2,HIGH);
          digitalWrite(5,LOW);
          digitalWrite(7,HIGH);
          digitalWrite(8,LOW);
          Serial.println("UP");
         }
        else if(Xaxis>200 && Yaxis<180 && Yaxis>100)  //DOWN
          {
           digitalWrite(5,HIGH);
           digitalWrite(2,LOW);
           digitalWrite(8,HIGH);
           digitalWrite(7,LOW);
           Serial.println("DOWN");
          }

        else if(Yaxis>180 && Xaxis<200 && Xaxis>120) //RIGHT
          {
           digitalWrite(5,HIGH);
           digitalWrite(2,LOW);  
           digitalWrite(7,HIGH);
           digitalWrite(8,LOW);
           Serial.println("RIGHT");
          }
        else if(Yaxis<110 && Xaxis<200 && Xaxis>120) //LEFT
          {
           digitalWrite(2,HIGH);
           digitalWrite(5,LOW);
           digitalWrite(8,HIGH);
           digitalWrite(7,LOW);
           Serial.println("LEFT");
          }  
       else
        {
           digitalWrite(2,LOW);
           digitalWrite(5,LOW);
           digitalWrite(8,LOW);
           digitalWrite(7,LOW);
           Serial.println("CENTER");   
        }
      }
  }

/**************************************************/

void loop()
{
  int Speed;
  
  //Receive the radio data
  receive_the_data();

//////////This small if will reset the data if signal is lost for 1 sec.
/////////////////////////////////////////////////////////////////////////
  /*unsigned long now = millis();
  if ( now - lastRecvTime > 1000 ) {
    // signal lost?
    reset_the_Data();
    //Go up and change the initial values if you want depending on
    //your aplications. Put 0 for throttle in case of drones so it won't
    //fly away
  } */

  
  Xaxis = received_data.ch1;   //X axis
  Yaxis = received_data.ch2;   //Y axis
  gyro  = received_data.ch3;
  LED   = received_data.ch4;
  DPAD  = received_data.ch5;
  FAST  = received_data.ch6;
  SPEED = received_data.ch7/10;

  //Speed=map(SPEED,0,10,0,255);
  Speed=255;
  if(SPEED<3)
   Speed=0;

  analogWrite(3,Speed);
  analogWrite(6,Speed+20);
  //Create the the PWM signals
  if(Speed!=0)
   motion();

  else
   {
    analogWrite(3,0);
    analogWrite(6,0);
    digitalWrite(2,LOW);
    digitalWrite(5,LOW);
    digitalWrite(8,LOW);
    digitalWrite(7,LOW);
    Serial.println("CENTER");
   }

  Serial.println(Speed); 
}//Loop end
