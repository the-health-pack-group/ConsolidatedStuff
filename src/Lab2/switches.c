#include "switches.h"
#include "supportFiles/display.h"
#include "xil_io.h"
#include "xparameters.h"
#include  <stdio.h>
#include "supportFiles/leds.h"

#define SWITHES_GPIO_CONFIG_INPUT 1


#define SWITCHES_GPIO_DEVICE_BASE_ADDRESS XPAR_SLIDE_SWITCHES_BASEADDR

#define TRI_REGISTER_OFFSET 0x04
#define DATA_REGISTER_OFFSET 0x0
#define TERMINATE_SWITCHES_SEQUENCE 0xF
#define ALL_SWITCHES_MASK 0xF
#define LEDS_OFF 0

/* This function initializes the necessary registers using the built-in Xil_out and Xil_in functions
 as defined in the xil_io.h file.
 @return returns one of two status depending on if the correct write went through or not.
*/
int32_t switches_init()
{
    //leds_init function requires a true or false value. 1 is given here since I would like an error shown if something went wrong.
    leds_init(1);

    // turn off all leds from any previous launch
    leds_write(LEDS_OFF);


    // write GpioRegister
    Xil_Out32(SWITCHES_GPIO_DEVICE_BASE_ADDRESS +TRI_REGISTER_OFFSET,SWITHES_GPIO_CONFIG_INPUT);

    // READ GpioRegister

     int32_t test_register =0;
     test_register = Xil_In32(SWITCHES_GPIO_DEVICE_BASE_ADDRESS + TRI_REGISTER_OFFSET);

    if (test_register ==SWITHES_GPIO_CONFIG_INPUT)
     {

         return SWITCHES_INIT_STATUS_OK;

     }

     return SWITCHES_INIT_STATUS_FAIL;

}

/* This function reads the value of the switches using the built-in Xil_In32 function
 @return the 32-bit value read.
 */

int32_t switches_read()
{
    int32_t myvar = Xil_In32(SWITCHES_GPIO_DEVICE_BASE_ADDRESS + DATA_REGISTER_OFFSET);
        return myvar;
}


/* this function reads the value of the switches and writes them to the board using the leds_write function defined in "leds.h".
 * The function will terminate when all switches are on simultaneously.
 */


void switches_runTest()
{

    switches_init();
    int32_t current_switches =0;

    //Get current switches until terminate sequence is reached
    while(current_switches!= TERMINATE_SWITCHES_SEQUENCE)
    {
        current_switches = (switches_read() & ALL_SWITCHES_MASK);
        leds_write(current_switches);

    }
    leds_write(LEDS_OFF);


}
