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
  unsigned long Loop_Index;
  // put your main code here, to run repeatedly:


/* Test Case:- 1, 
   1. Test with half the Queue and each have Moderate Buffer sizes ~67*50 = 3350 Byte
 */

for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue/2); Loop_Index++)
{
   Debug_Trace("Hello I am here, To Test the debug Trace printing_ %03d ", Loop_Index);
}




delay(10000);
}
