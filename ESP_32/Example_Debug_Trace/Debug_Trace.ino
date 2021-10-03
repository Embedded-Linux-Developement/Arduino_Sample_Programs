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


/*******************************************************************************
 *  External Includes
*******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Debug_Trace.h"


/*******************************************************************************
 *  Data type reserved for support trace functionality.
*******************************************************************************/

/* Following different states of a buffer Queue based on the processing.*/
enum  BackGround_Queue_Status{
  BackGround_Queue_FillInProgres      = 0x92,    /* Its a state where Queue filling is in progress and or started. */
  BackGround_Queue_New                = 0x20,    /* Its a state where Queue is just updated */
  BackGround_Queue_PrintInProgress    = 0x22,    /* Its a state where buffer in Queue printing is in-progress.*/
  BackGround_Queue_PrintCompleted     = 0x44,    /* Its a state where where buffer printing is completed, And can reuse again..*/
  BackGround_Queue_Empty              = 0x88,    /* Its a state where Queue is empty and ready to take new assignment.*/
};



/* Following is the Structure to store the Queue Informatations*/
typedef struct BackGround_Queue_Table_Tag {
   BufferAddType               BUfferStartAdd;               /* Represent the start add of the buffer, Its Virtual adders start from 0 to "Max_BackGround_Buffer_Reserved" */
   BufferAddType               BUfferSize;                   /* Represent the sizes of the buffer including \0. and can have maximum size of "Max_Debug_Buffer" */
   BackGround_Queue_Status     Queue_Status;                 /* Represent status if the Queue. */   
  /* Check if Queue Timeout is supported.*/ 
#if ((BackGround_Debug_Trace_TimeOut > 0) && (Enable_Background_WaitForBuffer == Config_ON) )
   unsigned long               Queue_Processing_Start_Time;  /* Represent the time at which "BackGround_Queue_FillInProgres" state is changes, Can use to applat timeout id required.*/   
#endif

}BackGround_Queue_Table_Type;

/*******************************************************************************
 *  Macros
*******************************************************************************/
#define Invalid_Queue_Index (Max_BackGround_Buffer_Queue + 1)

/* Memory required or storing the Time stamp info, Please do not change same its not configurable, 
    if doing so please make sure to update the code accordingly.*/
#define Max_Debug_Time_Buffer 25

/* Memory required or storing time out error string for further processing,*/
#define TimeOutError_Storage_Buffer_Sizes 103

/* Memory required or storing Overrun / Data lost error string for further processing,*/
#define OverRunError_Storage_Buffer_Sizes 104


/* Generic Macro function to increment Cyclic Queue or Buffers 
  VariableMaxLimit:- Shall considered that Index from 0 to VariableMaxLimit-1 */
#define Increment_Cyclic_Variable(Variable_to_Increment, VariableMaxLimit) {if(++Variable_to_Increment >= VariableMaxLimit){Variable_to_Increment = 0;}}

/* Generic Macro function to Decrement Cyclic Queue or Buffers 
  VariableMaxLimit:- Shall considered that Index from 0 to VariableMaxLimit-1 */
#define Decrement_Cyclic_Variable(Variable_to_Increment, VariableMaxLimit) ((Variable_to_Increment == 0)?(Variable_to_Increment = VariableMaxLimit - 1):((Variable_to_Increment >= VariableMaxLimit)?(Variable_to_Increment = VariableMaxLimit - 1):(--Variable_to_Increment)))


/*******************************************************************************
 *  Variables and Constense
*******************************************************************************/
/* Max buffer relocatated for debug info in Serial window.*/
static char buffer[Max_Debug_Buffer];

/* Check if time stamping is enabled.*/
#if (Enable_DebugTraceTimeStamping == Config_ON)
static char Timming_Buffer[Max_Debug_Time_Buffer];
#endif /* End of (Enable_DebugTraceTimeStamping == Config_ON)*/

#if (Enable_Background_Print_Support == Config_ON)

/* Reserve the Buffer for of the Backup. Additional 2 byte for terminatation in case of segmented cyclic buffer loop.*/
static char BackGround_Buffer[Max_BackGround_Buffer_Reserved + 2];
/* Queue Table for store the Queue status.*/
BackGround_Queue_Table_Type BackGround_Queue[Max_BackGround_Buffer_Queue];

/* If additional bufferis required*/
#if ( (Enable_IncludeLatestErrorTrace == Config_ON) && (Enable_Error_Reporting == Config_ON))

  /* Check wheather time out is Supported.*/
#if (BackGround_Debug_Trace_TimeOut > 0) 

/* Global buffer for storting timeout error*/
static char TimeOutError_Storage_Buffer[TimeOutError_Storage_Buffer_Sizes];

#endif /* End of (BackGround_Debug_Trace_TimeOut > 0)  */

/* Global buffer for storting Overrun or datalost error*/
static char OverRunError_Storage_Buffer[OverRunError_Storage_Buffer_Sizes];

#endif /* End of ( (Enable_IncludeLatestErrorTrace == Config_ON) && (Enable_Error_Reporting == Config_ON))*/

/* Variable to indicate the starting point of the buffer.*/
BufferAddType  BackGround_Queue_Start_Pointer;

/* Variable to indicate the End point of the buffer. 
   It also keep the info on which is the next Queue to be used.
   The pointer indicate the Queue to be used on next cycle.
   This may also contain Valied Queue, So same also needs to consider based on status.*/
BufferAddType  BackGround_Queue_End_Pointer;

/* Variable to indicate start of Next Free Bugger address.*/
BufferAddType  BackGround_Buffer_Start_Address;

/* Variable to indicate available total free buffer sizes.*/
BufferAddType  BackGround_Total_Free_Buffer_Sizes;

/* Variable Indicating Trace debug data is lost*/
BufferAddType Total_Buffer_Data_lost;

/* Mutex variable to access the buffers and status.*/
portMUX_TYPE BackGround_Debug_Trace_Mutex = portMUX_INITIALIZER_UNLOCKED; 

/* Handle for BackGround_Debug_Trace_Task */
TaskHandle_t BackGround_Debug_Trace_Task_Handle;

/* Check wheather time out is Supported.*/
#if (BackGround_Debug_Trace_TimeOut > 0) 
/* Variable to store the Timeout error count.*/
BufferAddType Queue_TimeOut_Detected;
#endif /* End of (BackGround_Debug_Trace_TimeOut > 0)  */

#endif /* End of Enable_Background_Print_Support == Config_ON*/

/*******************************************************************************
 *  Functions Extern deceleration
*******************************************************************************/
/* If back ground debug is Enabled.*/
#if (Enable_Background_Print_Support == Config_ON)

extern void BackGround_Debug_Trace_Task( void * pvParameters );

static BufferAddType Accrue_Required_BackgroundQueue(BufferAddType RequiredBufferSizes);
static void Report_Buffer_Data_Lost(void);

static unsigned long Trace_QueueGet_Time_Elapse(unsigned long Reference_Time);

#endif /* End of Enable_Background_Print_Support == Config_ON*/

/*******************************************************************************
 *  Class Objects.
*******************************************************************************/

