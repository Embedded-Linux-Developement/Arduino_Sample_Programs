/*****************************************************************************************************
Header file  Asynchronous Morse Code Generator.
Description:- Header file  Asynchronous Morse Code Generator, 
              Here Morse code generatation shall be in back ground, using a separate low priority task.
Limitation:- 1. At present support only for one LED, 
Copy Write:- 
Licence:- 
    <Asynchronous Morse Code Generator>
    Copyright (C) 2021  Jerry James

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    
Auther:- Jerry James
E-mail:- Jerrymbc@gmail.com
Date  :- 30 Aug 2021
version:- V1.0.1

*******************************************************************************************************/

#include <Arduino.h>
#include "Asynchronous_Morse_Code_Generator.h"

/* Indicate Max number of paten supported*/
#define Maximum_Supported_Paten 55
/* Indicate Max number of Bits in each supported*/
#define Maximum_PatenBits 8

/* Following is the Structure to store the data base of the Morse code pattern accordingly.*/
typedef struct Morse_Code_Parten_Table_Tag {
   char     ReferenceChar;                             /* Represent the ASCI Character for which paten is stored*/
   char     MorsePatern[Maximum_PatenBits];            /* Represent the Paten, and shall end with \0 */      
}Morse_Code_Parten_Table_Type;

/* Following is the Global const array to represent Morse code for each Character*/
const Morse_Code_Parten_Table_Type MorseCodePaternDatabase[Maximum_Supported_Paten] =
{
  {'a',"._"},
  {'b',"_..."},
  {'c',"_._."},
  {'d',"_.."},
  {'e',"."},
  {'f',".._."},
  {'g',"__."},
  {'h',"...."},
  {'i',".."},
  {'j',".___"},
  {'k',"_._"},
  {'l',"._..."},
  {'m',"__"},
  {'n',"_."},
  {'o',"___"},
  {'p',".__."},
  {'q',"__._"},
  {'r',"._."},
  {'s',"..."},
  {'t',"_"},
  {'u',".._"},
  {'v',"..._"},
  {'w',".__"},
  {'x',"_.._"},
  {'y',"_.__"},
  {'z',"__.."},
/*Numbers */
  {'0',"_____"},
  {'1',".____"},
  {'2',"..___"},
  {'3',"...__"},
  {'4',"...._"},
  {'5',"....."},
  {'6',"_...."},
  {'7',"__..."},
  {'8',"___.."},
  {'9',"____."},
/*Special Characters */
  {'&',"._..."},
  {'\'',".____."},
  {'@',".__._."},
  {')',"_.__._"},
  {'(',"_.__."},
  {':',"___..."},
  {',',"__..__"},
  {'=',"_..._"},
  {'!',"_._.__"},
  {'.',"._._._"},
  {'-',"_...._"},
  {'*',"_.._"},
  {'%',"_.._."},
  {'+',"._._."},
  {'"',"._.._."},
  {'?',"..__.."},
  {'/',"_.._."},
  {'/',"_.._."}
};



/* Buffer for each based on defined sizes*/
static char MC_Buffer_0_String[MC_Sizes_Buffer_0 + 1 ];
static char MC_Buffer_1_String[MC_Sizes_Buffer_1 + 1 ];
static char MC_Buffer_2_String[MC_Sizes_Buffer_2 + 1 ];
static char MC_MirroBuffer_String[MC_Sizes_MirrorBuffer + 1 ];

/* Variable to represent the status of the Each buffer*/
static Morse_Code_Buffer_Status Internal_Each_Buffer_Status[MC_Max_NumberOfBuffer];

/* Mutex variable to access the buffers and status.*/
portMUX_TYPE Morse_Code_Mutex = portMUX_INITIALIZER_UNLOCKED; 
/* Handle for BackgroundMorseCode_Task */
TaskHandle_t BackgroundMorseCode_Task_Handle;

/*
===========================================================================
===========================================================================
          Privite functions related to Control system System
===========================================================================
==========================================================================

/* ************************************************************************
 * Function to Init all required back ground process for Morse code
 * *************************************************************************/
void Process_MorseCode(void)
{




}




/* ********************************************************************************
 * Task for process the Morse code generation
   Periodicity:-  50ms
   Priority   :-  5 (Low)
 * *********************************************************************************/
void BackgroundMorseCode_Task( void * pvParameters ){
  /* Loop for the task.*/
  for(;;){

   /********************************************************************************
    *  Add Code after this line....
    * ******************************************************************************
   */

    /* Trigger function to do processing in every 50ms */
     Process_MorseCode();

   
   /* Switch task for 50ms */
   vTaskDelay(Morse_Code_processing_Task_Perion_ms / portTICK_PERIOD_MS);
  }
}


/*
===========================================================================
===========================================================================
          Public functions related to System Operatations
===========================================================================
==========================================================================
*/



/* ************************************************************************
 * Function to Init all required back ground process for Morse code
 * *************************************************************************/
