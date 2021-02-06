/*
 * File:   metercam.c
 *
 * Sleep for a given duration with minimum possible power consumption.
 * 
 * When time expires, activate ESP32-CAM.
 * 
 * Go back to sleep after Timeout, or when receiving request to do so
 * from ESP32-CAM.
 *
 * Created on January 31, 2021, 11:53 AM
 */


#include <stdint.h>
#include <xc.h>

#include "mcc_generated_files/tmr0.h"
#include "mcc_generated_files/eusart1.h"

// maximum on time until we forcibly switch off the ESP32CAM
#define MAX_ON_TIME_TICKS (20)

// unless we are told otherwise (via serial requests by ESP32CAM), we sleep
// this many intervals between wake-ups.
#define DEFAULT_NR_OF_SLEEP_INTERVALS (10)

// SYSTICK, approximately in seconds
volatile uint16_t global_time_ticks;

// our next hibernation will last for this many watchdog timer intervals
uint16_t desired_nr_of_sleep_intervals = DEFAULT_NR_OF_SLEEP_INTERVALS;

// needed by command_parser_ingest to determine what to do with the
// next incoming chunk of data
int command_parser_state;

void dio_enable_esp32cam()
{
    PORTAbits.RA2 = 1;
}

void dio_disable_esp32cam()
{
    PORTAbits.RA2 = 0;
}

// Enter deepest possible sleep mode, everything off, only thing that will
// eventually wake us is the watchdog. This is the most power-saving sleep
// possible for this controller.
void sleep_until_woken_by_watchdog(void)
{
    SLEEP();
}

void my_TMR0_SystickInterruptHandler(void)
{
    //TMR0_Reload()
    global_time_ticks++;
}

//
// Command Parser
//

// reset to initial state
void command_parser_init(void)
{
    command_parser_state = 10;
}
// Interpret the bytes received from our command interface as they 
// come in.
void command_parser_ingest(uint8_t b)
{
    // persist command across states
    static uint8_t cmd;
    
    // for reading an uint16 (states 100ff)
    static uint16_t val;
    
    switch(command_parser_state)
    {
        case 10:
            if(b=='A') // set new sleep duration, followed by 2 bytes
            {
                cmd = b;
                command_parser_state = 100; // read uint16
            }
            else if(b=='S') // go to sleep now
            {
                EUSART1_Write('s');
                EUSART1_Write((desired_nr_of_sleep_intervals>>8)&0xff); // hi
                EUSART1_Write(desired_nr_of_sleep_intervals&0xff);      // lo
                global_time_ticks = MAX_ON_TIME_TICKS;
            }
            else if(b=='E') // echo
            {
                EUSART1_Write('e');
            }
            else if(b=='V') // version
            {
                EUSART1_Write('1');
            }
            break;
            
        case 100: // read high byte of uint16
            val = (uint16_t)b << 8u;
            command_parser_state = 101;
            break;
            
        case 101: // read low byte of uint16
            val |= b;
            command_parser_state = 200; // process
            /* no-break: process immediately! */
            
        case 200:
            if(cmd=='A')
            {
                desired_nr_of_sleep_intervals = val;
                EUSART1_Write('a');
                EUSART1_Write((val>>8)&0xff); // hi
                EUSART1_Write(val&0xff);      // lo
            }
            command_parser_state = 10; // ready for next command
            break;
            
        default:
            command_parser_state = 10;
            break;
    }
}

void app_metercam(void) 
{
    
    // On startup, we activate the ESP32-CAM. Unless we hear otherwise,
    // we will switch it off after the configured timeout duration, and enter
    // our sleep cycle.
    
    uint32_t sleep_interval_countdown;
    
    TMR0_SetInterruptHandler(my_TMR0_SystickInterruptHandler);
    
    while(1)
    {
        command_parser_init();
        dio_enable_esp32cam();
        global_time_ticks = 0;
        TMR0_StartTimer();
        do
        {
            // poll UART for incoming requests
            if(EUSART1_is_rx_ready())
            {
                uint8_t b = EUSART1_Read();
                command_parser_ingest(b);
            }
            // @todo if sleep request: break
            CLRWDT();
        } while(global_time_ticks < MAX_ON_TIME_TICKS);

        TMR0_StopTimer();
        dio_disable_esp32cam();

        sleep_interval_countdown = desired_nr_of_sleep_intervals;
        while(sleep_interval_countdown--)
        {
            CLRWDT();
            sleep_until_woken_by_watchdog();
        }
    }
    // will never return
}
