#include "intervalTimer.h"
#include "xil_io.h"
#include <stdio.h>


#define INIT_DEFAULT 0 // default initialize value
#define RESET_DEFAULT 0 // reset timer value
#define TCSR0_OFFSET 0x00 // offset for TCSR0 register
#define TCSR1_OFFSET 0x10 // offset for TCSR1 register
#define TLR0_OFFSET 0x04  // offset for TLR0 register
#define TLR1_OFFSET 0x14  // offset for TLR1 register
#define TCR0_OFFSET 0x08  // offset for TCR0 register
#define TCR1_OFFSET 0x18// offset for TCR1 register

#define CASCADE_INIT_VALUE 0x800 // Value to enable cascade functionality

#define ENT0_START_MASK 0x80     // Mask for ENT0 start in TCSR0
#define ENT0_STOP_MASK 0xF7F     // Mask for ENT0 stop in TCSR0
#define LOAD_MASK 0x20            // Mask for load bit in TCSR0



// This function returns a value at the specified address.
//@param baseAddr: base address
//@param offset: offset
//@return value at specified address

uint32_t read_register(int32_t baseAddr, int32_t offset)
{
     return Xil_In32(baseAddr + offset);

}


// This function writes a specified value to a specified address
//@param baseAddr: base address
//@param offset: offset
//@param value: value to be written

void write_register(int32_t baseAddr, int32_t offset, uint32_t value)
{
    Xil_Out32(baseAddr + offset, value);
}



intervalTimer_status_t intervalTimer_init(uint32_t timerNumber)
{
    switch(timerNumber)
    {
    case INTERVAL_TIMER_TIMER_0:
        //write a 0 to the TCSR0 register
        write_register(XPAR_AXI_TIMER_0_BASEADDR, TCSR0_OFFSET, INIT_DEFAULT);
        // write a 0 to the TCSR1 register
        write_register(XPAR_AXI_TIMER_0_BASEADDR, TCSR1_OFFSET, INIT_DEFAULT);
        //set CASC bit with UDT0 cleared in TCSR0
        write_register(XPAR_AXI_TIMER_0_BASEADDR, TCSR0_OFFSET, CASCADE_INIT_VALUE);
        break;
    case INTERVAL_TIMER_TIMER_1:
         //write a 0 to the TCSR0 register
        write_register(XPAR_AXI_TIMER_1_BASEADDR, TCSR0_OFFSET, INIT_DEFAULT);
        // write a 0 to the TCSR1 register
        write_register(XPAR_AXI_TIMER_1_BASEADDR, TCSR1_OFFSET, INIT_DEFAULT);
        //set CASC bit with UDT0 cleared in TCSR0
        write_register(XPAR_AXI_TIMER_1_BASEADDR, TCSR0_OFFSET, CASCADE_INIT_VALUE);
        break;
    case INTERVAL_TIMER_TIMER_2:
         //write a 0 to the TCSR0 register
        write_register(XPAR_AXI_TIMER_2_BASEADDR, TCSR0_OFFSET, INIT_DEFAULT);
        // write a 0 to the TCSR1 register
        write_register(XPAR_AXI_TIMER_2_BASEADDR, TCSR1_OFFSET, INIT_DEFAULT);
        //set CASC bit with UDT0 cleared in TCSR0
        write_register(XPAR_AXI_TIMER_2_BASEADDR, TCSR0_OFFSET, CASCADE_INIT_VALUE);
        break;
    default:
        printf("IntervalTimer_init call failed./n/r");
        return INTERVAL_TIMER_STATUS_FAIL;
    }

    return INTERVAL_TIMER_STATUS_OK;

}



