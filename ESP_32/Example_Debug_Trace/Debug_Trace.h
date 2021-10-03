
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
     1. After Enabling this option if you use Macro function "Trace_Function_Call()" in any place,
         Then same hall help to print function Name and line number, use to check the flow or to debug,
     2. By dissabling / seting to Config_OFF, definatation of macro code remove, No need to remove the "Trace_Function_Call()" function  calls 
  */
#define Enable_Debug_Trace_Function_Calls Config_OFF


/* Macro to enable or disable Background printing, It helps to keep on print in background in NON blocking phasen 
     Config_ON   => Shall support Background printing, in a low priority task.
     Config_OFF  => Background printing shall be dissabled and shall print in forground, Like blocking function.
  */
#define Enable_Background_Print_Support Config_ON


/* Macro to enable or disable Holding / wait until required buffer or Background queue is free. else shall Ignore new request. 
     Config_ON   => The processing shall wait until required number of Buffer or Queue is get freed. 
     Config_OFF  => If enough buffer or Queue is not available, Then shall Ignore latest request and through error, Indicating New Trace Data lost.
  */
#define Enable_Background_WaitForBuffer Config_ON

/* This Macro is to Enable or dissable the printing of error message like timeout or Data lost because of Queue overflow.
     Config_ON   => Shall print error message.
     Config_OFF  => Shall not print error mesage.
     1. Only printing of erro shall be dissabled, Bur stack will still log error in its internal variables and do required process to overcome if possible.
*/
#define Enable_Error_Reporting Config_ON


/* This Macro is to Enable or dissable the printing of time stamping on each request automaticaly.
     Config_ON   => Shall enebale time stamping, shall consume upto 14 Bytes additional.
     Config_OFF  => Shall not print time stamp along with the debug trace. Use full if planning to use transimit any other info like dedicated serial based protocol.
*/
#define Enable_DebugTraceTimeStamping Config_ON

/* This Macro is to define the starting character / string to be transmits along with each trace buffer request, in serial output.
   1. By default "" being used, Indicating No start charactor/ string
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall not use for Buffer streaming.
   5. It shall not use for Error reporting printing.
   6. It shall not affect or consumed in buffer, directly printing at last stage.
*/
#define DebugTraceSerial_StartCharactor ""


/* This Macro is to define the character /strung to be used as each buffer terminatation in serial output.
   1. By default '\n' being used, 
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall not use for Buffer streaming.
   5. It shall not use for Error reporting printing.
   6. It shall not affect or consumed in buffer, directly printing at last stage.
*/
#define DebugTraceSerial_TerminatationCharactor "\n"

/* This Macro is to define the starting character / string to be transmits along with each Error messages, in serial output.
   1. By default "" being used, Indicating No start charactor/ string
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall used for Buffer streaming, If enabled option to include latest error Trace (Enable_IncludeLatestErrorTrace = Config_ON).
   5. It shall not use for Error reporting printing.
   6. It shall not affect or consumed in buffer, directly printing at last stage.
   7. Please do not add more than 15 Character, It may cause buffer overrun in current implementation.
   8. You can add more than 15 characters in such a way 
       total No of characters in (ErrorTraceSerial_StartCharactor + ErrorTraceSerial_TerminatationCharactor) <= 30 characters.
   9. Shall applicable only if error reporting is enabled, Enable_Error_Reporting = Config_ON.
*/
#define ErrorTraceSerial_StartCharactor ""


/* This Macro is to define the character /strung to be used as each buffer terminatation in serial output.
   1. By default '\n' being used, 
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall used for Buffer streaming, If enabled option to include latest error Trace (Enable_IncludeLatestErrorTrace = Config_ON).
   5. It shall not use for Error reporting printing.
   6. It shall not affect or consumed in buffer, directly printing at last stage.
   7. Please do not add more than 15 Character, It may cause buffer overrun in current implementation.
   8. You can add more than 15 characters in such a way 
       total No of characters in (ErrorTraceSerial_StartCharactor + ErrorTraceSerial_TerminatationCharactor) <= 30 characters.
   9. Shall applicable only if error reporting is enabled, Enable_Error_Reporting = Config_ON.
*/
#define ErrorTraceSerial_TerminatationCharactor "\n"



/* This Macro is to define the serial interface using to print output.
   1. By default serial is selected, 
   2. Based on needs can select any interface to stream the processed output.
   3. No critical section protection is applayed while doing serial printing.
*/
#define DebugTraceSerial_Print_Interface Serial.write

