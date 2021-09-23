/*********
  Example or sample code showing how to use interfaces or used at the time of development for testing.
*********/

#include <Arduino.h>
#include "Debug_Trace.h"


void setup() {
   
  /* Init Trace support  */
Init_Trace();


Serial.println("Init_Trace completed...");



}

void loop() 
{
  // put your main code here, to run repeatedly:

Debug_Trace("Hello I am here, To Test the debug Trace printing_ 1 ");
Debug_Trace("Hello I am here, To Test the debug Trace printing_ 2 ");
Debug_Trace("Hello I am here, To Test the debug Trace printing_ 3 ");
Debug_Trace("Hello I am here, To Test the debug Trace printing_ 4 ");




delay(500);
}