/*
===========================================================================
===========================================================================
          Privite functions related to Control system System
===========================================================================
==========================================================================

/* If back ground debug is Enabled.*/
#if (Enable_Background_Print_Support == Config_ON)


/* ************************************************************************
 * Function to get time difference with respect to current time, 
 *    Shall consider over flow case also.
 * ************************************************************************
 * */
static unsigned long Trace_QueueGet_Time_Elapse(unsigned long Reference_Time)
{
  /* Variable to store the difference in time.*/
  unsigned long Delta_Time = 0;
  unsigned long Current_Time = 0;

  /*Get current time*/
  Current_Time = millis();

  /* Check if overflow is happened. In case of overflow current value shall be smaller than the Reference time*/
  if (Current_Time < Reference_Time)
  {
    Delta_Time = Current_Time + (0xFFFFFFFF - Reference_Time);
  }
  else /* No over flow detected*/
  {
    Delta_Time = Current_Time - Reference_Time;
  }

  /* Return the difference*/
  return (Delta_Time);
}



/* ************************************************************************
 * Function to Init all required back ground process for Debug Trace in Background
 * *************************************************************************/
static void Process_BackGround_Debug_Trace(void)
{
#if (Enable_Error_Reporting == Config_ON)
  /* Local variable to store the Time out error message.*/
/* Check wheather time out is Supported.*/
#if (BackGround_Debug_Trace_TimeOut > 0) 
  BufferAddType Local_TimeOut_Cnt = 0;
#endif /* End of (BackGround_Debug_Trace_TimeOut > 0)  */

BufferAddType Local_Overflow_Cnt = 0;



  /* Array for printing value*/
  char TempBuffer[7];

#endif /* End of (Enable_Error_Reporting == Config_ON)*/

/* Variable to get the Queue to be processed.*/
BufferAddType LoopIndex;
BufferAddType CurrentQueueToBeProcessed;
/* Variable to check if queue in end point also checked, If so on Next Iteration needs to exit the loop*/
unsigned char EndQueue_Checked;

/* Check if error reporting is enabled, of So get the information.*/
#if (Enable_Error_Reporting == Config_ON)
  /* Enter in to Critical Section*/
  portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

  /* Get data lost count.*/
  Local_Overflow_Cnt = Total_Buffer_Data_lost;
  /* Clear data lost count.*/
  Total_Buffer_Data_lost = 0;

  /* Check wheather time out is Supported.*/
#if (BackGround_Debug_Trace_TimeOut > 0) 

  /* Get Time count.*/
  Local_TimeOut_Cnt = Queue_TimeOut_Detected;
  /* Clear data lost count.*/
  Queue_TimeOut_Detected = 0;

#endif /* End of (BackGround_Debug_Trace_TimeOut > 0)  */

/* If Error required to be stored in a buffer*/
#if (Enable_IncludeLatestErrorTrace == Config_ON)

  /* Check wheather time out is Supported.*/
#if (BackGround_Debug_Trace_TimeOut > 0) 

  /* Check if Timeout error present.*/
  if (Local_TimeOut_Cnt != 0)
  {
     sprintf(TimeOutError_Storage_Buffer,"%s!![Failter Error], Queue Timeout detected for %04d Times..%s",ErrorTraceSerial_StartCharactor, Local_TimeOut_Cnt,ErrorTraceSerial_TerminatationCharactor);
  }

#endif /* End of (BackGround_Debug_Trace_TimeOut > 0)  */

  /* Check if Timeout error present.*/
  if (Local_Overflow_Cnt != 0)
  {
     sprintf(OverRunError_Storage_Buffer,"%s!![Failter Error], Queue Overflow detected for %04d Times..%s",ErrorTraceSerial_StartCharactor, Local_Overflow_Cnt,ErrorTraceSerial_TerminatationCharactor);
  }



#endif /* End of (Enable_IncludeLatestErrorTrace == Config_ON)*/



  /* Exit from Critical Section. */
  portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);

   /* Check if serial printing is supported.*/
#if (Enable_DebugTrace_Via_SerialPort == Config_ON)

  /* Check wheather time out is Supported.*/
#if (BackGround_Debug_Trace_TimeOut > 0) 

  /* Check if Timeout error present.*/
  if (Local_TimeOut_Cnt != 0)
  {

/* If Error string is already stored in a buffer*/
#if (Enable_IncludeLatestErrorTrace == Config_ON)
 
  /* Print already populated string*/
   DebugTraceSerial_Print_Interface(TimeOutError_Storage_Buffer);

#else /* If error Not stored in any buffer.*/
    DebugTraceSerial_Print_Interface(ErrorTraceSerial_StartCharactor);
    DebugTraceSerial_Print_Interface("!![Failter Error], Queue Timeout detected for ");
    sprintf(TempBuffer,"%04d",Local_TimeOut_Cnt);
    DebugTraceSerial_Print_Interface(TempBuffer);
    DebugTraceSerial_Print_Interface(" Times..");
    DebugTraceSerial_Print_Interface(ErrorTraceSerial_TerminatationCharactor);

#endif /* End of (Enable_IncludeLatestErrorTrace == Config_ON)*/

  }

#endif /* End of (BackGround_Debug_Trace_TimeOut > 0)  */

  /* Check if Timeout error present.*/
  if (Local_Overflow_Cnt != 0)
  {
    
/* If Error string is already stored in a buffer*/
#if (Enable_IncludeLatestErrorTrace == Config_ON)
 
  /* Print already populated string*/
   DebugTraceSerial_Print_Interface(OverRunError_Storage_Buffer);

#else /* If error Not stored in any buffer.*/

    DebugTraceSerial_Print_Interface(ErrorTraceSerial_StartCharactor);
    DebugTraceSerial_Print_Interface("!![Failter Error], Queue Overflow detected for ");
    sprintf(TempBuffer,"%04d",Local_Overflow_Cnt);
    DebugTraceSerial_Print_Interface(TempBuffer);
    DebugTraceSerial_Print_Interface(" Times..");
    DebugTraceSerial_Print_Interface(ErrorTraceSerial_TerminatationCharactor);

#endif /* End of (Enable_IncludeLatestErrorTrace == Config_ON)*/

  }

  #endif /* End of (Enable_DebugTrace_Via_SerialPort == Config_ON)*/

#endif /* End of (Enable_Error_Reporting == Config_ON)*/

/* Set a init value to local variable */
 CurrentQueueToBeProcessed = Invalid_Queue_Index;
