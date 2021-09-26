
/*****************************************************************************************************
Header file  Debug Trace.
Description:- Component to print the debug information in serial in parallel.
                1. If Enabled This Component shall buffer the debug information and print via Serial in back ground.
                2. If Enable shall support to get the all latest debug info in to a single or separate buffer, Can use to print in HTML.
Limitation:- 1. Tested for Esp 32, May required more RAM memory for buffer all debug Info..
             2. Buffer sizes needs to be consider based on what frequency did the debug Info get keep on request. 
                 Because if enough buffer not allocated, then can Ignore old debug info to handle new request.  
Copy Write:- 
Licence:- 
    <Asynchronous Serial Debug Trave>
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
Date  :- 11 Spet 2021
version:- V1.0.1

*******************************************************************************************************/

#ifndef Debug_Trace_H
#define Debug_Trace_H


/* Define Config_ON*/
#ifndef Config_ON
#define Config_ON 0X55
#endif

/* Define Config_OFF*/
#ifndef Config_OFF
#define Config_OFF 0XAA
#endif


/*******************************************************************************
 *  Debug Trace Configuration Start
*******************************************************************************/
/* Set required Serial broad-rate.*/
#define Serial_BR_Rate 115200 /*serial communication at 9600 bits per second*/

/* Macro to enable or disable debug support,
     Config_ON   => Will print debug info over serial terminal.
     Config_OFF  => Shall Not print debug info over serial terminal.
  */
#define Enable_Debug_Support Config_ON

/* Macro to enable or disable tracing of the function calls,
     Config_ON   => Shall Print the function , file and line number from where the Macro Function "Trace_Function_Call()" is being called.
     Config_OFF  => Shall Dissable the Macro Function "Trace_Function_Call()" calls.
  */
#define Enable_Debug_Trace_Function_Calls Config_OFF


/* Macro to enable or disable Background printing, It helps to keep on print in background in NON blocking phasen 
     Config_ON   => Shall support Background printing, in a low priority task.
     Config_OFF  => Background printing shall be dissabled and shall print in Blocking phasen.
  */
#define Enable_Background_Print_Support Config_ON


/* Macro to enable or disable Holding / wait until required buffer or Background queue is free. else shall Ignore new request. 
     Config_ON   => The processing shall wait until required number of Buffer or Queue is get freed. 
     Config_OFF  => If enough buffer or Queue is not available, Then shall Ignore latest request and through error, Indicating New Trace Data lost.
  */
#define Enable_Background_WaitForBuffer Config_ON


/*Reserve Max debug variable sizes as 1KB.*/
#define Max_Debug_Buffer 1024
#define Max_Debug_Time_Buffer 128


/* Macro for define the Max buffer reserved for */
#define Max_BackGround_Buffer_Reserved 6144   /* Indicate Total of 6KB reserved for debug back up buffers.*/
#define Max_BackGround_Buffer_Queue 100       /* Indicate total 100 Queue is allocated for backup buffer.*/

/* To Define the periodicity of the task for processing the background task for printing.*/
#define BackGround_Debug_Trace_Task_Perion_ms 100 
/* To Define the priority of the task, Grater the value grater the priority, SO please try to select one with lowest priority.*/
#define BackGround_Debug_Trace_Task_Priority 0 

/* To Define the waiting delay after unsuccessful attempt, 
   Can consider the smallest intervall befor checking again wheather any Queue is get freed.. 
   This time shall option shall applicable only when Enable_Background_WaitForBuffer = Config_ON.
 */
#define BackGround_Each_cycleWaittingDelay 30 

/* To Define the Queue timeout, Use for cancell or ignore the Queue once timeout is detected. 
     If cannot complete the printing withen mentioned time then Queue content shall be ignored, Only id there is NO free buffer or Queue available. 
    unit in ms
    Support only when Enable_Background_WaitForBuffer = Config_ON
    0 ==> Indicate No support for timeout.
    2000 ==> Indicate 2sec / 2000ms Time out if Free Buffer or Queue is not available.*/
#define BackGround_Debug_Trace_TimeOut  30000

/*-----------------------------------------------------------------------------
 *  Generic Utility Configuration END
-------------------------------------------------------------------------------*/

/*******************************************************************************
 *  Program Specific Macros . Enums / Typedefs.
*******************************************************************************/

/* Following different function return values to define the state.*/
enum  Debug_Trace_FunStdRet_Type{
  Debug_Trace_OK       = 55,    /* Indicate Function processing is OK*/
  Debug_Trace_NOT_OK   = 48,    /* Indicate Function processing is NOT OK*/  
  };

/*******************************************************************************
 *  Macro Functions
*******************************************************************************/

/* If function call tracing is enabled,*/
#if (Enable_Debug_Trace_Function_Calls == Config_ON)
/* Print debug informatations*/
//#define Trace_Function_Call() Debug_Trace("Call Sequence:- In Function %-40s  , @ Line Number %04d , in file %s .", __func__,__LINE__,__FILE__)
/* One without file name*/
#define Trace_Function_Call() Debug_Trace("Call Sequence:- In Function %-40s  , @ Line Number %04d", __func__,__LINE__)

#else /* if (Enable_Debug_Trace_Function_Calls == Config_OFF)*/
/* Do nothing, as same is dissabled.*/
#define Trace_Function_Call() 

#endif /* End of (Enable_Debug_Trace_Function_Calls == Config_OFF)*/


/*******************************************************************************
 *  Public Function Definations Functions
*******************************************************************************/


/* For print debug trace.*/
extern Debug_Trace_FunStdRet_Type Debug_Trace(const char *fmt, ...);

/*Function to Init output serial for debug purpose. */
extern void Init_Trace(void);




#endif /* End of  Debug_Trace_H */
