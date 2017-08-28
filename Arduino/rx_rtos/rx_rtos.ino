#include <Arduino_FreeRTOS.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 4);

const byte Addr[6] = "00001";

// define two tasks for Blink & AnalogRead
void Receive( void *pvParameters );
void Transmit( void *pvParameters );

TaskHandle_t rx;

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  radio.begin();
  radio.setRetries(15, 15);


  xTaskCreate(
    Transmit
    ,  (const portCHAR *) "Transmitter"
    ,  256  // Stack size
    ,  NULL
    ,  2  // Priority
    ,  NULL );
  
  // Now set up two tasks to run independently.
  xTaskCreate(
    Receive
    ,  (const portCHAR *)"Receiver"   // A name just for humans
    ,  256  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &rx );


  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void Receive(void *pvParameters)  // This is a task.
{
  byte data_rx=0;
  for (;;) // A Task shall never return or exit.
  {
    //Serial.println("R");
    // Open a pipe for reading
    radio.openReadingPipe(0, Addr);
    radio.startListening();
    if (radio.available())
     {
       radio.read(&data_rx, sizeof(byte));
       Serial.print("RX from rx:  ");
       Serial.println(data_rx);
     }
     delay(15);
     vTaskPrioritySet( rx, 1 );
     taskYIELD();
    }
}


void Transmit(void *pvParameters)  // This is a task.
{
  byte data_tx=255;
  for (;;)
  { 
    //Serial.println("T");
   
    // Open a pipe for writing
    radio.openWritingPipe(Addr);
    radio.stopListening();

    data_tx=(data_tx-1)%255;
    Serial.print("TX to rx:  ");
    Serial.println(data_tx);
    
    radio.write(&data_tx, sizeof(byte));
    delay(15);
    vTaskPrioritySet( rx, 3 );
    taskYIELD();
    
  }
}


