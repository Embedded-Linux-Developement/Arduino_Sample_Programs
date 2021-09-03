/*********

*********/

#include <Arduino.h>
#include "Asynchronous_Morse_Code_Generator.h"


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
   
  /* Init Morse Code status */
Morse_Code_Init();


Serial.println("Morse_Code_Init completed...");



}

void loop() {
  // put your main code here, to run repeatedly:

#if 0  /* To test all Buffers*/
  /* Play only If the status is Clean*/
  if (Morse_Code_GetStatus(MorseCodeBUffer_0) == MC_BUffer_Clean)
  {
    Morse_Code_Sent("Test Buffer 0", MorseCodeBUffer_0);

    Serial.println("1. Printing Buf 0:- Test Buffer 0");
  }
  if (Morse_Code_GetStatus(MorseCodeBUffer_1) == MC_BUffer_Clean)
  {

    Morse_Code_Sent("Test Buffer 1", MorseCodeBUffer_1);

    Serial.println("1. Printing Buf 1:- Test Buffer 1");
  }
  if (Morse_Code_GetStatus(MorseCodeBUffer_2) == MC_BUffer_Clean)
  {
    Morse_Code_Sent("Test Buffer 2  and is the longest", MorseCodeBUffer_2);

    Serial.println("1. Printing Buf 2:- Test Buffer 2  and is the longest");
  }

#endif



#if 1  /* To test all Characters*/
  /* Play only If the status is Clean*/
  if (Morse_Code_GetStatus(MorseCodeBUffer_2) == MC_BUffer_Clean)
  {
    Morse_Code_Sent("a b c d e f g h i j k l m n o p q u r s t u v w x y z 1 2 3 4 5 6 7 8 9 & \\ @ ) ( : ' = ! . - * % + \" ? / ", MorseCodeBUffer_2);

    Serial.println("1. Printing Buf 0:- Test Buffer 0");
  }

#endif


delay(500);
}