/* Clear end point detection*/
  EndQueue_Checked = false;

 /* Enter in to Critical Section*/
 portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);
 /* Loop and Identify the Queue to be processed in this cycle.*/
 for (LoopIndex = BackGround_Queue_Start_Pointer; LoopIndex != Invalid_Queue_Index; /* Increment shall be cyclic, So handled saperately*/ )
 {
    /* Check if the Queue is ready to print*/
    if((BackGround_Queue[LoopIndex].Queue_Status == BackGround_Queue_New) || 
       (BackGround_Queue[LoopIndex].Queue_Status == BackGround_Queue_PrintInProgress) )
       {
         /* Set this as Queue to be processed.*/
         CurrentQueueToBeProcessed = LoopIndex;

         /* State status indicating that Printing is in progress.*/
         BackGround_Queue[CurrentQueueToBeProcessed].Queue_Status = BackGround_Queue_PrintInProgress;

         /* Exit the loop*/
         LoopIndex = Invalid_Queue_Index;
       }

       /* Increment Loop index, id still did not found any.*/
       if (LoopIndex != Invalid_Queue_Index)
       {
         /* Increment the Index. */
         LoopIndex++;

         /* Check wheather its time to make it cyclic. */
         if (LoopIndex >= Max_BackGround_Buffer_Queue)
         {
           /* Reset the Index from start*/
           LoopIndex = 0;
         }

         /* Judgement for End Queue also checked.*/
         /* If end point already detected, Exit this time.*/
         if (EndQueue_Checked == true)
         {
           /* Exit this for loop*/
           LoopIndex = Invalid_Queue_Index;
         }
         /* If current queue is matching with the end budder*/
         else if (LoopIndex == BackGround_Queue_End_Pointer)
         {
           /* Allow to go for one more iteration.*/
           EndQueue_Checked = true;
         }
         else
         {
           /* Do Nothing. */
         }
       }/* Emd of if (LoopIndex != Invalid_Queue_Index)*/
 }
 /* Exit from Critical Section. */
 portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);

 /* Check if the Queue is available for printing*/
 if (CurrentQueueToBeProcessed != Invalid_Queue_Index)
 {

   /* Check if serial printing is supported.*/
#if (Enable_DebugTrace_Via_SerialPort == Config_ON)

   /* Transmit Starting character based on the configuration*/
   (void)DebugTraceSerial_Print_Interface(DebugTraceSerial_StartCharactor);

   /*Initate the printing of the buffer*/
   (void)DebugTraceSerial_Print_Interface((char *)(&BackGround_Buffer[BackGround_Queue[CurrentQueueToBeProcessed].BUfferStartAdd]));

   /* Check of buffer is on edge*/
   if ((BackGround_Queue[CurrentQueueToBeProcessed].BUfferSize + BackGround_Queue[CurrentQueueToBeProcessed].BUfferStartAdd) > Max_BackGround_Buffer_Reserved)
   {
     /*Initate transmutation of remaining buffer*/
     (void)DebugTraceSerial_Print_Interface((char *)(BackGround_Buffer));
   }

   /* Print Terminatation charactor.*/
   (void)DebugTraceSerial_Print_Interface(DebugTraceSerial_TerminatationCharactor);

   /* Enter in to Critical Section*/
   portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

   /* Set the state that buffer processing is completed.*/
   BackGround_Queue[CurrentQueueToBeProcessed].Queue_Status = BackGround_Queue_PrintCompleted;

   /* Exit from Critical Section. */
   portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);
#endif /* End of (Enable_DebugTrace_Via_SerialPort == Config_ON)*/
 }
}

/* ************************************************************************
 * Function to update the Buffer data lost detected.
 * *************************************************************************/
static void Report_Buffer_Data_Lost(void)
{

  /* Enter in to Critical Section*/
  portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);
   
   /* Increment the Data lost.*/
   Total_Buffer_Data_lost++;

  /* Exit from Critical Section. */
  portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);
}

/* ************************************************************************
 * Function to free required amound of buffer if not available and 
   prepate the Queue for storing the buffer.
   Input:- Required Buffer sizes, including byte for '\0'
   Output ( Return): Queue index in which message to be stored. 
                     shall Return Invalid_Queue_Index, if Failed to accrue.
 * *************************************************************************/
static BufferAddType Accrue_Required_BackgroundQueue(BufferAddType RequiredBufferSizes)
{
  /* Variable to store return Queue Index*/
  BufferAddType Return_Queue_Index = Invalid_Queue_Index;
  /* Variable for local loop Index*/
  BufferAddType Local_Loop_Index;
  /* Variable to check if queue in end point also checked, If so on Next Iteration needs to exit the loop*/
  unsigned char EndQueue_Checked;

  /* Check if any Queue is readely available, else waite until same is available.*/
#if (Enable_Background_WaitForBuffer == Config_ON) /* If needs to waite.*/

  /* If queue is already select, Do not enter this loop again, on next iteration onwards.*/
  while (Return_Queue_Index == Invalid_Queue_Index)
  {
#endif /* End of Enable_Background_WaitForBuffer == Config_ON */

    /* Enter in to Critical Section*/
    portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

    /* Check if current pointing Queue is free to use.*/
    if ((BackGround_Queue[BackGround_Queue_End_Pointer].Queue_Status == BackGround_Queue_Empty) ||
        (BackGround_Queue[BackGround_Queue_End_Pointer].Queue_Status == BackGround_Queue_PrintCompleted))
    {
      /* Select the current Queue */
      Return_Queue_Index = BackGround_Queue_End_Pointer;
      /* Block the Queue, such that other cannot Access this Queue.*/
      BackGround_Queue[Return_Queue_Index].Queue_Status = BackGround_Queue_FillInProgres;
     
     /* Clear the memory allocation to avoid in miss calculation.*/
      BackGround_Queue[Return_Queue_Index].BUfferSize = 0;
      BackGround_Queue[Return_Queue_Index].BUfferStartAdd = 0;

      /* Check wheather time out is Supported, If Yes update requested time.*/
#if ((BackGround_Debug_Trace_TimeOut > 0) && (Enable_Background_WaitForBuffer == Config_ON))

      /* Store the time at which we reserved the Queue.*/
      BackGround_Queue[Return_Queue_Index].Queue_Processing_Start_Time = millis();

#endif /* End of ((BackGround_Debug_Trace_TimeOut > 0) && (Enable_Background_WaitForBuffer == Config_ON) )*/

      /* Increment Queue for Next Cycle.*/
      BackGround_Queue_End_Pointer++;
      /* Check wheather its needs to cycle, If so make it overflow. */
      if (BackGround_Queue_End_Pointer >= Max_BackGround_Buffer_Queue)
      {
        /* Reset the End point*/
        BackGround_Queue_End_Pointer = 0;
      }

      /* Check wheather End Queue is colidded with the Start point, If so Shift the start point.*/
      if (BackGround_Queue_End_Pointer == BackGround_Queue_Start_Pointer)
      {

        /* Move the Start point one point forward.*/
        BackGround_Queue_Start_Pointer++;
        /* Check wheather its needs to cycle, If so make it overflow. */
        if (BackGround_Queue_Start_Pointer >= Max_BackGround_Buffer_Queue)
        {
          /* Reset the End point*/
          BackGround_Queue_Start_Pointer = 0;
        }
      }
    }
    else /* No Free Queue is available.*/
    {

      /* Check wheather time out is Supported, If Yes check for time out and take necessary action.*/
#if ((BackGround_Debug_Trace_TimeOut > 0) && (Enable_Background_WaitForBuffer == Config_ON))

      /* Check wheather timeout time elapsed, If so Change the state of the Queue, Such that on next cycle it shall be consumed.*/
      if (Trace_QueueGet_Time_Elapse(BackGround_Queue[BackGround_Queue_End_Pointer].Queue_Processing_Start_Time) > BackGround_Debug_Trace_TimeOut)
      {
        /* Change the state of the current Queue to empty*/
        BackGround_Queue[BackGround_Queue_End_Pointer].Queue_Status = BackGround_Queue_Empty;

        /* Take back / Free the memory allocatated to this Queue. */
        BackGround_Total_Free_Buffer_Sizes += BackGround_Queue[BackGround_Queue_End_Pointer].BUfferSize;
        BackGround_Queue[BackGround_Queue_End_Pointer].BUfferSize = 0;
        BackGround_Queue[BackGround_Queue_End_Pointer].BUfferStartAdd = 0;

        /* Increase error counter, for Timeout.*/
        Queue_TimeOut_Detected++;
      }
      else
      {
        /* Do nothing.*/
      }
#endif /* End of ((BackGround_Debug_Trace_TimeOut > 0) && (Enable_Background_WaitForBuffer == Config_ON) )*/
    }

    /* Exit from Critical Section. */
    portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);

#if (Enable_Background_WaitForBuffer == Config_ON) /* If needs to waite.*/

    /* Add delay equivelent to processing task period for Background task.*/
    /*If still Queue did not found */
    if (Return_Queue_Index == Invalid_Queue_Index)
    {
      /* Waite for waitting period if buffer is full.*/
      vTaskDelay(BackGround_Each_cycleWaittingDelay / portTICK_PERIOD_MS);
    }

  } /* End of while (Return_Queue_Index == Invalid_Queue_Index) */

