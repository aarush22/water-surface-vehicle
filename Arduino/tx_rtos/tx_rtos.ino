#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 4);

const byte Addr[6] = "00001";

// define two tasks for Blink & AnalogRead
void Joystick_read( void *pvParameters );
void Transmission( void *pvParameters );

QueueHandle_t dataQueue;


// the setup function runs once when you press reset or power the board
void setup() {

  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  digitalWrite(5,HIGH);
  digitalWrite(6,HIGH);
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  radio.begin();
  radio.setRetries(15, 15);

  dataQueue = xQueueCreate( 5, 4 );

  xTaskCreate(
    Transmission
    ,  (const portCHAR *) "Transmit and Receive"
    ,  256  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );

    xTaskCreate(
    Joystick_read
    ,  (const portCHAR *)"Read Joystick"   // A name just for humans
    ,  256  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
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
  byte data_tx[4]={0,0,0,0};
  byte data_rx=0;
  for (;;) // A Task shall never return or exit.
  {
    //Serial.print("R");
    // Open a pipe for reading
    /*radio.openReadingPipe(0, Addr);
    radio.startListening();
    if (radio.available())
     {
       radio.read(&data_rx, sizeof(byte));
       Serial.print("RX from rx:  ");
       Serial.println(data_rx);
     }
    delay(15);*/
    
    radio.openWritingPipe(Addr);
    radio.stopListening();
    if(xQueueReceive( dataQueue, data_tx, portMAX_DELAY )){
      radio.write(data_tx, 4);
      Serial.print((data_tx[0]*255)+data_tx[1]);
      Serial.print("  :  ");
      Serial.println((data_tx[2]*255)+data_tx[3]);
    }
    delay(15);
    }
}

void Joystick_read(void *pvParameters)  // This is a task.
{
  byte data[4] = {0,0,0,0};
  int Left_ctr =0;
  int Right_ctr =0;
  
  for (;;)
  { 
    Left_ctr = analogRead(A0);
    Right_ctr = analogRead(A1);
    data[0] =Left_ctr/255;
    data[1] =Left_ctr-(data[0]*255);
    data[2] =Right_ctr/255;
    data[3] =Right_ctr-(data[2]*255);
    xQueueSend(dataQueue,data,portMAX_DELAY);
  }
}
