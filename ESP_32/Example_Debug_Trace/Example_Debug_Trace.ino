/*********
  Example or sample code showing how to use interfaces or used at the time of development for testing.
*********/

#include <Arduino.h>
#include "Debug_Trace.h"
// Load Wi-Fi library
#include <WiFi.h>

#include "Html_Templates.h"



/* Golbal array for Buffer stream */
char Test_Buffer_Stream[Max_BackGround_Buffer_Reserved + 400];

/* Function decleratations.*/
void Process_Webserver(void);

void WebServer_Processing_Task(void *pvParameters);

/* To connect to server...*/
/* Handle for BackGround_Debug_Trace_Task */
TaskHandle_t WebServer_Processing_Task_Handle;

/* change these values to match your network*/
const char* ssid = "UPdate Yours";
const char* password = "UPdate Yours";

/*Define timeout time in milliseconds (example: 2000ms = 2s)*/
#define html_TimeOut_Clint 2000

/*Set web server port number to 80 */
WiFiServer server(80);

void setup()
{

   /* Init Trace support  */
   Init_Trace();

   Serial.println("Init_Trace completed...");

   /* Init Web server.*/
   /* Initate and waite unti                                                                                                                                                                                                                                                                                                                                                                  l get connected.*/
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED)
   {
      delay(500);
   }
   server.begin();

   Debug_Trace("WiFi get connected and IP address is %s", WiFi.localIP().toString().c_str());


   /* Init back ground task for process same.*/
   /* Initate Low Priority Background Process Monitering Task */
   xTaskCreatePinnedToCore(
       WebServer_Processing_Task,         /* Task function. */
       "WebServer_Processing_Task",       /* name of task. */
       5120,                              /* 5K Stack size of task */
       NULL,                              /* parameter of the task */
       0,                                 /* priority of the task, Grater the value Higher the priority.*/
       &WebServer_Processing_Task_Handle, /* Task handle to keep track of created task */
       1);                                /* pin task to core 1, Along with loop() function. */
}

void loop()
{
   unsigned long Loop_Index = 200;

   int incomingByte = 0; // for incoming serial data

   // put your main code here, to run repeatedly:

#if 0
/* Test Case:- 1, 
   1. Test with half the Queue and each have Moderate Buffer sizes ~68*50 = 3400 Byte
 */

for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue/2); Loop_Index++)
{
   Debug_Trace("Hello I am here, To Test the debug Trace printing_ %03d ", Loop_Index);
}

/* Flesh for flesh the output*/
vTaskDelay(10000 / portTICK_PERIOD_MS);
#endif

#if 0
/* Test Case:- 2, 
   1. Test with Full the Queue and each have Moderate Buffer sizes ~68*100 = 6800 Byte
   2. Grater than the allocated buffer 6144.
   3. Expecting to waite for buffer to get free on buffer Index 90= ~ 68*90 = 6120 Bytes

 */

for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue); Loop_Index++)
{
   Debug_Trace("Hello I am here, To Test the debug Trace printing_ %03d", Loop_Index);
}


/* Flesh for flesh the output*/
vTaskDelay(15000 / portTICK_PERIOD_MS);
#endif

#if 0
/* Test Case:- 3, 
   1. Test with Double the Queue and consumming buffer less tthan max, Each request can use 25 Bytes
   2. Grater than the allocated buffer 6144.
   3. Expecting to waite for Queue / timeout ( based on configuration) to and max buffer usage shall not exceed 26*200 = 5200 Bytes

 */

for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue * 2); Loop_Index++)
{
   Debug_Trace("Hello I am here, To Test the debug Trace printing_ %03d", Loop_Index);
}

/* Flesh for flesh the output*/
vTaskDelay(15000 / portTICK_PERIOD_MS);
#endif

#if 0
/* Test Case:- 4, 
   1. Test with full buffer with Queue pending, uses 20 Queue and each buffer upto 368, So 368*20 = 7360 Byte
   2. Grater than the allocated buffer 6144.
   3. Expecting to waite for buffer to get free on buffer Index 16= ~ 368*16 = 5888 Bytes

 */

for (Loop_Index = 0; Loop_Index < (20); Loop_Index++)
{
   Debug_Trace("Hello printing_ %03d I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace", Loop_Index);
}

/* Flesh for flesh the output*/
vTaskDelay(15000 / portTICK_PERIOD_MS);
#endif

#if 0
/* Test Case:- 5, 
   1. Test with full buffer with Queue pending, uses 16 Queue and each buffer upto 368, So 368*20 = 7360 Byte
   2. Grater than the allocated buffer 6144.
   3. Expecting to No waite for buffer because shall give give enough delay for complete the printing. 16= ~ 368*16 = 5888 Bytes

 */

for (Loop_Index = 0; Loop_Index < (17); Loop_Index++)
{
   Debug_Trace("Hello printing_ %03d I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace", Loop_Index);
}