#endif /* End of Enable_Background_WaitForBuffer == Config_ON */

/*---------------------------------------------------------------------------------------
               End if Queue Searching and Start of Buffer searching
-----------------------------------------------------------------------------------------*/

  /* Check if any queue is allocated, If waitting is dissabled then this check is mandatory, 
    But keeping it in both case to avoid any mistack/currotion.*/
  if (Return_Queue_Index != Invalid_Queue_Index)
  {

    /* Clear allocated buffer information if any,*/
    BackGround_Queue[Return_Queue_Index].BUfferStartAdd = 0;
    BackGround_Queue[Return_Queue_Index].BUfferSize = 0;

    /* Check if required amound of buffer is available, else waite until same is available. Based on the configuration*/
#if (Enable_Background_WaitForBuffer == Config_ON) /* If needs to waite.*/
    do
    {
#endif /* End of Enable_Background_WaitForBuffer == Config_ON */

      /* Clear end point detection*/
      EndQueue_Checked = false;

      /* Enter in to Critical Section*/
      portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

      /* Check if Free Buffer is available.*/
      if (RequiredBufferSizes <= BackGround_Total_Free_Buffer_Sizes)
      {
        /*Allocate Remaining free memory to the Queue and update its paramaters.*/
        BackGround_Queue[Return_Queue_Index].BUfferStartAdd = BackGround_Buffer_Start_Address;
        BackGround_Queue[Return_Queue_Index].BUfferSize = RequiredBufferSizes;

        /* Recalculate the Next Buffer start address in cyclic passion.*/
        if((BackGround_Buffer_Start_Address + RequiredBufferSizes) >= Max_BackGround_Buffer_Reserved)
        {
          /* Calculate new starting addres*/
          BackGround_Buffer_Start_Address = (BackGround_Buffer_Start_Address + RequiredBufferSizes) - Max_BackGround_Buffer_Reserved;
        }
        else /* No complicatation Simple addition in calculating next address*/
        {
          BackGround_Buffer_Start_Address += RequiredBufferSizes;
        }

        /* Re calculate the remaining buffer sizes, No additional check required because same is already did. */
        BackGround_Total_Free_Buffer_Sizes = (BackGround_Total_Free_Buffer_Sizes - RequiredBufferSizes);

      }
      else /* Free memory Not Available, So make it available*/
      {

        /* Loop until all available and possible memoryes are freed.*/
        for (Local_Loop_Index = BackGround_Queue_Start_Pointer;
             ((Local_Loop_Index != Invalid_Queue_Index) &&               /* If any over flow condation detected*/
              (RequiredBufferSizes > BackGround_Total_Free_Buffer_Sizes) /* If get required memory*/
             );
             /* No Increment operatation here because its a cyclic*/)
        {

          /* Check If Current Queue is not free for scavenging its used memory.*/
          if ((BackGround_Queue[Local_Loop_Index].Queue_Status == BackGround_Queue_Empty) ||
              (BackGround_Queue[Local_Loop_Index].Queue_Status == BackGround_Queue_PrintCompleted))
          {
            /* Free the buffer */
            /* Change the state of the current Queue to empty*/
            BackGround_Queue[Local_Loop_Index].Queue_Status = BackGround_Queue_Empty;

            /* Take back / Free the memory allocatated to this Queue. */
            BackGround_Total_Free_Buffer_Sizes += BackGround_Queue[Local_Loop_Index].BUfferSize;
            BackGround_Queue[Local_Loop_Index].BUfferSize = 0;
            BackGround_Queue[Local_Loop_Index].BUfferStartAdd = 0;

            /* Increment first Queue pointer to next, because current one is made free*/
            BackGround_Queue_Start_Pointer++;
            /* Check for Overflow, if so Restart same in Cyclic Pasion.*/
            if (BackGround_Queue_Start_Pointer >= Max_BackGround_Buffer_Queue)
            {
              /* Reset the End point*/
              BackGround_Queue_Start_Pointer = 0;
            }

          }
          else /* Current Queue is Not ready or in state based on which we can scavenge its memory.*/
          {

            /* Check wheather time out is Supported, If Yes check for time out and take necessary action.*/
#if ((BackGround_Debug_Trace_TimeOut > 0) && (Enable_Background_WaitForBuffer == Config_ON))

            /* Check wheather timeout time elapsed, If so Change the state of the Queue, Such that on next cycle it shall be consumed.*/
            if (Trace_QueueGet_Time_Elapse(BackGround_Queue[Local_Loop_Index].Queue_Processing_Start_Time) > BackGround_Debug_Trace_TimeOut)
            {
              /* Change the state of the current Queue to empty*/
              BackGround_Queue[Local_Loop_Index].Queue_Status = BackGround_Queue_Empty;

              /* Take back / Free the memory allocatated to this Queue. */
              BackGround_Total_Free_Buffer_Sizes += BackGround_Queue[Local_Loop_Index].BUfferSize;
              BackGround_Queue[Local_Loop_Index].BUfferSize = 0;
              BackGround_Queue[Local_Loop_Index].BUfferStartAdd = 0;

              /* Increase error counter, for Timeout.*/
              Queue_TimeOut_Detected++;

              /* Increment first Queue pointer to next, because current one is made free*/
              BackGround_Queue_Start_Pointer++;
              /* Check for Overflow, if so Restart same in Cyclic Pasion.*/
              if (BackGround_Queue_Start_Pointer >= Max_BackGround_Buffer_Queue)
              {
                /* Reset the End point*/
                BackGround_Queue_Start_Pointer = 0;
              }
            }
            else
            {
              /* Terminate current loop as there was no processable open queue is available.*/
              Local_Loop_Index = Invalid_Queue_Index;
            }
#else /* If Timeout not supported, Exit the loop as no free queue is available.*/

            /* Terminate current loop as there was no processable open queue is available.*/
            Local_Loop_Index = Invalid_Queue_Index;

#endif /* End of ((BackGround_Debug_Trace_TimeOut > 0) && (Enable_Background_WaitForBuffer == Config_ON) )*/

          } /* End of else for if to check Queue is empty.*/

          /* Increment to next Queue Pointer, If Already Not concludded to exit.*/
          if ((Local_Loop_Index != Invalid_Queue_Index))
          {

            /* Increment the counter.*/
            Local_Loop_Index++;

            /* Check for Overflow, if so Restart same in Cyclic Pasion.*/
            if (Local_Loop_Index >= Max_BackGround_Buffer_Queue)
            {
              /* Reset the End point*/
              Local_Loop_Index = 0;
            }
            /* Judgement for End Queue also checked.*/
            /* If end point already detected, Exit this time.*/
            if (EndQueue_Checked == true)
            {
              /* Exit this for loop*/
              Local_Loop_Index = Invalid_Queue_Index;
            }
            /* If current queue is matching with the end budder*/
            else if (Local_Loop_Index == BackGround_Queue_End_Pointer)
            {
              EndQueue_Checked = true;
            }
            else
            {
              /* Do Nothing. */
            }

          } /* End of if (Local_Loop_Index != Invalid_Queue_Index)*/
        } /* End of for loop */
      } /* End if else for (RequiredBufferSizes <= BackGround_Total_Free_Buffer_Sizes).*/

      /* Exit from Critical Section. */
      portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);

#if (Enable_Background_WaitForBuffer == Config_ON) /* If needs to waite.*/

      /* Add delay equivelent to processing task period for Background task.*/
      /* If still enough buffer is not availabe. 
          And Do Not add delay if required memory is already freed*/
      if ((RequiredBufferSizes != BackGround_Queue[Return_Queue_Index].BUfferSize) &&
          (RequiredBufferSizes > BackGround_Total_Free_Buffer_Sizes))
      {
        /* Waite for waitting period because buffer is full.*/
        vTaskDelay(BackGround_Each_cycleWaittingDelay / portTICK_PERIOD_MS);
      }

    } while (RequiredBufferSizes != BackGround_Queue[Return_Queue_Index].BUfferSize);
