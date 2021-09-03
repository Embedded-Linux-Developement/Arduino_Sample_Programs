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
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
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

/* Following Pipeline states define different states in generating and transimiting a Morse code in back ground.*/
enum  Morse_Code_Generation_Pipeline{
  MC_Generation_Mirroring            = 20,    /* Its a state where the mirroring of Buffer and status took place */
  MC_Generation_CharEncoding         = 22,    /* Its a state where loop for the paters of the  morse code for each character in the buffer.*/
  MC_Generation_SigTx_ON             = 24,    /* Its a state where each upcoming Dot / Dash Transmit or set as ON*/
  MC_Generation_SigTx_ON_wait        = 28,    /* Its a state where wait after Set each previous Dot / Dash with respect to there ON dubitation, Its just wait for the duration requested*/
  MC_Generation_SigTx_OFF            = 40,    /* Its a state where Clear / OFF  previous set Dot / Dash accordingly*/
  MC_Generation_SigTx_OFF_wait       = 42,    /* Its a state where wait till OFF dubitation, Its just wait for the duration requested.*/
  MC_Generation_Char_wait            = 44,    /* Its a state where wait till OFF dubitation of character completed, after a Off. This state shall trigger from MC_Generation_SigTx_ON only*/
  MC_Generation_Word_wait            = 46,    /* Its a state where wait till OFF dubitation of word completed, after a Char wait. This state shall trigger from MC_Generation_CharEncoding only*/
  MC_Generation_Process_Completed    = 48,    /* Its a state where processing and generatation of the mentioned buffer is completed.*/
};




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
  {'l',"._.."},
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

/* Macro to enable of dissable the Delimiter after each butter, Shall add additional bytes
   True => Enable
   False => Dissabled
*/
#define Add_Buffer_Delimiter True

/* Additional margin for buffer to ass special characters*/
#if (Add_Buffer_Delimiter == True)
#define BufferAdditionalMargin 5  
#else
#define BufferAdditionalMargin 2
#endif

/* Buffer for each based on defined sizes*/
static char MC_Buffer_0_String[MC_Sizes_Buffer_0 + BufferAdditionalMargin ];
static char MC_Buffer_1_String[MC_Sizes_Buffer_1 + BufferAdditionalMargin ];
static char MC_Buffer_2_String[MC_Sizes_Buffer_2 + BufferAdditionalMargin ];
static char MC_MirroBuffer_String[MC_Sizes_MirrorBuffer + BufferAdditionalMargin ];

/* Const array to hold the buffer address*/
const char *MC_Buffer_Pointer[MC_Max_NumberOfBuffer] = 
{
   MC_Buffer_0_String,
   MC_Buffer_1_String,
   MC_Buffer_2_String
};

/* Variable to represent the status of the Each buffer*/
static Morse_Code_Buffer_Status Internal_Each_Buffer_Status[MC_Max_NumberOfBuffer];

/* Mutex variable to access the buffers and status.*/
portMUX_TYPE Morse_Code_Mutex = portMUX_INITIALIZER_UNLOCKED; 
/* Handle for BackgroundMorseCode_Task */
TaskHandle_t BackgroundMorseCode_Task_Handle;


/* Macro function to decrement the waitting counter*/
#define Wait_Counter_Decrement() {if(Waiting_Time !=0) {Waiting_Time--;}}

/* Macro function to Increment PreWaite counter*/
#define Pre_Wait_Counter_Increment() {if(PreWaiting_Time !=0xFFFFFFFFU) {PreWaiting_Time++;}}





/*
===========================================================================
===========================================================================
          Privite functions related to Control system System
===========================================================================
==========================================================================

/* ************************************************************************
 * Function to Init all required back ground process for Morse code
 * *************************************************************************/