/* Flesh for flesh the output*/
vTaskDelay(15000 / portTICK_PERIOD_MS);
#endif

#if 0
/* Test Case:- 6, ( Over Night Test)
   1. Test with 200 Queue and each have Moderate Buffer sizes ~68*200 and 368*200 = 13600  + 73600 Byte
   2. Grater than the allocated buffer 6144.
   3. Expecting to waite for buffer and Queue to get free on buffer abd Required Queues.
   4. Will get delay, but shhall not expect any and Error, all buffer shall process in paraller.

 */

for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue * 2); Loop_Index++)
{
   Debug_Trace("Hello I am here, To Test the debug Trace printing_ %03d", Loop_Index);
   Debug_Trace("Hello printing_ %03d I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace", Loop_Index);

}

/* Flesh for flesh the output*/
vTaskDelay(15000 / portTICK_PERIOD_MS);
#endif

#if 0
/* Test Case:- 7, 
   1. Test Buffer buffer case 96*64 = 6144 Byte
       Buffer sizes = 96, 13 Character stack shall add for time stamp, then our string shall be 96-13 = 83 in this case bounder reach @ queue 64
   2. Grater than the allocated buffer 6144.
   3. Expecting No glitch @ index 63 to 67

 */

for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue); Loop_Index++)
{
   Debug_Trace("Index = %03d Hello I am here, To Test the debug Trace printing_,To Test the debug T", Loop_Index);
}

/* Flesh for flesh the output*/
vTaskDelay(15000 / portTICK_PERIOD_MS);
#endif

#if 0
/* Test Case:- 8, One Byte overlapping 
   1. Test Buffer buffer case 96*64 = 6144 Byte
       Buffer sizes = 96, 13 Character stack shall add for time stamp, then our string shall be 96-13 = 83 in this case bounder reach @ queue 64
       @index 63 add string having one more byte, just  to see the overlapping works in cyclic buffer mechanism,
   2. Grater than the allocated buffer 6144.
   3. Expecting No glitch @ index 63 to 67

 */

for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue); Loop_Index++)
{
   /* Check if index is Not 63*/
   if(Loop_Index != 63)
   {
     Debug_Trace("Index = %03d Hello I am here, To Test the debug Trace printing_,To Test the debug T", Loop_Index);
   }
   /*if Index in 63 add one more byte*/
   else
   {
      Debug_Trace("Index = %03d Hello I am here, To Test the debug Trace printing_,To Test the debug T1", Loop_Index);
   }
}

/* Flesh for flesh the output*/
vTaskDelay(15000 / portTICK_PERIOD_MS);
#endif

#if 0
/* Test Case:- 9, Queue and buffer fulling at same time, Such that No 
   1. Test Buffer buffer case (61*99) + (1 * 105) = 6144 Byte
       Buffer sizes = 61 for 99 Queue, 13 Character stack shall add for time stamp, then our string shall be 61-13 = 48 in this case bounder reach @ queue 99 Index
       and last buffer sizes = 105, 13 Character stack shall add for time stamp, then our string shall be 105-13 = 92
   2. Grater than the allocated buffer 6144.
   3. Expecting No glitch @ index 98 to 0, And No increae in time, All 0 to 99 buffering shall complete in less than 3ms

 */

for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue); Loop_Index++)
{
   /* Check if index is Not 63*/
   if(Loop_Index != 99)
   {
     Debug_Trace("Index = %03d Hello I am here, To Test the debug.", Loop_Index);
   }
   /*if Index in 63 add one more byte*/
   else
   {
      Debug_Trace("Index = %03d Hello I am here, To Test the debug Trace printing_,To Test the debug Trace prin", Loop_Index);
   }
}

/* Flesh for flesh the output*/
/* Just enough time to complete 100 Queue in 100ms = 100 * 100ms = 10000ms */
vTaskDelay(10000 / portTICK_PERIOD_MS);
#endif

#if 0
   /* Test Case:- 10, One Byte overlapping + Stream to Buffer test.
   1. Test Buffer buffer case 96*64 = 6144 Byte
       Buffer sizes = 96, 13 Character stack shall add for time stamp, then our string shall be 96-13 = 83 in this case bounder reach @ queue 64
       @index 63 add string having one more byte, just  to see the overlapping works in cyclic buffer mechanism,
   2. Grater than the allocated buffer 6144.
   3. Expecting No glitch @ index 63 to 67

 */

   for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue); Loop_Index++)
   {
      /* Check if index is Not 63*/
      if (Loop_Index != 63)
      {
         Debug_Trace("Index = %03d Hello I am here, To Test the debug Trace printing_,To Test the debug T", Loop_Index);
      }
      /*if Index in 63 add one more byte*/
      else
      {
         Debug_Trace("Index = %03d Hello I am here, To Test the debug Trace printing_,To Test the debug T1", Loop_Index);
      }
   }

   /* Flesh for flesh the output*/
   vTaskDelay(15000 / portTICK_PERIOD_MS);

   Populate_BufferStream_FromQueue(Test_Buffer_Stream, (Max_BackGround_Buffer_Reserved + 400));

   Serial.write(Test_Buffer_Stream);