/* This Macro is to define the serial interface using to initialise the serial port.
   1. By default serial is selected, 
   2. Based on needs can select any interface to stream the processed output.
   3. make sure this shall inline with serial output interface.
*/
#define DebugTraceSerial_Print_Init Serial.begin

/*Reserve Max debug variable sizes as 1KB.
  If request more than Max_Debug_Buffer when Enable_Background_Print_Support Config_ON, then shall ignore rest of the string.
      It shall also consider including memory required to store time Info.*/
#define Max_Debug_Buffer 1024


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
      For Example:-  0    ==> Indicate No support for timeout.
      For Example:-  2000 ==> Indicate 2sec / 2000ms Time out if Free Buffer or Queue is not available.*/
#define BackGround_Debug_Trace_TimeOut  30000



/*-----------------------------------------------------------------------------
 *  Buffer streaming Configuration START
-------------------------------------------------------------------------------*/
/*
Buffer streaming:- 
    1. purpose is to puse the trace output to a comman global buffer.
    2. In this you can Pipe output to Buffer along with Serial port
    3. Or You can select stream only to Buffer.
    4. Or you can select stream only to Serial port, in that case you can save memory required for streaming buffer.
    5. In the streamed buffer can use to print log to html server.
    6. This stack shall not reserve the buffer, User has to reserve the buffer and request for populate the buffer, each time its required.
*/


/* This Macro is to Enable or dissable the printing over serial
     Config_ON   => Shall print over serial terminal.
     Config_OFF  => Shall not print over serial terminal.
                      1. This will still keep buffer and queue and do all processing,
                      2. Only point is shall not out vis serial out
                      3. Can useful if you did not want to print serila and just needs to extract buffer stream to publish.
                      4. Will increase the performance becaus not no needs to wait till previous buffers flushes uart. 
                      5. Shall also dissable printing of Error handling report for Queue timeout and Queue data lost.
*/
#define Enable_DebugTrace_Via_SerialPort Config_ON


/* This Macro is to Enable or dissable including of latest error trace in to the final buffer.
     Config_ON   => Shall include the error trace in final buffer output.
     Config_OFF  => Shall not include the error trace in final buffer output.
*/
#define Enable_IncludeLatestErrorTrace Config_ON

/* This Macro is to define the starting character / string to be transmits along with each trace buffer request, in populated buffer stream.
   1. By default "" being used, Indicating No start charactor/ string
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall not use for Serial output.
   5. It shall affect or consumed size in buffer.
   6. It shall be helpfull to add html tags, as per the design.
*/
#define DebugBufferStream_StartCharactor ""


/* This Macro is to define the character /strung to be used as each buffer terminatation in populated buffer stream.
   1. By default '\n' being used, 
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall not use for Serial output.
   5. It shall affect or consumed size in buffer.
   6. It shall be helpfull to add html tags, as per the design.
*/
#define DebugBufferStream_TerminatationCharactor "<br>\n"


/* This Macro is to define the starting character / string to be transmits along with each Error messages, in Debug Buffer Stream.
   1. By default "" being used, Indicating No start charactor/ string
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall used for Buffer streaming, If enabled option to include latest error Trace (Enable_IncludeLatestErrorTrace = Config_ON).
   5. It shall not use for Error reporting printing.
   6. It shall consumed the buffer for population.
   7. Please do not have any limit, only only limitation is the requested buffer for populating the buffer stream.
   8. Shall applicable only if error reporting is enabled, Enable_Error_Reporting = Config_ON.
*/
#define ErrorDebugBufferStream_StartCharactor ""


/* This Macro is to define the character /strung to be used as each buffer terminatation, in Debug Buffer Stream.
   1. By default '\n' being used, 
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall used for Buffer streaming, If enabled option to include latest error Trace (Enable_IncludeLatestErrorTrace = Config_ON).
   5. It shall not use for Error reporting printing.
   6. It shall consumed the buffer for population.
   7. Please do not have any limit, only only limitation is the requested buffer for populating the buffer stream.
   8. Shall applicable only if error reporting is enabled, Enable_Error_Reporting = Config_ON.
*/
#define ErrorDebugBufferStream_TerminatationCharactor "<br>\n"



/* This Macro is to define the Pre-delimoter before starting the Debug Queue buffer strings.
   1. By default "" being used, Indicating No start charactor/ string
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall not use for Serial output.
   5. It shall affect or consumed size in buffer.
   6. It shall only add Pre-delimoter string befor starting to populate Buffer, only once.
   7. It shall be helpfull to add html tags, as per the design.
*/
#define DebugBufferStream_Pre_delimoter "<h2> Debug Trave Messages </h2> <p> "