#endif /* End of Enable_Background_WaitForBuffer == Config_ON */

    /* Check if Possible to get the required buffer, else release the Queue.*/
    if (RequiredBufferSizes != BackGround_Queue[Return_Queue_Index].BUfferSize)
    {

      /* Enter in to Critical Section*/
      portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

      /* Free the Queue reserved for Processing*/
      BackGround_Queue[Return_Queue_Index].Queue_Status = BackGround_Queue_Empty;
      BackGround_Queue[Return_Queue_Index].BUfferSize = 0;
      BackGround_Queue[Return_Queue_Index].BUfferStartAdd = 0;
/* Check if Queue Timeout is supported.*/ 
#if ((BackGround_Debug_Trace_TimeOut > 0) && (Enable_Background_WaitForBuffer == Config_ON) )
      BackGround_Queue[Return_Queue_Index].Queue_Processing_Start_Time = 0;
#endif

      /* Pull Queue end back one step.*/
      /* Check wheather its needs to cycle, If so make it overflow. */
      if (BackGround_Queue_End_Pointer != 0U)
      {

        /* Decrement Queue for Next Cycle.*/
        BackGround_Queue_End_Pointer--;
      }
      else
      {
        /* Reset the End point in back ward direction.*/
        BackGround_Queue_End_Pointer = Max_BackGround_Buffer_Queue - 1;
      }

      /* Exit from Critical Section. */
      portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);

      /* Make current selected Queue invalid*/
      Return_Queue_Index = Invalid_Queue_Index;

    } /* End of If (RequiredBufferSizes != BackGround_Queue[Return_Queue_Index].BUfferSize) */

  } /* End of if (Return_Queue_Index != Invalid_Queue_Index)*/

  /* Return the final Queue allocated, if any.*/
  return(Return_Queue_Index);
}

/* ********************************************************************************
 * Task for process the Morse code generation
   Periodicity:-  50ms
   Priority   :-  5 (Low)
 * *********************************************************************************/

void BackGround_Debug_Trace_Task(void *pvParameters)
{
  /* Loop for the task.*/
  for (;;)
  {

    /********************************************************************************
    *  Add Code after this line....
    * ******************************************************************************
   */

    /* Trigger function to do processing in every period based on periodicity*/
    Process_BackGround_Debug_Trace();

    /* Switch task for 50ms */
    vTaskDelay(BackGround_Debug_Trace_Task_Perion_ms / portTICK_PERIOD_MS);
  }
}



#endif /* End of Enable_Background_Print_Support == Config_ON*/

/*
===========================================================================
===========================================================================
                      Public Generic functions
===========================================================================
===========================================================================
*/

/* ************************************************************************
 * Function to Init output serial for debug purpose.
 * ************************************************************************
 * */
void Init_Trace(void)
{
#if ((Enable_Debug_Support == Config_ON) && (Enable_Background_Print_Support == Config_ON))
  unsigned short Index;

#endif


  /* Do operation only if debug support is ON*/
#if (Enable_Debug_Support == Config_ON)

   /* Check if serial printing is supported.*/
#if (Enable_DebugTrace_Via_SerialPort == Config_ON)

  /* Start the Serial Port*/
  DebugTraceSerial_Print_Init(Serial_BR_Rate);

 #endif /* End of (Enable_DebugTrace_Via_SerialPort == Config_ON)*/

#if (Enable_Background_Print_Support == Config_ON)

  /* Enter in to Critical Section*/
  portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

  /* Init the Queue Table*/
  for (Index = 0; Index < Max_BackGround_Buffer_Queue; Index++)
  {
    BackGround_Queue[Index].BUfferStartAdd = 0;
    BackGround_Queue[Index].BUfferSize = 0;
    BackGround_Queue[Index].Queue_Status = BackGround_Queue_Empty;

    /* Check wheather time out is Supported.*/
#if ((BackGround_Debug_Trace_TimeOut > 0) && (Enable_Background_WaitForBuffer == Config_ON) )
   BackGround_Queue[Index].Queue_Processing_Start_Time = 0;
#endif /* End of (BackGround_Debug_Trace_TimeOut > 0)  */
  }

  /* Initate Other global variables required.*/
  BackGround_Queue_Start_Pointer = 0;
  BackGround_Queue_End_Pointer = 0;
  BackGround_Buffer_Start_Address = 0;
  BackGround_Total_Free_Buffer_Sizes = Max_BackGround_Buffer_Reserved;
  Total_Buffer_Data_lost = 0;

  /* Check wheather time out is Supported.*/
#if (BackGround_Debug_Trace_TimeOut > 0) 
/* Clear Error counter.*/
Queue_TimeOut_Detected = 0;

#endif /* End of (BackGround_Debug_Trace_TimeOut > 0)  */


/* If Error required to be stored in a buffer*/
#if (Enable_IncludeLatestErrorTrace == Config_ON)
  /* Check if Timeout error present.*/
  sprintf(OverRunError_Storage_Buffer,"%sNo Queue Overflow detected Yet...%s",ErrorTraceSerial_StartCharactor, ErrorTraceSerial_TerminatationCharactor);


  /* Check wheather time out is Supported.*/
#if (BackGround_Debug_Trace_TimeOut > 0) 
    sprintf(TimeOutError_Storage_Buffer,"%sNo Queue Timeout detected Yet...%s",ErrorTraceSerial_StartCharactor, ErrorTraceSerial_TerminatationCharactor);
#endif /* End of (BackGround_Debug_Trace_TimeOut > 0)  */

#endif /* End of (Enable_IncludeLatestErrorTrace == Config_ON)*/




  /* Exit from Critical Section. */
  portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);

  /* Initate back ground Task for processing.*/
  /* Start the Back ground task for process the Buffer.*/
  /* Initate Low Priority Background Process Monitering Task */
  xTaskCreatePinnedToCore(
      BackGround_Debug_Trace_Task,         /* Task function. */
      "BackGround_Debug_Trace_Task",       /* name of task. */
      1024,                                /* 1K Stack size of task */
      NULL,                                /* parameter of the task */
      BackGround_Debug_Trace_Task_Priority,/* priority of the task, Grater the value Higher the priority.*/
      &BackGround_Debug_Trace_Task_Handle, /* Task handle to keep track of created task */
      1);                                  /* pin task to core 1, Along with loop() function. */




