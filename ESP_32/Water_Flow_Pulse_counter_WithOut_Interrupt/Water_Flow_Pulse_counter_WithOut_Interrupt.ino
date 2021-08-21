
#include "driver/pcnt.h"                                 /* Include the required header file, Its working Arduion IDE 2.0*/

#define PCNT_UNIT_Used      PCNT_UNIT_0                  /* Select the Pulse Count 0  as the unit..*/
#define PCNT_H_LIM_VAL      10000                        /* Set the max limit to trigger the interrupt*/
#define PCNT_INPUT_SIG_IO   4                            /* Pulse Input selected as GPIO 4 */

int contadorOverflow;                                    /* Variable to store the over flow count */


pcnt_isr_handle_t user_isr_handle = NULL;                /* User ISR handler for Interrupt */

/* *********************************************************************************
  ISR Function to trigen when ever over flow is detected.......
*************************************************************************************/
void IRAM_ATTR CounterOverflow_ISR(void *arg)             
{
  /* Increment Over flow counter */
  contadorOverflow = contadorOverflow + 1;  
  /* Clear counter*/              
  PCNT.int_clr.val = BIT(PCNT_UNIT_Used);                 
  pcnt_counter_clear(PCNT_UNIT_Used);                     
}

//------------------------------------------------------------
void Init_PulseCounter (void)
{
  pcnt_config_t pcntFreqConfig = { };                        // Declear variable for cinfig
  pcntFreqConfig.pulse_gpio_num = PCNT_INPUT_SIG_IO;         // Set the port ping using for counting
  pcntFreqConfig.pos_mode = PCNT_COUNT_INC;                  // set Counter mode: Increase counter value
  pcntFreqConfig.counter_h_lim = PCNT_H_LIM_VAL;             // Set Over flow Interupt / event value
  pcntFreqConfig.unit = PCNT_UNIT_Used;                      //  Set Pulsos unit to ne used
  pcntFreqConfig.channel = PCNT_CHANNEL_0;                   //  select PCNT channel 0
  pcnt_unit_config(&pcntFreqConfig);                         // Configure PCNT.

  pcnt_counter_pause(PCNT_UNIT_Used);                        // Pause PCNT counter such that we can set event.
  pcnt_counter_clear(PCNT_UNIT_Used);                        // Clear PCNT counter to avoid ant mis counting.

  pcnt_event_enable(PCNT_UNIT_Used, PCNT_EVT_H_LIM);         // Enable event for when PCNT watch point event: Maximum counter value
  pcnt_isr_register(CounterOverflow_ISR, NULL, 0, &user_isr_handle);  // Set call back function for the Event.
  pcnt_intr_enable(PCNT_UNIT_Used);                          // Enable PCNT

  pcnt_counter_resume(PCNT_UNIT_Used);                       // Re-started PCNT.

 Serial.println("PCNT Init Completed....");
}


/* *********************************************************************************
   Function to clean the Counter and its variables......

*************************************************************************************/
void Clean_Counters()                                       
{

  contadorOverflow = 0;                                     // Clear global Over flow counter.
  pcnt_counter_clear(PCNT_UNIT_Used);                       // Clean Pulse Counter...

}



void setup() {
   Serial.begin(115200);
   delay(500);
  // put your setup code here, to run once:
Init_PulseCounter();
}

void loop() 
{
  /* Add static variable to store previous value*/
  static int16_t Previous_count = 0;
  static int Previous_contadorOverflow = 0;
  // put your main code here, to run repeatedly:
  int16_t count = 0;

   pcnt_get_counter_value(PCNT_UNIT_Used, &count);

   /* Print only if there is any difference or change in tha value.*/
   if( (Previous_count != count) || (Previous_contadorOverflow != contadorOverflow) )
   {
   Serial.print(" Each Count = ");
   Serial.print((int)count);
   Serial.print(", Overflow Count = ");
   Serial.println((int)contadorOverflow);

    Previous_count = count;
   Previous_contadorOverflow = contadorOverflow;

   }

delay(500);  /* Delay is given to give result properly..*/

}