void Get_Encoded_MorseCode(char InputChar, char *OutputEncodedBuffer)
{
  /* Static Variable to store previous requested character, Just to speed up is keep on requesting*/
  static char Previous_InputChar = 0xff;
  /* Static variable to hold previous Encoded string*/
  static char Previous_OutputEncodedBuffer[(Maximum_PatenBits + 2)] = " ";
  /* Index of the loop*/
  unsigned char Index;

  /* Check if current and previous characters matching*/
  if (Previous_InputChar != InputChar)
  {
    /* Clear the Buffer*/
    strcpy((char *)Previous_OutputEncodedBuffer, " ");
    Previous_InputChar = 0xff;

    /* Loop for march the character*/
    for (Index = 0; Index < Maximum_Supported_Paten; Index++)
    {
      /* Check for matching*/
      if (MorseCodePaternDatabase[Index].ReferenceChar == InputChar)
      {
        /* Copy the content*/
        strcpy((char *)Previous_OutputEncodedBuffer, (char *)(&MorseCodePaternDatabase[Index].MorsePatern[0]));
        /* Break the loop */
        Index = Maximum_Supported_Paten + 5;
        /* Update previous char*/
        Previous_InputChar = InputChar;
      }
      else
      {
        /* Do nothing.*/
      }
    }
  }
  else /* Request for same character*/
  {
    /* Do nothing*/
  }

  /* Copy final output to the output array*/
  strcpy((char *)OutputEncodedBuffer, (char *)Previous_OutputEncodedBuffer);


}

/* ************************************************************************
 * Function to Init all required back ground process for Morse code
 * *************************************************************************/