#endif /* End of Enable_Background_Print_Support == Config_ON*/

#endif /* End of (Enable_Debug_Support == Config_ON)*/
}



/* ************************************************************************
 * Function to print output for debug purpose.
 * ************************************************************************
 * */
Debug_Trace_FunStdRet_Type Debug_Trace(const char *fmt, ...)
{

  Debug_Trace_FunStdRet_Type ReturnValue = Debug_Trace_OK;

  /* Do operation only if debug support is ON*/
#if (Enable_Debug_Support == Config_ON)
  int Ret_Var;
  va_list args;

/* If back ground debug is Enabled.*/
#if (Enable_Background_Print_Support == Config_ON)
  BufferAddType TotalString_Len;
  BufferAddType CurrentQueue_Index;

  BufferAddType OverallBufferIndex;
  BufferAddType CurrentProcessingIndex;

  /* store the starting address .*/
  BufferAddType AllocatedBufferStart_Addres;

#endif /* End of Enable_Background_Print_Support == Config_ON*/

  va_start(args, fmt);
  Ret_Var = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

/* Check if time stamping is enabled.*/
#if (Enable_DebugTraceTimeStamping == Config_ON)

  /* Get Timming related Info*/
  sprintf(Timming_Buffer, "%011ld: ", millis());

#endif /* End of (Enable_DebugTraceTimeStamping == Config_ON)*/

  /* If error Detected in processing format*/
  if (Ret_Var < 0)
  {
    sprintf(buffer, "Error in Generating the debug Trace from function vsnprintf().");
  }
  else
  {
    /* Do nothing*/
  }

/* If back ground debug is Enabled.*/
#if (Enable_Background_Print_Support == Config_ON)

/* Check if time stamping is enabled.*/
#if (Enable_DebugTraceTimeStamping == Config_ON)

  /* Calculate total memory required for the buffering*/
  TotalString_Len = strlen(Timming_Buffer) + strlen(buffer) + 1;

#else /* Time stamping not supported.*/

  /* Calculate total memory required for the buffering*/
  TotalString_Len = strlen(buffer) + 1;

#endif /* End of (Enable_DebugTraceTimeStamping == Config_ON)*/


  /* If required memory grater than Max allowed, The ignore rest of the buffer.*/
  if(TotalString_Len > Max_Debug_Buffer)
  {
    /* Add Upper Cap to the buffer.*/
    TotalString_Len = Max_Debug_Buffer;
  }

  /* Get Queue Index for storing the buffer.*/
  CurrentQueue_Index = Accrue_Required_BackgroundQueue(TotalString_Len);

  /* Check if Queue is detected as valied.*/
  if (CurrentQueue_Index != Invalid_Queue_Index)
  {

    /* Check if the Queue is in correct state.*/
    if (BackGround_Queue[CurrentQueue_Index].Queue_Status == BackGround_Queue_FillInProgres)
    {
      /* Get the buffer address*/
      AllocatedBufferStart_Addres = BackGround_Queue[CurrentQueue_Index].BUfferStartAdd;
      /* Set Over all buffer index to zero*/
      OverallBufferIndex = 0;

/* Check if time stamping is enabled.*/
#if (Enable_DebugTraceTimeStamping == Config_ON)

      /* Get String length */
      TotalString_Len = strlen(Timming_Buffer);
      /* Loop for storing the Time buffer.*/
      for (CurrentProcessingIndex = 0; ((CurrentProcessingIndex < TotalString_Len) && (Timming_Buffer[CurrentProcessingIndex] != '\0') && (OverallBufferIndex < BackGround_Queue[CurrentQueue_Index].BUfferSize)); OverallBufferIndex++, CurrentProcessingIndex++)
      {
        /* Store the timming string.*/
        BackGround_Buffer[AllocatedBufferStart_Addres] = Timming_Buffer[CurrentProcessingIndex];

        /* Increase the main buffer index. IN a cyclic buffer manner*/
        AllocatedBufferStart_Addres++;

        /* Check is needs to loop around*/
        if (AllocatedBufferStart_Addres >= Max_BackGround_Buffer_Reserved)
        {
          /* Add terminatation character*/
          BackGround_Buffer[Max_BackGround_Buffer_Reserved] = '\0';
          /* Re-start the buffer add pointer.*/
          AllocatedBufferStart_Addres = 0;
        }
        else
        {
          /* Do nothing. */
        }
      }

#endif /* End of (Enable_DebugTraceTimeStamping == Config_ON)*/

      /* Get String length */
      TotalString_Len = strlen(buffer);
      /* Loop for storing the Time buffer.*/
      for (CurrentProcessingIndex = 0; ((CurrentProcessingIndex < TotalString_Len) && (buffer[CurrentProcessingIndex] != '\0') && (OverallBufferIndex < BackGround_Queue[CurrentQueue_Index].BUfferSize)); OverallBufferIndex++, CurrentProcessingIndex++)
      {
        /* Store the timming string.*/
        BackGround_Buffer[AllocatedBufferStart_Addres] = buffer[CurrentProcessingIndex];

        /* Increase the main buffer index. IN a cyclic buffer manner*/
        AllocatedBufferStart_Addres++;

        /* Check is needs to loop around*/
        if (AllocatedBufferStart_Addres >= Max_BackGround_Buffer_Reserved)
        {
          /* Add terminatation character*/
          BackGround_Buffer[Max_BackGround_Buffer_Reserved] = '\0';
          /* Re-start the buffer add pointer.*/
          AllocatedBufferStart_Addres = 0;
        }
        else
        {
          /* Do nothing. */
        }
      }

      /* Add Terminatation to last buffer Location.*/
      BackGround_Buffer[AllocatedBufferStart_Addres] = '\0';

      /* Enter in to Critical Section*/
      portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

      /* Change the Queue Status New such that Back ground unit can provess and start printing.*/
      BackGround_Queue[CurrentQueue_Index].Queue_Status = BackGround_Queue_New;

      /* Exit from Critical Section. */
      portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);
    }
    /* Queue is not in correct state, So return Neg_State.*/
    else
    {
      /* Set Neg return value*/
      ReturnValue = Debug_Trace_NOT_OK;
      /* Set Data Lost Error Flag*/
      Report_Buffer_Data_Lost();
    }
  }
  /* Failed to accrue the Queue */
  else
  {
    /* Set Neg return value*/
    ReturnValue = Debug_Trace_NOT_OK;
    /* Set Data Lost Error Flag*/
    Report_Buffer_Data_Lost();
  }