#endif


#if 1
   /* Test Case:- 11, Test on HTML
   1. Test Buffer buffer case 96*64 = 6144 Byte
       Buffer sizes = 96, 13 Character stack shall add for time stamp, then our string shall be 96-13 = 83 in this case bounder reach @ queue 64
       @index 63 add string having one more byte, just  to see the overlapping works in cyclic buffer mechanism,
   2. Grater than the allocated buffer 6144.
   3. Expecting No glitch @ index 63 to 67

 */

   for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue * 20); Loop_Index++)
   {
      /* Check if index is Not 63*/
      if (Loop_Index != 63)
      {
         Debug_Trace("Index = %03d Hello I am here, To Test the debug Trace printing_,To Test the debug T", Loop_Index);
      }
      /*if Index in 63 add one more byte*/
      else
      {
         Debug_Trace("Index = %03d Hello I am here, To Test the debug Trace printing_,To Test the debug T1", Loop_Index);
      }

   /* waite for 500ms*/
   vTaskDelay(500 / portTICK_PERIOD_MS);
   }



   



#endif
















   Debug_Trace("\n-------------------------------------\n End of First Loop \n Please press any Key to continue...\n-------------------------------------", Loop_Index);

   // send data only when you receive data:
   while (Serial.available() <= 0)
   {
      /* Waite for 1 Sec*/
      vTaskDelay(1000 / portTICK_PERIOD_MS);
   }

   while (Serial.available() > 0)
   {
      // read out all  the incoming byte:
      incomingByte = Serial.read();
   }
}




/* Process all Html Web server here.*/
void Process_Webserver(void)
{

   /* Variable to store the HTTP request*/
   String Client_Header_Request;
   String currentLine;

   /*Current time and previous time for time out purpose.*/
   unsigned long currentTime = millis();
   /*Previous time*/
   unsigned long previousTime = 0;

   char ClientCurrent_Char;

   /* Check if any  client has connected*/
   WiFiClient client = server.available();

   if (client)
   { /* If a new client connects,*/

      /* Read current time.*/
      currentTime = millis();

      /* Update the previous time*/
      previousTime = currentTime;

      //Debug_Trace("New Clint has been connected...");

      /* Clean the string to make a String to hold incoming data from the client*/
      currentLine = "";

      /*loop while the client's connected, This time out logic is just for example purpose, Shall not work in over flow case.*/
      while (client.connected() && currentTime - previousTime <= html_TimeOut_Clint)
      {
         /* Get the current time.*/
         currentTime = millis();

         /* if there's bytes to read from the client, */
         if (client.available())
         {
            /* Read each available Byte form client*/
            ClientCurrent_Char = client.read();

            /* Stor into a string variable.*/
            Client_Header_Request += ClientCurrent_Char;

            /* if the byte is a newline character */
            if (ClientCurrent_Char == '\n')
            {
               /* if the current line is blank, you got two newline characters in a row.
             that's the end of the client HTTP request, so send a response:
          */
               if (currentLine.length() == 0)
               {
                  /* HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK) */
                  /* and a content-type so the client knows what's coming, then a blank line:*/
                  client.println("HTTP/1.1 200 OK");
                  client.println("Content-type:text/html");
                  client.println();

                  // Display the HTML web page
                  client.print(Debug_Header_Temp);

                  /* Get Queue string*/
                  Populate_BufferStream_FromQueue(Test_Buffer_Stream, (2000));


                  client.print(Debug_Body_1);
                  client.print(Test_Buffer_Stream);



                  client.print(Debug_Footer_Temp);

                  // The HTTP response ends with another blank line
                  client.println();
                  // Break out of the while loop
                  break;
               }
               else
               { // if you got a newline, then clear currentLine
                  currentLine = "";
               }
            }
            else if (ClientCurrent_Char != '\r')
            {                                     // if you got anything else but a carriage return character,
               currentLine += ClientCurrent_Char; // add it to the end of the currentLine
            }
         }
      }
      // Clear the header variable
      Client_Header_Request = "";
      // Close the connection
      client.stop();
   }
}

void WebServer_Processing_Task(void *pvParameters)
{
   /* Loop for the task.*/
   for (;;)
   {

      /********************************************************************************
    *  Add Code after this line....
    * ******************************************************************************
   */

      /* Trigger function to do processing in every period based on periodicity*/
      Process_Webserver();

      /* Switch task for 100ms */
      vTaskDelay(100 / portTICK_PERIOD_MS);
   }
}