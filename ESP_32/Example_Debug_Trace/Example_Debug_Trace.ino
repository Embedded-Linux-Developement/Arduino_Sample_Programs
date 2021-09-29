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


#if 0
/* Test Case:- 1, 
   1. Test with half the Queue and each have Moderate Buffer sizes ~68*50 = 3400 Byte
 */

for (Loop_Index = 0; Loop_Index < (Max_BackGround_Buffer_Queue/2); Loop_Index++)
{
   Debug_Trace("Hello I am here, To Test the debug Trace printing_ %03d ", Loop_Index);
}

/* Flesh for flesh the output*/
delay(10000);
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
delay(10000);
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
delay(10000);
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
delay(10000);
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
delay(10000);
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
delay(10000);
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
delay(10000);
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
delay(10000);
#endif



#if 1
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




}