/* This Macro is to define the Post-delimoter After populating the Debug Queue buffer strings.
   1. By default "" being used, Indicating No start charactor/ string
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall not use for Serial output.
   5. It shall affect or consumed size in buffer.
   6. It shall only add Post-delimoter string After starting to populate Buffer, only once at the end.
   7 It shall be helpfull to add html tags, as per the design.
*/
#define DebugBufferStream_Post_delimoter "</p><hr style=\"border: 1px dashed blue\">"

/* This Macro is to define the Pre-delimoter before start populating the Error strings into buffer stream, at once.
   1. By default "" being used, Indicating No start charactor/ string
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall used for Buffer streaming, If enabled option to include latest error Trace (Enable_IncludeLatestErrorTrace = Config_ON).
   5. It shall not use for Error reporting printing.
   6. It shall consumed the buffer for population.
   7. Please do not have any limit, only only limitation is the requested buffer for populating the buffer stream.
   8. Shall applicable only if error reporting is enabled, Enable_Error_Reporting = Config_ON.
   9. It shall only add Pre-delimoter string befor starting to populate Bufferstream for Error messages, only once.
*/
#define ErrorDebugBufferStream_Pre_delimoter "<h2> Debug Trace Error Hadling </h2><p style=\"color:red\">"

/* This Macro is to define the Post-delimoter after compliting populating the Error strings into buffer stream, at once.
   1. By default "" being used, Indicating No start charactor/ string
   2. you can use any string also.
   3. Please make sure same should be within single / double quote 
   4. It shall used for Buffer streaming, If enabled option to include latest error Trace (Enable_IncludeLatestErrorTrace = Config_ON).
   5. It shall not use for Error reporting printing.
   6. It shall consumed the buffer for population.
   7. Please do not have any limit, only only limitation is the requested buffer for populating the buffer stream.
   8. Shall applicable only if error reporting is enabled, Enable_Error_Reporting = Config_ON.
   9. It shall only add Post-delimoter at the end after the population of Bufferstream for Error messages, only once.
*/
#define ErrorDebugBufferStream_Post_delimoter "</p><hr style=\"border: 1px dashed Green\">"

/*-----------------------------------------------------------------------------
 *  Debug Trace and Buffer streaming Configuration END
-------------------------------------------------------------------------------*/

/*******************************************************************************
 *  Program Specific Macros . Enums / Typedefs.
*******************************************************************************/

/* Following different function return values to define the state.*/
enum  Debug_Trace_FunStdRet_Type{
  Debug_Trace_OK       = 55,    /* Indicate Function processing is OK*/
  Debug_Trace_NOT_OK   = 48,    /* Indicate Function processing is NOT OK*/  
  };

/* Data type to define the data type used for pointing buffer Address*/
typedef unsigned short BufferAddType;

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


/* ************************************************************************
 * Function to Init output serial for debug purpose.
 *  1. without Init nothing shall work.
 * ************************************************************************
 */
extern void Init_Trace(void);

/* ************************************************************************
 * Function to print output for debug purpose.
 *  1. first input Argument is a string and can use format specifiers like %d, %s, %f... and so on
 *  2. Second input argument onwards provide variable name based on the format specifiers mentioned in the first input argument.
 *  3. Its use is excate simile to the printf() functions Standard C.
 *  4. Here only limitation is you cannot generate final string more than "Max_Debug_Buffer" bytes mentioned in the above configuration part.
 * ************************************************************************
 */
extern Debug_Trace_FunStdRet_Type Debug_Trace(const char *fmt, ...);


/* ************************************************************************
 * Function to populate the buffer stream based on the current available Queues.
 *  1. * InputBufferStream => First input argument shall be the starting address of the buffer to which string shall be populated.
 *  2. user needs to make sure it shall have enough memory to copy the requested memort size, else result in memory overflow.
 *  3. BufferStreamSize => Second argument is sizes of the required streaming buffer.
 *  4. It shall populate latest Queue which can fit within the requested memory. 
 *  5. Please use this function only when its required, because it will block serial printing for populate the string. 
 *  6. Its execution  time shall depends on the buffer sizes requested and total available queue to populate.
 *  7. If requested size if grater than "Max_BackGround_Buffer_Reserved", then shall consider only upto "Max_BackGround_Buffer_Reserved".
 * ************************************************************************
 */
extern void Populate_BufferStream_FromQueue(char * InputBufferStream, BufferAddType BufferStreamSize);


#endif /* End of  Debug_Trace_H */