/* If back ground debug is Dissabled.*/
#else

/* Check if time stamping is enabled.*/
#if (Enable_DebugTraceTimeStamping == Config_ON)

  /* Print Timing related inform*/
  (void)DebugTraceSerial_Print_Interface(Timming_Buffer);

#endif /* End of (Enable_DebugTraceTimeStamping == Config_ON)*/

  /* Print the  out to Uart Serial.*/
  (void)DebugTraceSerial_Print_Interface(buffer);

  (void)DebugTraceSerial_Print_Interface(DebugTraceSerial_TerminatationCharactor);

#endif /* End of Enable_Background_Print_Support == Config_ON*/

#endif /* End of (Enable_Debug_Support == Config_ON)*/

  /* Return the return value.*/
  return (ReturnValue);
}



/* ************************************************************************
 * Function to populate the buffer stream based on the current available Queues.
 *  1. * InputBufferStream => First input argument shall be the starting address of the buffer to which string shall be populated.
 *  2. user needs to make sure it shall have enough memory to copy the requested memort size, else result in memory overflow. 
 *  3. BufferStreamSize => Second argument is sizes of the required streaming buffer. enough space required considering Null character '\0' and error messages if required.
 *  4. It shall populate latest Queue which can fit within the requested memory. 
 *  5. Please use this function only when its required, because it will block serial printing for populate the string. 
 *  6. Its execution  time shall depends on the buffer sizes requested and total available queue to populate.
 *  7. If requested size if grater than "Max_BackGround_Buffer_Reserved", then shall consider only upto "Max_BackGround_Buffer_Reserved".
 *  8. If request is not enough to store error report ( based on configuration) and if Not possible to includ even a single Queue, then buffer shall be empty.
 * ************************************************************************
 */