void Process_MorseCode(void)
{
  /*************************************************************************
   *                           Static variables
  *************************************************************************/
 /* Static variable for Store the Pipeline of the Current buffer   */
 static Morse_Code_Generation_Pipeline CurrentBufferPipeline = MC_Generation_Mirroring;
 /* Store the index of the current processing buffer.*/
 static unsigned char Processing_Buffer_Index = 0;
 /* Store the index of the location mirror we are processing.*/
 static unsigned char Each_Buffer_Char_Index = 0;
 /* Current Character to be processed*/
 static  char Current_Char_To_Process = ' ';
 /* Encoded string*/
 static char Each_Char_Encoded_Buffer[(Maximum_PatenBits + 2)] = "";
 /* Current index of the encoded string*/
 static unsigned char Each_Char_Encoding_Index = 0;
 /* waiting time for any waiting state, in ticks*/
 static unsigned long Waiting_Time = 0;
 /* Pre waiting time for calculate how much time waited in any previous wait pipeline*/
 static unsigned long PreWaiting_Time = 0;


/* Switch to go through each Pipeline status*/
 switch (CurrentBufferPipeline)
 {
/* -----------------------------------------------------------------
  Its a state where the mirroring of Buffer and status took place

  PipeLine Input Flow  :- Entry Point, MC_Generation_Process_Completed
  PipeLine Output Flow :- MC_Generation_CharEncoding
  -----------------------------------------------------------------*/
 case MC_Generation_Mirroring:
 {

   /* Check if buffer index is within the range */
   if (Processing_Buffer_Index < MC_Max_NumberOfBuffer)
   {
     /* Enter in to Critical Section*/
     portENTER_CRITICAL(&Morse_Code_Mutex);

     /* Check the status of the buffer*/
     if (Internal_Each_Buffer_Status[Processing_Buffer_Index] != MC_BUffer_Clean)
     {
       /* Copy this buffer to mirror.*/
       strcpy((char *)MC_MirroBuffer_String, (char *)(MC_Buffer_Pointer[Processing_Buffer_Index]));

       /* Update the status if previous was in "MC_BUffer_InQueue"*/
       if (Internal_Each_Buffer_Status[Processing_Buffer_Index] == MC_BUffer_InQueue)
       {
         /* Update the status */
         Internal_Each_Buffer_Status[Processing_Buffer_Index] = MC_BUffer_FirstInProgress;
       }

       /* Clear required global variables */
       Each_Buffer_Char_Index = 0;
       Current_Char_To_Process = ' ';
       strcpy((char *)Each_Char_Encoded_Buffer, "");
       Each_Char_Encoding_Index = 0;
       Waiting_Time = 0;
       PreWaiting_Time = 0;


       /* Initate switching to next Pipeline*/
       CurrentBufferPipeline = MC_Generation_CharEncoding;
     }
     else
     {
       /* Skip to Next Buffer */

      /* Increment the buffer pointer to next*/
        Processing_Buffer_Index++;
     }



     /* Exit from Critical Section. */
     portEXIT_CRITICAL(&Morse_Code_Mutex);
   }
   else /* Stored Index exceed the limit*/
   {
     /* Reseting the Index */
     Processing_Buffer_Index = 0;
   }
   break;
 }


   /* -----------------------------------------------------------------
  Its a state where loop for the paters of the  morse code for each character in the buffer.

  PipeLine Input Flow  :- MC_Generation_Mirroring, MC_Generation_Word_wait
                          MC_Generation_Char_wait
  PipeLine Output Flow :- MC_Generation_Word_wait, MC_Generation_SigTx_ON, 
                          MC_Generation_Mirroring ( When Error found),
                          MC_Generation_Process_Completed
  -----------------------------------------------------------------*/
 case MC_Generation_CharEncoding:
 {


   /* Check wheather Buffer Index is within the range, else reset*/
   if (Each_Buffer_Char_Index < (MC_Sizes_MirrorBuffer + BufferAdditionalMargin))
   {

     /* Record current Tike time consumed*/
     Pre_Wait_Counter_Increment();

     /* Get current char to process */
     Current_Char_To_Process = MC_MirroBuffer_String[Each_Buffer_Char_Index];

     /* Increase the buffer pointer to process new charactor next time*/
     Each_Buffer_Char_Index++;

     /* Check if current state is null pointer, if so switch to MC_Generation_Process_Completed */
     if (Current_Char_To_Process == '\0')
     {
       /* Switch to Pipeline Complete*/
       CurrentBufferPipeline = MC_Generation_Process_Completed;
     }
     /* If current charactor is a Space add word wait*/
     else if (Current_Char_To_Process == ' ')
     {
       /* Calculate the waiting time required*/
       if (PreWaiting_Time < Morse_Code_BwtWord_Time)
       {
         Waiting_Time = (Morse_Code_BwtWord_Time - PreWaiting_Time);
       }
       else /* No Wait required*/
       {
         Waiting_Time = 0;
       }

       /* Clear pre-waite, as not required further*/
       PreWaiting_Time = 0;

       /* Switch to Pipeline MC_Generation_Word_wait*/
       CurrentBufferPipeline = MC_Generation_Word_wait;
     }
     /* Normal character, So decord and check*/
     else
     {
       /* Decord the charactor*/
       Get_Encoded_MorseCode(Current_Char_To_Process, Each_Char_Encoded_Buffer);

       /* Switch to Pipeline MC_Generation_SigTx_ON*/
       CurrentBufferPipeline = MC_Generation_SigTx_ON;
      /* Reset required global variables*/
      Each_Char_Encoding_Index = 0;

     }
   }
   else /* Buffer Index outside the range*/
   {
     /* Reset the index */
     Each_Buffer_Char_Index = 0;
     /* Switch the Pipeline to start, Error Recover*/
     CurrentBufferPipeline = MC_Generation_Mirroring;

   }

   break;
 }


   /* -----------------------------------------------------------------
  Its a state where each upcoming Dot / Dash Transmit or set as ON.

  PipeLine Input Flow  :- MC_Generation_CharEncoding, MC_Generation_SigTx_OFF_wait
  PipeLine Output Flow :- MC_Generation_SigTx_ON_wait, MC_Generation_Char_wait
  -----------------------------------------------------------------*/
 case MC_Generation_SigTx_ON:
 {

   /* Set previous wait */
   Pre_Wait_Counter_Increment();/* One tick being already consumed in this tick*/

   /* Check if buffer for encoder overrun */
   if (Each_Char_Encoding_Index < (Maximum_PatenBits + 2))
   {

     /* Check if its a Null charactor*/
     if (Each_Char_Encoded_Buffer[Each_Char_Encoding_Index] == '\0')
     {
       /* Clear buffer Index */
       Each_Char_Encoding_Index = 0;

       /* Check if previous time is less than required one */
       if (PreWaiting_Time < Morse_Code_BwtCharacter_Time)
       {
         /* Set Timeout for Charactor wait*/
         Waiting_Time = Morse_Code_BwtCharacter_Time - PreWaiting_Time;
       }
       /* Already more time passed*/
       else
       {
         /* Clear waitting time */
         Waiting_Time = 0;
       }

       /* Set Pipeline to MC_Generation_Char_wait*/
       CurrentBufferPipeline = MC_Generation_Char_wait;
     }
     else
     {
       /* Set waitting time */
       PreWaiting_Time = 1; /* One tick being already consumed in this tick*/
       /* Check if the current pattern in Dot*/
       if (Each_Char_Encoded_Buffer[Each_Char_Encoding_Index] == Morse_Dot_Char)
       {
         /* Store the ON time for Dot*/
         Waiting_Time = Morse_Code_Dot_Time - PreWaiting_Time; 
       }
       /* Check if the current pattern in Dot*/
       else if (Each_Char_Encoded_Buffer[Each_Char_Encoding_Index] == Morse_Dash_Char)
       {
         /* Store the ON time for Dot*/
         Waiting_Time = Morse_Code_Dash_Time - PreWaiting_Time; 
       }
       else /* Ignore un known Characters*/
       {
         /* Clear the waiting time*/
         Waiting_Time = 0;
       }

       /* Check if its a valied char*/
       if(Waiting_Time != 0)
       {
          /* Set Port Out to make LED ON*/
           digitalWrite(Morse_Code_LED, DotAndDash_ON);

          /* Set Next Pipe line*/
          CurrentBufferPipeline = MC_Generation_SigTx_ON_wait;
       }
       else
       {
         /* Do nothing, Keep in same Pipe line and ignore current value*/
       }

       /* Increase the pointer*/
       Each_Char_Encoding_Index++;
     }
   }
   /* Encoding buffer overflowed*/
   else
   {
     /* Clear buffer Index */
     Each_Char_Encoding_Index = 0;

     /* Set Timeout for Charactor wait*/
     Waiting_Time = Morse_Code_BwtCharacter_Time - PreWaiting_Time; 

     /* Set Pipeline to MC_Generation_Char_wait*/
     CurrentBufferPipeline = MC_Generation_Char_wait;
   }


   break;
 }


   /* -----------------------------------------------------------------
  Its a state where wait after Set each previous Dot / Dash 
  with respect to there ON dubitation, Its just wait for the duration requested

  PipeLine Input Flow  :- MC_Generation_SigTx_ON
  PipeLine Output Flow :- MC_Generation_SigTx_OFF
  -----------------------------------------------------------------*/
 case MC_Generation_SigTx_ON_wait:
 {

   /* Decrement the Wait counter, if its Not Zero */
   Wait_Counter_Decrement();

   /* Increment Pre-wait counter*/
   Pre_Wait_Counter_Increment();

   /* Check wheather wait is over */
   if (Waiting_Time == 0)
   {
     /* Clear Pre-waite to Zero, Because this time of slot is also belong to ON wait */
     PreWaiting_Time = 0;

     /* Set next pipe line to MC_Generation_SigTx_OFF*/
     CurrentBufferPipeline = MC_Generation_SigTx_OFF;
   }
   else
   {
     /* Do nothing */
   }

   break;
 }

   /* -----------------------------------------------------------------
  Its a state where Clear / OFF  previous set Dot / Dash accordingly

  PipeLine Input Flow  :- MC_Generation_SigTx_ON_wait
  PipeLine Output Flow :- MC_Generation_SigTx_OFF_wait
  -----------------------------------------------------------------*/
 case MC_Generation_SigTx_OFF:
 {

   /* Increment Pre-counter as this time slot is also includdedin OFF time period*/
   /* Expecting PreWaiting_Time = 0, before this step based on pipe line flow.*/
   Pre_Wait_Counter_Increment();

   /* Set Morse code LED Port Pin to OFF state, */
   digitalWrite(Morse_Code_LED, DotAndDash_OFF);

   /* Set Wait time for the Off period*/
   Waiting_Time = Morse_Code_BwtDotDash_Time - PreWaiting_Time;

   /* Set next pipe line to MC_Generation_SigTx_OFF_wait*/
   CurrentBufferPipeline = MC_Generation_SigTx_OFF_wait;

   break;
 }

   /* -----------------------------------------------------------------
  Its a state where wait till OFF dubitation, Its just wait for the duration requested.

  PipeLine Input Flow  :- MC_Generation_SigTx_OFF
  PipeLine Output Flow :- MC_Generation_SigTx_ON
  -----------------------------------------------------------------*/
 case MC_Generation_SigTx_OFF_wait: 
 {

   /* Decrement the Wait counter, if its Not Zero */
   Wait_Counter_Decrement();
   
   /* Increment Pre-wait counter, and preserve its time */
   Pre_Wait_Counter_Increment();

   /* Check wheather wait is over */
   if(Waiting_Time == 0)
   {

      /* Set next pipe line to MC_Generation_SigTx_ON. to further process*/
      CurrentBufferPipeline = MC_Generation_SigTx_ON;

   }
   else
   {
     /* Do nothing */
   }


   break;
 }


/* -----------------------------------------------------------------
  Its a state where wait till OFF dubitation of character completed, after a Off. 
  This state shall trigger from MC_Generation_SigTx_ON only

  PipeLine Input Flow  :- MC_Generation_SigTx_ON
  PipeLine Output Flow :- MC_Generation_CharEncoding
  -----------------------------------------------------------------*/
 case MC_Generation_Char_wait: 
 {

   /* Decrement the Wait counter, if its Not Zero */
   Wait_Counter_Decrement();
   
   /* Increment Pre-wait counter, and preserve its time */
   Pre_Wait_Counter_Increment();

   /* Check wheather wait is over */
   if(Waiting_Time == 0)
   {

      /* Set next pipe line to MC_Generation_CharEncoding. to further process*/
      CurrentBufferPipeline = MC_Generation_CharEncoding;

   }
   else
   {
     /* Do nothing */
   }


   break;
 }


/* -----------------------------------------------------------------
  Its a state where wait till OFF dubitation of word completed, after a Char wait. 
  This state shall trigger from MC_Generation_CharEncoding only

  PipeLine Input Flow  :- MC_Generation_CharEncoding
  PipeLine Output Flow :- MC_Generation_CharEncoding
  -----------------------------------------------------------------*/
 case MC_Generation_Word_wait: 
 {

   /* Decrement the Wait counter, if its Not Zero */
   Wait_Counter_Decrement();
   
   /* Increment Pre-wait counter, and preserve its time */
   Pre_Wait_Counter_Increment();

   /* Check wheather wait is over */
   if(Waiting_Time == 0)
   {

      /* Set next pipe line to MC_Generation_CharEncoding. to further process*/
      CurrentBufferPipeline = MC_Generation_CharEncoding;

   }
   else
   {
     /* Do nothing */
   }


   break;
 }

/* -----------------------------------------------------------------
  Its a state where processing and generatation of the mentioned buffer is completed.

  PipeLine Input Flow  :- MC_Generation_CharEncoding
  PipeLine Output Flow :- MC_Generation_Mirroring
  -----------------------------------------------------------------*/
 case MC_Generation_Process_Completed:
 {

   /* Clear all global variables */
   Each_Buffer_Char_Index = 0;
   Current_Char_To_Process = ' '; 
   strcpy((char *)Each_Char_Encoded_Buffer, "");
   Each_Char_Encoding_Index = 0;
   Waiting_Time = 0;
   PreWaiting_Time = 0;

   /* Update the Buffer status if its in MC_BUffer_FirstInProgress, Other vise not required as no in flow.*/
   /* Update the status if previous was in "MC_BUffer_FirstInProgress"*/
   if (Internal_Each_Buffer_Status[Processing_Buffer_Index] == MC_BUffer_FirstInProgress)
   {
     /* Update the status */
     Internal_Each_Buffer_Status[Processing_Buffer_Index] = MC_BUffer_Played;
   }
   /* Set next pipe line to MC_Generation_Mirroring. to further process*/
   CurrentBufferPipeline = MC_Generation_Mirroring;

   /* Increment the buffer pointer to next*/
   Processing_Buffer_Index++;

   /* Check if its grater than Max buffer, If so Set to zero*/
   if(Processing_Buffer_Index >= MC_Max_NumberOfBuffer)
   {
     /* Set to Zero */
     Processing_Buffer_Index = 0;
   }

   break;
 }
 
 } /* End of Switch CurrentBufferPipeline */
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

   /* Set the direction of the port pin */
   pinMode(Morse_Code_LED, OUTPUT);

   /* Off the LED at starting*/
   digitalWrite(Morse_Code_LED, DotAndDash_OFF);


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
      if (( CurrentBufferPrt[BufferIndex] >= 'A' ) && ( CurrentBufferPrt[BufferIndex] <= 'Z' )) 
      {
         CurrentBufferPrt[BufferIndex]  += 0x20; // so that bit 5 only gets cleared for alphas 
      }
     
    }

/* If addition of delimiter is enabled.*/
#if (Add_Buffer_Delimiter == True)
    
    /* Add delimiter*/
    CurrentBufferPrt[BufferIndex] = ' ';
    BufferIndex++;
    CurrentBufferPrt[BufferIndex] = ':';
    BufferIndex++;
    CurrentBufferPrt[BufferIndex] = ' ';
    BufferIndex++;

#endif

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