intervalTimer_status_t intervalTimer_initAll()
{
    // calling intervalTimer_init for timer 0:
    if (intervalTimer_init(INTERVAL_TIMER_TIMER_0) == INTERVAL_TIMER_STATUS_FAIL)
    {
        printf("Error in calling intervalTimer_initAll on timer 0 /n/r");
        return INTERVAL_TIMER_STATUS_FAIL;
    }
    // calling intervalTimer_init for timer 1:
    if (intervalTimer_init(INTERVAL_TIMER_TIMER_1) == INTERVAL_TIMER_STATUS_FAIL)
    {
        printf("Error in calling intervalTimer_initAll on timer 1 /n/r");
        return INTERVAL_TIMER_STATUS_FAIL;
    }

    // calling intervalTimer_init for timer 2:
    if (intervalTimer_init(INTERVAL_TIMER_TIMER_2) == INTERVAL_TIMER_STATUS_FAIL)
    {
        printf("Error in calling intervalTimer_initAll on timer 2 /n/r");
        return INTERVAL_TIMER_STATUS_FAIL;
    }

    return INTERVAL_TIMER_STATUS_OK;

}

void intervalTimer_start(uint32_t timerNumber)
{
    //POSSIBLY MISSING DO NOTHING IF TIMER IS ALREADY STARTED

    // This variable will store current value of TCSR0 so it can be masked with ENT0_START_MASK.
    uint32_t current_TCSR0_value = 0;
    // This variable will be used to set the ENT0 bit to a 1
    uint32_t start_mask = ENT0_START_MASK; // can possibly omit, it's here because I'm unsure if the #define ENT0_START_MASK will convert to 32bit.

    // write a 1 to the ENT0 bit of the TCRS0 register of specified timerNumber
    switch(timerNumber)
    {
    case INTERVAL_TIMER_TIMER_0:
        // read the current value of timer 0
        current_TCSR0_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCSR0_OFFSET);
        // write the newly masked value into timer 0
        write_register(XPAR_AXI_TIMER_0_BASEADDR, TCSR0_OFFSET, current_TCSR0_value | start_mask);
        break;
    case INTERVAL_TIMER_TIMER_1:
         // read the current value of timer 1
        current_TCSR0_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCSR0_OFFSET);
        // write the newly masked value into timer 1
        write_register(XPAR_AXI_TIMER_1_BASEADDR, TCSR0_OFFSET, current_TCSR0_value | start_mask);
        break;
    case INTERVAL_TIMER_TIMER_2:
        // read the current value of timer 2
        current_TCSR0_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCSR0_OFFSET);
        // write the newly masked value into timer 2
        write_register(XPAR_AXI_TIMER_2_BASEADDR, TCSR0_OFFSET, current_TCSR0_value | start_mask);
        break;
    default:
        printf("IntervalTimer_start call failed./n/r");

    }

}


void intervalTimer_stop(uint32_t timerNumber)
{
    //POSSIBLY MISSING DO NOTHING IF TIMER IS ALREADY STOPPED

    // This variable will store current value of TCSR0 so it can be masked with ENT0_STOP_MASK.
    uint32_t current_TCSR0_value = 0;
    // This variable will be used to set the ENT0 bit to a 0
    uint32_t stop_mask = ENT0_STOP_MASK; // can possibly omit, it's here because I'm unsure if the #define ENT0_STOP_MASK will convert to 32bit.

    // write a 0 to the ENT0 bit of the TCRS0 register of specified timerNumber
    switch(timerNumber)
    {
    case INTERVAL_TIMER_TIMER_0:
        // read the current value of timer 0
        current_TCSR0_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCSR0_OFFSET);
        // write the newly masked value into timer 0
        write_register(XPAR_AXI_TIMER_0_BASEADDR, TCSR0_OFFSET, current_TCSR0_value & stop_mask);
        break;
    case INTERVAL_TIMER_TIMER_1:
         // read the current value of timer 1
        current_TCSR0_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCSR0_OFFSET);
        // write the newly masked value into timer 1
        write_register(XPAR_AXI_TIMER_1_BASEADDR, TCSR0_OFFSET, current_TCSR0_value & stop_mask);
        break;
    case INTERVAL_TIMER_TIMER_2:
        // read the current value of timer 2
        current_TCSR0_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCSR0_OFFSET);
        // write the newly masked value into timer 2
        write_register(XPAR_AXI_TIMER_2_BASEADDR, TCSR0_OFFSET, current_TCSR0_value & stop_mask);
        break;
    default:
        printf("IntervalTimer_stop call failed./n/r");

    }

}

