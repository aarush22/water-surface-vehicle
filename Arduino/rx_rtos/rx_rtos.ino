#include <Arduino_FreeRTOS.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(7, 4);

Servo esc_l;
Servo esc_r;

int escPin_l = 9;
int escPin_r = 10;
int minPulseRate = 1000;
int maxPulseRate = 2000;

const byte Addr[6] = "00001";

// define a transmission task
void Transmission( void *pvParameters );


// the setup function runs once when you press reset or power the board
void setup() {
  
  // Attach the the servo to the correct pin and set the pulse range
  esc_l.attach(escPin_l, minPulseRate, maxPulseRate); 
  esc_r.attach(escPin_r, minPulseRate, maxPulseRate); 
  // Write a minimum value (most ESCs require this correct startup)
  esc_l.write(0);
  esc_r.write(0);
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  radio.begin();
  radio.setRetries(15, 15);

  xTaskCreate(
    Transmission
    ,  (const portCHAR *) "Transmitter and Receive"
    ,  256  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );


  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/



void Transmission(void *pvParameters)  // This is a task.
{
  int throttle_l=0;
  int throttle_r=0;
  unsigned short left_fan=0;
  unsigned short right_fan=0;
  byte data_rx[4]={0,0,0,0};
  byte data_tx=111;
  
  for (;;) // A Task shall never return or exit.
  {
    // Open a pipe for reading
    radio.openReadingPipe(0, Addr);
    radio.startListening();
   
    if (radio.available()){
      radio.read(data_rx, 4);
      left_fan=(data_rx[0]*255)+data_rx[1];
      right_fan=(data_rx[2]*255)+data_rx[3];
      throttle_l=map(left_fan,0,1023,1000,2000);
      throttle_r=map(right_fan,0,1023,1000,2000);
      Serial.print(throttle_l);
      Serial.print(" : ");
      Serial.println(throttle_l);
      esc_l.writeMicroseconds(throttle_l);
      esc_r.writeMicroseconds(throttle_r);
    } 
    delay(15);
    
    //radio.openWritingPipe(Addr);
    //radio.stopListening();
    //if(xQueueReceive( dataQueue, data_tx, portMAX_DELAY )){
   // radio.write(data_tx, 4);
   // }
    //delay(15);
    }
}
