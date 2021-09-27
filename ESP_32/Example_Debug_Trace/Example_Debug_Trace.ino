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
  unsigned long Loop_Index = 200;

  // put your main code here, to run repeatedly:


#if 1
/* Test Case:- 1, 
   1. Test with half the Queue and each have Moderate Buffer sizes ~68*50 = 3400 Byte
 */

for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue/2); Loop_Index++)
{
   Debug_Trace("Hello I am here, To Test the debug Trace printing_ %03d ", Loop_Index);
}

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

#endif


#if 0
/* Test Case:- 3, 
   1. Test with full buffer with Queue pending, uses 20 Queue and each buffer upto 368, So 368*20 = 7360 Byte
   2. Grater than the allocated buffer 6144.
   3. Expecting to waite for buffer to get free on buffer Index 16= ~ 368*16 = 5888 Bytes

 */

for (Loop_Index = 0; Loop_Index < (20); Loop_Index++)
{
   Debug_Trace("Hello printing_ %03d I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace", Loop_Index);
}

#endif


#if 0
/* Test Case:- 4, 
   1. Test with full buffer with Queue pending, uses 16 Queue and each buffer upto 368, So 368*20 = 7360 Byte
   2. Grater than the allocated buffer 6144.
   3. Expecting to No waite for buffer because shall give give enough delay for complete the printing. 16= ~ 368*16 = 5888 Bytes

 */

for (Loop_Index = 0; Loop_Index < (17); Loop_Index++)
{
   Debug_Trace("Hello printing_ %03d I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace printing_Hello I am here, To Test the debug Trace", Loop_Index);
}

#endif


#if 0
/* Test Case:- 5, ( Over Night Test)
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

#endif




delay(10000);
}