void intervalTimer_reset(uint32_t timerNumber)
{
     // This variable will store current value of TCSR0 so it can be masked with LOAD_MASK.
    uint32_t current_TCSR0_value = 0;
    // This variable will store current value of TCSR1 so it can be masked with LOAD_MASK.
    uint32_t current_TCSR1_value = 0;
    // This variable will be used to set the ENT0 bit to a 0
    uint32_t load_mask = LOAD_MASK; // can possibly omit, it's here because I'm unsure if the #define LOAD_MASK will convert to 32bit.


    switch(timerNumber)
    {
    case INTERVAL_TIMER_TIMER_0:

        // stop the current timer
        intervalTimer_stop(timerNumber);

        //write a 0 to the TLR0 register
        write_register(XPAR_AXI_TIMER_0_BASEADDR, TLR0_OFFSET, RESET_DEFAULT);
        // write a 1 to the LOAD0 bit in the TCSR0 register:
        //read current value first
        current_TCSR0_value = read_register(XPAR_AXI_TIMER_0_BASEADDR,TCSR0_OFFSET);
        //write newly masked value
        write_register(XPAR_AXI_TIMER_0_BASEADDR, TCSR0_OFFSET, current_TCSR0_value | load_mask);

        //write a 0 to the TLR1 register
        write_register(XPAR_AXI_TIMER_0_BASEADDR, TLR1_OFFSET, RESET_DEFAULT);
        //Write a 1 to the LOAD1 bit in the TCSR1 register:
        //read current value first
        current_TCSR1_value = read_register(XPAR_AXI_TIMER_0_BASEADDR,TCSR1_OFFSET);
        //write newly masked value
        write_register(XPAR_AXI_TIMER_0_BASEADDR, TCSR1_OFFSET, current_TCSR1_value | load_mask);


        break;

    case INTERVAL_TIMER_TIMER_1:
        // stop the current timer
        intervalTimer_stop(timerNumber);

        //write a 0 to the TLR0 register
        write_register(XPAR_AXI_TIMER_1_BASEADDR, TLR0_OFFSET, RESET_DEFAULT);
        // write a 1 to the LOAD0 bit in the TCSR0 register:
        //read current value first
        current_TCSR0_value = read_register(XPAR_AXI_TIMER_1_BASEADDR,TCSR0_OFFSET);
        //write newly masked value
        write_register(XPAR_AXI_TIMER_1_BASEADDR, TCSR0_OFFSET, current_TCSR0_value | load_mask);

        //write a 0 to the TLR1 register
        write_register(XPAR_AXI_TIMER_1_BASEADDR, TLR1_OFFSET, RESET_DEFAULT);
        //Write a 1 to the LOAD1 bit in the TCSR1 register:
        //read current value first
        current_TCSR1_value = read_register(XPAR_AXI_TIMER_1_BASEADDR,TCSR1_OFFSET);
        //write newly masked value
        write_register(XPAR_AXI_TIMER_1_BASEADDR, TCSR1_OFFSET, current_TCSR1_value | load_mask);

        break;

    case INTERVAL_TIMER_TIMER_2:
       // stop the current timer
        intervalTimer_stop(timerNumber);

        //write a 0 to the TLR0 register
        write_register(XPAR_AXI_TIMER_2_BASEADDR, TLR0_OFFSET, RESET_DEFAULT);
        // write a 1 to the LOAD0 bit in the TCSR0 register:
        //read current value first
        current_TCSR0_value = read_register(XPAR_AXI_TIMER_2_BASEADDR,TCSR0_OFFSET);
        //write newly masked value
        write_register(XPAR_AXI_TIMER_2_BASEADDR, TCSR0_OFFSET, current_TCSR0_value | load_mask);

        //write a 0 to the TLR1 register
        write_register(XPAR_AXI_TIMER_2_BASEADDR, TLR1_OFFSET, RESET_DEFAULT);
        //Write a 1 to the LOAD1 bit in the TCSR1 register:
        //read current value first
        current_TCSR1_value = read_register(XPAR_AXI_TIMER_2_BASEADDR,TCSR1_OFFSET);
        //write newly masked value
        write_register(XPAR_AXI_TIMER_2_BASEADDR, TCSR1_OFFSET, current_TCSR1_value | load_mask);
        break;

    default:
        printf("IntervalTimer_reset call failed./n/r");

    }

    intervalTimer_init(timerNumber);
}

