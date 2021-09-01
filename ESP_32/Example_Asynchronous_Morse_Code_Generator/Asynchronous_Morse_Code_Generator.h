#ifndef Asynchronous_Morse_Code_Generator_H
#define Asynchronous_Morse_Code_Generator_H

/*****************************************************************************************************
Header file  Asynchronous Morse Code Generator.
Description:- Header file  Asynchronous Morse Code Generator, 
              Here Morse code generatation shall be in back ground, using a separate low priority task.
Limitation:- 1. At present support only for one LED, 
             2. At present did not suport chime out
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

/* Port Pin tp out the Morse code*/
#define Morse_Code_LED 2

/* Period of the Morse code processing task in ms, 
 It should be minimum 5 times shorter than Dot ON time, 
 for accuracy reasion in high load case.*/
#define Morse_Code_processing_Task_Perion_ms 50

/* Macro function to get the ticks for ms*/
#define Morse_Code_Get_Ticks(InputTime_ms) ((InputTime_ms)/Morse_Code_processing_Task_Perion_ms)

/* Represent LED ON state for Dot and Dash */
#define DotAndDash_ON HIGH
/* Represent LED OFF state for Dot and Dash */
#define DotAndDash_OFF LOW

/* Represent time for Dot in ticks set to 250ms,  */
#define Morse_Code_Dot_Time Morse_Code_Get_Ticks(250)

/* Represent  time for Dash = 3 x Dot Time  */
#define Morse_Code_Dash_Time (Morse_Code_Dot_Time * 3)

/* Represent time Dot and Dash =  Dot Time  */
#define Morse_Code_BwtDotDash_Time Morse_Code_Dot_Time

/* Represent time between 2 character in same word =  Dash Time  */
#define Morse_Code_BwtCharacter_Time Morse_Code_Dash_Time

/* Represent time between 2 word in same word = 7 x Dot Time  */
#define Morse_Code_BwtWord_Time (7 * Morse_Code_Dot_Time)



/* Character representing the Dot sound, Its use to configure different Charactres */
#define Morse_Dot_Char  '.'
/* Character representing the Dash sound */
#define Morse_Dash_Char  '_'
/* Represent sizes of each buffer.*/
#define MC_Sizes_Buffer_0 25
#define MC_Sizes_Buffer_1 50
#define MC_Sizes_Buffer_2 250
#define MC_Sizes_MirrorBuffer 255 /* Must be grates or grater than of all buffer.*/

/* Represent maximum number of buffer*/
#define MC_Max_NumberOfBuffer 3 


/*******************************************************************************
 *  Program Specific data types.
*******************************************************************************/
/*Following Enumerators are to each buffer via which Morse code needs tp be transmit.
  Here for make Morse code senting a concerrent process we introduced 3 different buffer.
  each buffer have different sizes, and shall sent cyclicly until respective buffer is get cleared.
  The back ground process shall first sent buffer 0, then 1, and then 2. and will keep on respite, until it get cleared.
  Current memory allocation for each buffer are 
     Buffer 0  => 25 Bytes
     Buffer 1  => 50 Bytes
     Buffer 2  => 250 Bytes ( Can use for bigger information to be transmitted) )
*/
enum  Morse_Code_Out_BUffer{
  MorseCodeBUffer_0     = 0,                      /* Indicate 0th Buffer  */
  MorseCodeBUffer_1     = 1,                      /* Indicate 1th Buffer*/
  MorseCodeBUffer_2     = 2,                      /* Indicate 2th Buffer*/
  MorseCodeBUffer_MAx   = MC_Max_NumberOfBuffer   /* Indicate invalided buffer range*/
};

/* Data type for represent the status of each buffer.*/
enum  Morse_Code_Buffer_Status{
  MC_BUffer_Clean            = 0,  /* Indicate Buffer is cleaned and No date for transimitting, And shall not process*/
  MC_BUffer_InQueue          = 1,  /* Indicate Buffer is updated and waitting to be transmitted*/
  MC_BUffer_FirstInProgress  = 2,  /* Indicate Buffer transimatation is progress for first time.*/
  MC_BUffer_Played           = 3   /* Indicate buffer is fully transmitted atleast one time. And not in loop, to stop you may required to clean.*/
};


/*
===========================================================================
===========================================================================
          Public functions related to Morse code
===========================================================================
==========================================================================
*/


/* ************************************************************************
 * Function to Init all required back ground process for Morse code
 * *************************************************************************/
extern void Morse_Code_Init(void);

/* ************************************************************************
 * Function to Sent the morse code to the respective buffer.
 * *************************************************************************/
extern void Morse_Code_Sent(const char *InputString,Morse_Code_Out_BUffer DestBuffer);


/* ************************************************************************
 * Function to Clear the morse code of the respective buffer.
 * Shall not stop if requested one is on going. shall stop once its completed.
 * *************************************************************************/
extern void Morse_Code_Clear(Morse_Code_Out_BUffer DestBuffer);


/* ************************************************************************
 * Function to get curent status of the processing.
 * *************************************************************************/
extern Morse_Code_Buffer_Status Morse_Code_GetStatus(Morse_Code_Out_BUffer DestBuffer);




#endif /* End of Asynchronous_Morse_Code_Generator_H*/