void Morse_Code_Init(void)
{
  unsigned char Index;

  /* Enter in to Critical Section*/
  portENTER_CRITICAL(&Morse_Code_Mutex);

  /* Clear all buffer status to Clear stat.*/
  for( Index = 0; Index < MC_Max_NumberOfBuffer; Index++)
  {
    Internal_Each_Buffer_Status[Index] = MC_BUffer_Clean;
  }

  /* Exit from Critical Section. */
  portEXIT_CRITICAL(&Morse_Code_Mutex);

  /* Start the Back ground task for process the Buffer.*/
  /* Initate Low Priority Background Process Monitering Task */
  xTaskCreatePinnedToCore(
      BackgroundMorseCode_Task,         /* Task function. */
      "BackgroundMorseCode_Task",       /* name of task. */
      5120,                             /* 5K Stack size of task */
      NULL,                             /* parameter of the task */
      5,                                /* priority of the task, Grater the value Higher the priority.*/
      &BackgroundMorseCode_Task_Handle, /* Task handle to keep track of created task */
      1);                               /* pin task to core 1, Along with loop() function. */




}

/* ************************************************************************
 * Function to Sent the morse code to the respective buffer.
 * *************************************************************************/
void Morse_Code_Sent(const char *InputString, Morse_Code_Out_BUffer DestBuffer)
{
  /* Variable for Loop*/
  unsigned short BufferIndex;
  /* Variable to store Max sizes allowed*/
  unsigned short Current_Buffer_Sizes = 0;
  /* Variable to store the buffer pointer*/
  char *CurrentBufferPrt;

  /*Get Max sizes based on the Buffer type*/
  if (DestBuffer == MorseCodeBUffer_0)
  {
    Current_Buffer_Sizes = MC_Sizes_Buffer_0;
    CurrentBufferPrt = MC_Buffer_0_String;
  }
  else if (DestBuffer == MorseCodeBUffer_1)
  {
    Current_Buffer_Sizes = MC_Sizes_Buffer_1;
    CurrentBufferPrt = MC_Buffer_1_String;
  }
  else if (DestBuffer == MorseCodeBUffer_2)
  {
    Current_Buffer_Sizes = MC_Sizes_Buffer_2;
    CurrentBufferPrt = MC_Buffer_2_String;
  }
  else /* Un supported buffer, So ignore*/
  {
    Current_Buffer_Sizes = 0;
  }

  /* Check is buffer is valied*/
  if (Current_Buffer_Sizes != 0)
  {

    /* Enter in to Critical Section*/
    portENTER_CRITICAL(&Morse_Code_Mutex);

    /* Copy the data to respective buffer*/
    for (BufferIndex = 0; ((BufferIndex < Current_Buffer_Sizes) && (InputString[BufferIndex] != '\0')); BufferIndex++)
    {
      /* Copy content*/
      CurrentBufferPrt[BufferIndex] = InputString[BufferIndex];
      /* Convert to Lowe case if any */
      if (( CurrentBufferPrt[BufferIndex] & 0xDF >= 'A' ) && ( CurrentBufferPrt[BufferIndex] & 0xDF <= 'Z' )) 
      {
         CurrentBufferPrt[BufferIndex]  &= 0xDF; // so that bit 5 only gets cleared for alphas 
      }
     
    }
    /* Add Null Character at the end */
    CurrentBufferPrt[BufferIndex] = '\0';

    /* Update the the status to indicate new request buffer is in Queue */
    Internal_Each_Buffer_Status[DestBuffer] = MC_BUffer_InQueue;

    /* Exit from Critical Section. */
    portEXIT_CRITICAL(&Morse_Code_Mutex);

  } /* End of buffer checking.*/
  else
  {
    /* Do Nothing*/
  }
}

/* ************************************************************************
 * Function to Clear the morse code of the respective buffer.
 * Shall not stop if requested one is on going. shall stop once its completed.
 * *************************************************************************/
void Morse_Code_Clear(Morse_Code_Out_BUffer DestBuffer)
{

  /* Check if buffer ID us valied */
  if (DestBuffer < MorseCodeBUffer_MAx)
  {
    /* Enter in to Critical Section*/
    portENTER_CRITICAL(&Morse_Code_Mutex);

    /* Update the the status to indicate new request buffer is cleanned */
    Internal_Each_Buffer_Status[DestBuffer] = MC_BUffer_Clean;

    /* Exit from Critical Section. */
    portEXIT_CRITICAL(&Morse_Code_Mutex);
  }
  else
  {
    /* Do nothing because request is in valied.*/
  }
}

/* ************************************************************************
 * Function to get curent status of the processing.
 * *************************************************************************/
Morse_Code_Buffer_Status Morse_Code_GetStatus(Morse_Code_Out_BUffer DestBuffer)
{
   /* Return Variable*/
  Morse_Code_Buffer_Status Return_Value = MC_BUffer_Clean;

  /* Check if buffer ID us valied */
  if (DestBuffer < MorseCodeBUffer_MAx)
  {
    /* Enter in to Critical Section*/
    portENTER_CRITICAL(&Morse_Code_Mutex);

    /* Get the the status of the requested buffer*/
    Return_Value = Internal_Each_Buffer_Status[DestBuffer];

    /* Exit from Critical Section. */
    portEXIT_CRITICAL(&Morse_Code_Mutex);
  }
  else
  {
    /* Do nothing because request is in valied.*/
  }

  /* Return that status*/
  return(Return_Value);

}