void intervalTimer_resetAll()
{
    //reset all timers
    intervalTimer_reset(INTERVAL_TIMER_TIMER_0);
    intervalTimer_reset(INTERVAL_TIMER_TIMER_1);
    intervalTimer_reset(INTERVAL_TIMER_TIMER_2);
}

intervalTimer_status_t intervalTimer_test(uint32_t timerNumber)
{
    //These variables will be used to read the value and see if the timer is changing when it should, and not changing when it shouldn't be changing.
    uint32_t prev_value = 0;
    uint32_t current_value = 0;
    switch(timerNumber)
        {
        case INTERVAL_TIMER_TIMER_0:

            // reset the current timer
            intervalTimer_reset(timerNumber);

            // start the current timer
            intervalTimer_start(timerNumber);

            // read timer a few times to verify that it's changing
            prev_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCR0_OFFSET);

            // reading a few times...
            current_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCR0_OFFSET);
            if (current_value == prev_value)
               return INTERVAL_TIMER_STATUS_FAIL; // value hasn't changed; error

            current_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCR0_OFFSET);

            if (current_value == prev_value)
                return INTERVAL_TIMER_STATUS_FAIL; // value hasn't changed; error;



            // stop the current timer
            intervalTimer_stop(timerNumber);

            // read timer a few times to verify that it's not changing
            prev_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCR0_OFFSET);

            // reading a few times...
            current_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCR0_OFFSET);
             if (current_value != prev_value)
               return INTERVAL_TIMER_STATUS_FAIL; // value has changed; error

             current_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCR0_OFFSET);

             if (current_value != prev_value)
              return INTERVAL_TIMER_STATUS_FAIL; // value has changed; error;
             break;


        case INTERVAL_TIMER_TIMER_1:
            // reset the current timer
                        intervalTimer_reset(timerNumber);

                        // start the current timer
                        intervalTimer_start(timerNumber);

                        // read timer a few times to verify that it's changing
                        prev_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCR0_OFFSET);

                        // reading a few times...
                        current_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCR0_OFFSET);
                        if (current_value == prev_value)
                           return INTERVAL_TIMER_STATUS_FAIL; // value hasn't changed; error

                        current_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCR0_OFFSET);

                        if (current_value == prev_value)
                            return INTERVAL_TIMER_STATUS_FAIL; // value hasn't changed; error;



                        // stop the current timer
                        intervalTimer_stop(timerNumber);

                        // read timer a few times to verify that it's not changing
                        prev_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCR0_OFFSET);

                        // reading a few times...
                        current_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCR0_OFFSET);
                         if (current_value != prev_value)
                           return INTERVAL_TIMER_STATUS_FAIL; // value has changed; error

                         current_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCR0_OFFSET);

                         if (current_value != prev_value)
                          return INTERVAL_TIMER_STATUS_FAIL; // value has changed; error;
                         break;
        case INTERVAL_TIMER_TIMER_2:
            // reset the current timer
                        intervalTimer_reset(timerNumber);

                        // start the current timer
                        intervalTimer_start(timerNumber);

                        // read timer a few times to verify that it's changing
                        prev_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCR0_OFFSET);

                        // reading a few times...
                        current_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCR0_OFFSET);
                        if (current_value == prev_value)
                           return INTERVAL_TIMER_STATUS_FAIL; // value hasn't changed; error

                        current_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCR0_OFFSET);

                        if (current_value == prev_value)
                            return INTERVAL_TIMER_STATUS_FAIL; // value hasn't changed; error;



                        // stop the current timer
                        intervalTimer_stop(timerNumber);

                        // read timer a few times to verify that it's not changing
                        prev_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCR0_OFFSET);

                        // reading a few times...
                        current_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCR0_OFFSET);
                         if (current_value != prev_value)
                           return INTERVAL_TIMER_STATUS_FAIL; // value has changed; error

                         current_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCR0_OFFSET);

                         if (current_value != prev_value)
                          return INTERVAL_TIMER_STATUS_FAIL; // value has changed; error;
        default:
            printf("IntervalTimer_test call failed./n/r");

        }
    return INTERVAL_TIMER_STATUS_OK;

}