void Populate_BufferStream_FromQueue(char *InputBufferStream, BufferAddType BufferStreamSize)
{

  /* If Debug suppport is enabled and Background Queuing / printing is enabled*/
#if ((Enable_Debug_Support == Config_ON) && (Enable_Background_Print_Support == Config_ON))

  /* Local variable to loop index*/
  BufferAddType LoopIndex;
  unsigned char LoopExit_flag;
  /* Local variable to hold calculated buffer*/
  BufferAddType Total_Calculated_QueueSize;
  BufferAddType Current_Consumed_memory;
  /* Local variable to hold total number of reserved byte required for each queue based on start and end delimiter*/
  BufferAddType Byte_Per_Queue;
  /* represent Current Queue starting point.*/
  BufferAddType Local_Current_Queue_Starting_Point;

  /* Calculate Number of byte required per Queue.*/
  Byte_Per_Queue = strlen(DebugBufferStream_StartCharactor) + strlen(DebugBufferStream_TerminatationCharactor);

  /* Set the init value, indicating No Queue is allocated as start.*/
  Local_Current_Queue_Starting_Point = Invalid_Queue_Index;

  /* Init the variable,*/
  Total_Calculated_QueueSize = 0;

  /* Enter in to Critical Section*/
  portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

/* If Error trace needs to consider in final buffer.*/
#if ((Enable_IncludeLatestErrorTrace == Config_ON) && (Enable_Error_Reporting == Config_ON))

  /*Calculate the memory for Pre and Post delimoter for error messages*/
  Total_Calculated_QueueSize += strlen(ErrorDebugBufferStream_Pre_delimoter) +
                                strlen(ErrorDebugBufferStream_Post_delimoter);

  /* Calculate all one time buffer*/
  Total_Calculated_QueueSize += strlen(ErrorDebugBufferStream_StartCharactor) +
                                strlen(OverRunError_Storage_Buffer) +
                                strlen(ErrorDebugBufferStream_TerminatationCharactor);
  /* Check wheather time out is Supported.*/
#if (BackGround_Debug_Trace_TimeOut > 0)

  /* Calculate all one time buffer*/
  Total_Calculated_QueueSize += strlen(ErrorDebugBufferStream_StartCharactor) +
                                strlen(TimeOutError_Storage_Buffer) +
                                strlen(ErrorDebugBufferStream_TerminatationCharactor);
#endif /* End of (BackGround_Debug_Trace_TimeOut > 0) */

#endif /* End of #if ( (Enable_IncludeLatestErrorTrace == Config_ON) && (Enable_Error_Reporting == Config_ON))*/

  /*Calculate the memory for Pre and Post delimoter for Debug Trace messages*/
  Total_Calculated_QueueSize += strlen(DebugBufferStream_Pre_delimoter) +
                                strlen(DebugBufferStream_Post_delimoter);

  /* Check if memory is filled*/
  if (Total_Calculated_QueueSize < BufferStreamSize)
  {

    /* Calculate the starting Queue to be considered based on the requested memory*/
    /* Loop Back from End point backwards to find the starting Queue.*/
    LoopIndex = BackGround_Queue_End_Pointer;

    /* Increment the End point, Such that same shall much once its decrement inside the loop.*/
    Increment_Cyclic_Variable(LoopIndex, Max_BackGround_Buffer_Queue);

    LoopExit_flag = false;

    do
    {
      /* Decrement the index in cyclic pasion.*/
      Decrement_Cyclic_Variable(LoopIndex, Max_BackGround_Buffer_Queue);

      /* Check if Queue is in correct state to consider.*/
      if ((BackGround_Queue[LoopIndex].Queue_Status != BackGround_Queue_FillInProgres) &&
          (BackGround_Queue[LoopIndex].Queue_Status != BackGround_Queue_Empty) &&
          (BackGround_Queue[LoopIndex].BUfferSize != 0)) /* If buffer sizes is Zero, No needs to consider.*/
      {
        /* Check if buffer of this Queue fits in the current buffer to populate.
            BackGround_Queue[LoopIndex].BUfferSize-1, is considered because space for terminatation characer is not required to consider.*/
        if ((Total_Calculated_QueueSize + Byte_Per_Queue + (BackGround_Queue[LoopIndex].BUfferSize - 1)) < BufferStreamSize)
        {
          /* UPdate the Total calculated memory.*/
          Total_Calculated_QueueSize += (Byte_Per_Queue + (BackGround_Queue[LoopIndex].BUfferSize - 1));

          /* Update the starting Queue to current Index,*/
          Local_Current_Queue_Starting_Point = LoopIndex;
        }
        else /* Enough memory not present So exiting the loop*/
        {
          /* Exit the Loop*/
          LoopExit_flag = true;
        }

      } /* End of Queue status and size validatation.*/

    } while ((LoopExit_flag != true) && (LoopIndex != BackGround_Queue_Start_Pointer)); /* Loop till start point is detected.*/

  } /*End of if Total_Calculated_QueueSize < BufferStreamSize*/
  else
  {
    /* DO nothing.*/
  }

  /* Check if atleast on buffer can fit into the Queue, Else return blank string. */
  if ((Local_Current_Queue_Starting_Point != Invalid_Queue_Index) && (Total_Calculated_QueueSize < BufferStreamSize))
  {
    /* Set current consumed memory as zero.*/
    Current_Consumed_memory = 0;

    /* Populate Error codes based on the configuration.*/

/* If Error trace needs to consider in final buffer.*/
#if ((Enable_IncludeLatestErrorTrace == Config_ON) && (Enable_Error_Reporting == Config_ON))

    /* String copy the Error message Pre-delimoter*/
    sprintf(&InputBufferStream[Current_Consumed_memory], "%s", ErrorDebugBufferStream_Pre_delimoter);
    /* Calculate current consumed Buffer*/
    Current_Consumed_memory += strlen(ErrorDebugBufferStream_Pre_delimoter);

    /* String copy the Data Lose Error messages */
    sprintf(&InputBufferStream[Current_Consumed_memory], "%s%s%s", ErrorDebugBufferStream_StartCharactor,
            OverRunError_Storage_Buffer,
            ErrorDebugBufferStream_TerminatationCharactor);
    /* Calculate current consumed Buffer*/
    Current_Consumed_memory += strlen(ErrorDebugBufferStream_StartCharactor) +
                               strlen(OverRunError_Storage_Buffer) +
                               strlen(ErrorDebugBufferStream_TerminatationCharactor);

    /* Check wheather time out is Supported.*/
#if (BackGround_Debug_Trace_TimeOut > 0)

    /* String copy the Time out Error messages, and overright the previous null character '\0'.*/
    sprintf(&InputBufferStream[Current_Consumed_memory], "%s%s%s", ErrorDebugBufferStream_StartCharactor,
            TimeOutError_Storage_Buffer,
            ErrorDebugBufferStream_TerminatationCharactor);

    /* Calculate all one time buffer*/
    Current_Consumed_memory += strlen(ErrorDebugBufferStream_StartCharactor) +
                               strlen(TimeOutError_Storage_Buffer) +
                               strlen(ErrorDebugBufferStream_TerminatationCharactor);
#endif /* End of (BackGround_Debug_Trace_TimeOut > 0) */

    /* String copy the Error message Post-delimoter*/
    sprintf(&InputBufferStream[Current_Consumed_memory], "%s", ErrorDebugBufferStream_Post_delimoter);
    /* Calculate current consumed Buffer*/
    Current_Consumed_memory += strlen(ErrorDebugBufferStream_Post_delimoter);

#endif /* End of #if ( (Enable_IncludeLatestErrorTrace == Config_ON) && (Enable_Error_Reporting == Config_ON))*/

    /* Loop for each Queue buffer from starting point.*/
    /* Set the starting point of the queue based on previousely calculated Queue location.*/
    LoopIndex = Local_Current_Queue_Starting_Point;

    /* Decrement the current starting point, Such that same shall much once its incremented inside the loop.*/
    Decrement_Cyclic_Variable(LoopIndex, Max_BackGround_Buffer_Queue);

    LoopExit_flag = false;

    /* String copy the Debug Trace Buffer / string Pre-delimoter*/
    sprintf(&InputBufferStream[Current_Consumed_memory], "%s", DebugBufferStream_Pre_delimoter);
    /* Calculate current consumed Buffer*/
    Current_Consumed_memory += strlen(DebugBufferStream_Pre_delimoter);

    do
    {
      /* Increment the index in cyclic pasion.*/
      Increment_Cyclic_Variable(LoopIndex, Max_BackGround_Buffer_Queue);

      /* Check if Queue is in correct state to consider.*/
      if ((BackGround_Queue[LoopIndex].Queue_Status != BackGround_Queue_FillInProgres) &&
          (BackGround_Queue[LoopIndex].Queue_Status != BackGround_Queue_Empty) &&
          (BackGround_Queue[LoopIndex].BUfferSize != 0)) /* If buffer sizes is Zero, No needs to consider.*/
      {

        /* If Buffer memory allocation for this queue is on border, or a border case*/
        if ((BackGround_Queue[LoopIndex].BUfferStartAdd + BackGround_Queue[LoopIndex].BUfferSize) > Max_BackGround_Buffer_Reserved)
        {
          /* Copy first and second set of data to the output buffer*/
          sprintf(&InputBufferStream[Current_Consumed_memory], "%s%s%s%s",
                  DebugBufferStream_StartCharactor,
                  &BackGround_Buffer[BackGround_Queue[LoopIndex].BUfferStartAdd],
                  &BackGround_Buffer[0],
                  DebugBufferStream_TerminatationCharactor);
        }
        else /* If buffer is not segmented or on boarder.*/
        {
          /* Copy first and second set of data to the output buffer*/
          sprintf(&InputBufferStream[Current_Consumed_memory], "%s%s%s",
                  DebugBufferStream_StartCharactor,
                  &BackGround_Buffer[BackGround_Queue[LoopIndex].BUfferStartAdd],
                  DebugBufferStream_TerminatationCharactor);
        }

        /* Calculate all one time buffer*/
        Current_Consumed_memory += ((BackGround_Queue[LoopIndex].BUfferSize - 1) + Byte_Per_Queue);

        /* Check if buffer all calculated sizes consumed.*/
        if (Current_Consumed_memory >= Total_Calculated_QueueSize)
        {
          /* Exit the Loop*/
          LoopExit_flag = true;
        }
        else
        {
          /* Do Nothing.*/
        }

      } /* End of Queue status and size validatation.*/
      else
      {
        /* Do nothing.*/
      }

    } while ((LoopExit_flag != true) && (LoopIndex != BackGround_Queue_End_Pointer)); /* Loop till End point is detected.*/

    /* String copy the Debug Trace Buffer / string Post-delimoter*/
    sprintf(&InputBufferStream[Current_Consumed_memory], "%s", DebugBufferStream_Post_delimoter);
    /* Calculate current consumed Buffer*/
    Current_Consumed_memory += strlen(DebugBufferStream_Post_delimoter);

    /* Add null character to the end of the string.*/
    InputBufferStream[(Current_Consumed_memory)] = '\0';
  }
  /* No Queue fit into requested buffer or No trace request is generated yet.*/
  else if (BufferStreamSize >= 1) /* Terminate if atleast one byte is available, else do nothing.*/
  {
    /* Terminate first character itself.*/
    InputBufferStream[0] = '\0';
  }

  /* Exit from Critical Section. */
  portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);

#endif /* End of ( (Enable_Debug_Support == Config_ON) && (Enable_Background_Print_Support == Config_ON))*/
}
