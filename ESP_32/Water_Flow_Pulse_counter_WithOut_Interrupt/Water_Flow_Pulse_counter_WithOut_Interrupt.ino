#include <dummy.h>



/*
  Application:
  - Interface water flow sensor with ESP32 board using interrupts. using Pulse counter for ESP32
  
  Board:
  - ESP32 Dev Module
  Sensor:
  - G 1/2 Water Flow Sensor
 */


#include <pcnt.h>

#define LED_BUILTIN 2
#define SENSOR  4     // Connected in GPIO 4

/**
 * TEST CODE BRIEF
 *
 * Use PCNT module to count rising edges and the print the counter value.
 *
 * Functionality of GPIOs used in this example:
 *   - GPIO2 - output pin of a sample 1 Hz pulse generator,
 *   - GPIO4 - pulse input pin,
 *
 * Load example, open a serial port to view the message printed on your screen.
 *
 * To do this test, you should connect any pulse outing sensore, like water flow meter.
 *
 * An interrupt will be triggered when the counter value:
 *   - reaches Max value set, to count the overflow.,
 *   - will be reset to zero.
 */
#define PCNT_H_LIM_VAL      0xFFFF     /* Max value on which event to be triggered.*/
#define PCNT_L_LIM_VAL      0x0000     /* Min value on which event to be triggered.*/

#define PCNT_INPUT_SIG_IO   SENSOR  // Pulse Input GPIO
#define PCNT_INPUT_CTRL_Ignore   -1  // ontrol signal input GPIO number, a negative value will be ignored

unsigned long int Counter_Overflow_Count = 0;

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
static void IRAM_ATTR pcnt_Event_Callback_intr_handler(void *arg)
{
    int pcnt_unit = (int)arg;
    uint32_t status; // information on the event type that caused the interrupt

    /* Get the PCNT event type that caused an interrupt*/
    pcnt_get_event_status(pcnt_unit, &status);
    /* If event is for Reaching Max value then Set Overflow variable for reference*/
    if (status & PCNT_EVT_H_LIM)
    {
      Counter_Overflow_Count++;
    }

    /* Print the event status For debugging purpose....*/
    if (status & PCNT_EVT_THRES_1)
    {
      Serial.println("THRES1 EVT");
    }
    if (status & PCNT_EVT_THRES_0)
    {
      Serial.println("THRES0 EVT");
    }
    if (status & PCNT_EVT_L_LIM)
    {
      Serial.println("L_LIM EVT");
    }
    if (status & PCNT_EVT_H_LIM)
    {
      Serial.println("H_LIM EVT");
    }
    if (status & PCNT_EVT_ZERO)
    {
      Serial.println("ZERO EVT");
    }
}


/* Initialize PCNT functions:
 *  - configure and initialize PCNT
 *  - set up the input filter
 *  - set up the counter events to watch
 */
static void pcnt_Chanel_init(int unit)
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = PCNT_INPUT_CTRL_Ignore,
        .channel = PCNT_CHANNEL_0,
        .unit = unit,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_DISABLE,    //  Inhibit counter(counter value will not change in this condition)
        .hctrl_mode = PCNT_MODE_DISABLE,    //  Inhibit counter(counter value will not change in this condition)
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = PCNT_H_LIM_VAL,
        .counter_l_lim = PCNT_L_LIM_VAL
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    //pcnt_set_filter_value(unit, 100);
    //cnt_filter_enable(unit);

    /* Enable events on zero, maximum and minimum limit values */
    pcnt_event_enable(unit, PCNT_EVT_ZERO);
    pcnt_event_enable(unit, PCNT_EVT_H_LIM);
    pcnt_event_enable(unit, PCNT_EVT_L_LIM);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);

    /* Init the counter */
    Counter_Overflow_Count = 0;

    /* Install interrupt service and add isr callback handler */
    pcnt_isr_service_install(0);
    pcnt_isr_handler_add(unit, pcnt_Event_Callback_intr_handler, (void *)unit);

    /* Everything is set up, now go to counting */
    pcnt_counter_resume(unit);
}





void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);
  /* Init counter */
  pcnt_Chanel_init(PCNT_UNIT_0);
}

void loop()
{
  int16_t count = 0;

   pcnt_get_counter_value(PCNT_UNIT_0, &count);

   Serial.print(" Each Count = ");
   Serial.print((int)count);
   Serial.print(", Overflow Count = ");
   Serial.println((int)Counter_Overflow_Count);
  
}