intervalTimer_status_t intervalTimer_testAll()
{
    // call interval timer test on all timers
    // return status FAIL if failed, or status OK otherwise
    if ((intervalTimer_test(INTERVAL_TIMER_TIMER_0)) == INTERVAL_TIMER_STATUS_FAIL)
        return INTERVAL_TIMER_STATUS_FAIL;

    if ((intervalTimer_test(INTERVAL_TIMER_TIMER_1)) == INTERVAL_TIMER_STATUS_FAIL)
            return INTERVAL_TIMER_STATUS_FAIL;

    if ((intervalTimer_test(INTERVAL_TIMER_TIMER_2)) == INTERVAL_TIMER_STATUS_FAIL)
            return INTERVAL_TIMER_STATUS_FAIL;

    return INTERVAL_TIMER_STATUS_OK;


}

double intervalTimer_getTotalDurationInSeconds(uint32_t timerNumber)
{

    uint32_t counter0_value = 0;
    uint32_t counter1_value = 0;
    uint32_t check_counter1 = 0;
    double total_value = 0;



        switch(timerNumber)
        {
        case INTERVAL_TIMER_TIMER_0:
            // read the current value of counter 0
            counter0_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCR0_OFFSET);
            //read the current value of counter 1
            counter1_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCR1_OFFSET);
            // read current value of counter 1 again
            check_counter1 = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCR1_OFFSET);
            if (check_counter1 != counter1_value)
            {
                //re-read counter 0
                counter0_value = read_register(XPAR_AXI_TIMER_0_BASEADDR, TCR0_OFFSET);

                // add new counter 1 with counter 0
                total_value = check_counter1 + counter0_value;
                return total_value;
            }

            // add both values

            total_value = counter0_value + counter1_value;
            // return duration in seconds
            return total_value/XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ;

        case INTERVAL_TIMER_TIMER_1:
            // read the current value of counter 0
            counter0_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCR0_OFFSET);
            //read the current value of counter 1
            counter1_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCR1_OFFSET);

            // read current value of counter 1 again
                        check_counter1 = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCR1_OFFSET);
                        if (check_counter1 != counter1_value)
                        {
                            //re-read counter 0
                            counter0_value = read_register(XPAR_AXI_TIMER_1_BASEADDR, TCR0_OFFSET);

                            // add new counter 1 with counter 0
                            total_value = check_counter1 + counter0_value;
                            return total_value;
                        }
            // add both values
            total_value = counter0_value + counter1_value;
            // return duration in seconds
            return total_value/XPAR_AXI_TIMER_1_CLOCK_FREQ_HZ;


        case INTERVAL_TIMER_TIMER_2:
            // read the current value of counter 0
            counter0_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCR0_OFFSET);
            //read the current value of counter 1
            counter1_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCR1_OFFSET);

            // read current value of counter 1 again
                        check_counter1 = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCR1_OFFSET);
                        if (check_counter1 != counter1_value)
                        {
                            //re-read counter 0
                            counter0_value = read_register(XPAR_AXI_TIMER_2_BASEADDR, TCR0_OFFSET);

                            // add new counter 1 with counter 0
                            total_value = check_counter1 + counter0_value;
                            return total_value;
                        }
            // add both values
            total_value = counter0_value + counter1_value;
            // return duration in seconds
            return total_value/XPAR_AXI_TIMER_2_CLOCK_FREQ_HZ;

        default:
            printf("IntervalTimer_getTotalDurationInSeconds call failed./n/r");
            return 0;

        }
}



