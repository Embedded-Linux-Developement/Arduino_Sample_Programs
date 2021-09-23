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
  BackGround_Queue_FillInProgres      = F2,    /* Its a state where Queue filling is in progress and or started. */
  BackGround_Queue_New                = 20,    /* Its a state where Queue is just updated */
  BackGround_Queue_InProgress         = 22,    /* Its a state where buffer in Queue printing is in-progress.*/
  BackGround_Queue_PrintCompleted     = 44,    /* Its a state where where buffer printing is completed, And can reuse again..*/
  BackGround_Queue_Empty              = 88,    /* Its a state where Queue is empty and ready to take new assignment.*/
};

/* Data type to define the data type used for pointing buffer Address*/
typedef unsigned short BufferAddType;



/* Following is the Structure to store the Queue Informatations*/
typedef struct BackGround_Queue_Table_Tag {
   BufferAddType               BUfferStartAdd;       /* Represent the start add of the buffer, Its Virtual adders start from 0 to "Max_BackGround_Buffer_Reserved" */
   BufferAddType               BUfferSize;           /* Represent the sizes of the buffer including \0. and can have maximum size of "Max_Debug_Buffer" */
   BackGround_Queue_Status     Queue_Status;         /* Represent status if the Queue. */     
}BackGround_Queue_Table_Type;



/*******************************************************************************
 *  Variables and Constense
*******************************************************************************/
/* Max buffer relocatated for debug info in Serial window.*/
static char buffer[Max_Debug_Buffer];
static char Timming_Buffer[Max_Debug_Time_Buffer];

#if (Enable_Background_Print_Support == True)

/* Reserve the Buffer for of the Backup.*/
static char BackGround_Buffer[Max_BackGround_Buffer_Reserved];
/* Queue Table for store the Queue status.*/
BackGround_Queue_Table_Type BackGround_Queue[Max_BackGround_Buffer_Queue];

/* Variable to indicate the starting point of the buffer.*/
BufferAddType  BackGround_Queue_Start_Pointer;

/* Variable to indicate the End point of the buffer. 
   It also keep the info on which is the next Queue to be used.
   The pointer indicate the Queue to be used on next cycle.*/
BufferAddType  BackGround_Queue_End_Pointer;

/* Variable to indicate start of Next Free Bugger address.*/
BufferAddType  BackGround_Buffer_Start_Address;

/* Variable to indicate available total free buffer sizes.*/
BufferAddType  BackGround_Total_Free_Buffer_Sizes;


/* Mutex variable to access the buffers and status.*/
portMUX_TYPE BackGround_Debug_Trace_Mutex = portMUX_INITIALIZER_UNLOCKED; 
/* Handle for BackGround_Debug_Trace_Task */
TaskHandle_t BackGround_Debug_Trace_Task_Handle;



#endif /* End of Enable_Background_Print_Support == True*/

/*******************************************************************************
 *  Functions Extern deceleration
*******************************************************************************/
/* If back ground debug is Enabled.*/
#if (Enable_Background_Print_Support == True)

static void BackGround_Debug_Trace_Task( void * pvParameters );

#endif /* End of Enable_Background_Print_Support == True*/

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
#if (Enable_Background_Print_Support == True)

/* ************************************************************************
 * Function to Init all required back ground process for Debug Trace in Background
 * *************************************************************************/
static void Process_BackGround_Debug_Trace(void)
{




}


/* ************************************************************************
 * Function to free required amound of buffer if not available
 * *************************************************************************/
static void Free_Required_BackgroundBuffer(BufferAddType RequiredBufferSizes)
{

  /* Enter in to Critical Section*/
  portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

  /* Check if Free Buffer is available.*/
  if (RequiredBufferSizes > BackGround_Total_Free_Buffer_Sizes)
  {
    /* Free required Buffer */
  }

  /* Exit from Critical Section. */
  portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);
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

#endif /* End of Enable_Background_Print_Support == True*/

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
#if ((Enable_Debug_Support == True) && (Enable_Background_Print_Support == True))
  unsigned short Index;

#endif

  /* For tracing the the function call.*/
  Trace_Function_Call();

  /* Do operation only if debug support is ON*/
#if (Enable_Debug_Support == True)
  /* Start the Serial Port*/
  Serial.begin(Serial_BR_Rate);

#if (Enable_Background_Print_Support == True)

  /* Enter in to Critical Section*/
  portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

  /* Init the Queue Table*/
  for (Index = 0; Index < Max_BackGround_Buffer_Queue; Index++)
  {
    BackGround_Queue.BUfferStartAdd = 0;
    BackGround_Queue.BUfferSize = 0;
    BackGround_Queue->Queue_Status = BackGround_Queue_Empty;
  }

/* Initate Other global variables required.*/
BackGround_Queue_Start_Pointer = 0;
BackGround_Queue_End_Pointer   = 0;
BackGround_Buffer_Start_Address = 0;
BackGround_Total_Free_Buffer_Sizes = Max_BackGround_Buffer_Reserved;


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




#endif /* End of Enable_Background_Print_Support == True*/

#endif /* End of (Enable_Debug_Support == True)*/
}



/* ************************************************************************
 * Function to print output for debug purpose.
 * ************************************************************************
 * */
Debug_Trace_FunStdRet_Type Debug_Trace(const char *fmt, ...)
{

Debug_Trace_FunStdRet_Type ReturnValue = Debug_Trace_OK;

  /* Do operation only if debug support is ON*/
#if (Enable_Debug_Support == True)
  int Ret_Var;
  va_list args;

/* If back ground debug is Enabled.*/
#if (Enable_Background_Print_Support == True)
unsigned long TotalString_Len;



#endif /* End of Enable_Background_Print_Support == True*/

  va_start(args, fmt);
  Ret_Var = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  /* Get Timming related Info*/
  sprintf(Timming_Buffer, "%011ld: ", millis());

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
#if (Enable_Background_Print_Support == True)

/* Calculate total memory required for the buffering*/
TotalString_Len = strlen(Timming_Buffer) + strlen(buffer) + 1;

/* Enter in to Critical Section*/
portENTER_CRITICAL(&BackGround_Debug_Trace_Mutex);

/* Check if Free Buffer is available.*/
if(TotalString_Len > BackGround_Total_Free_Buffer_Sizes)
{
  /* Free required Buffer */

}


/* Exit from Critical Section. */
portEXIT_CRITICAL(&BackGround_Debug_Trace_Mutex);


/* If back ground debug is Dissabled.*/
#else
  /* Print Timing related inform*/
  Serial.write(Timming_Buffer);
  /* Print the  out to Uart Serial.*/
  (void)Serial.write(buffer);

  Serial.write("\n");

#endif /* End of Enable_Background_Print_Support == True*/


#endif /* End of (Enable_Debug_Support == True)*/

/* Return the return value.*/
return(ReturnValue);

}











