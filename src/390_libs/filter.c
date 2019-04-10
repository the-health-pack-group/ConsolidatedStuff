#include "filter.h"
#include <stdio.h>

#define FILTER_IIR_FILTER_COUNT 10                          //This is the number of IIR filters we are using
#define IIR_A_COEFFICIENT_COUNT 10                          //This is how many 'A' coefficients there are per IIR filter
#define IIR_FILTER_LAST_A_INDEX (IIR_A_COEFFICIENT_COUNT-1) //This is the index of the last coefficient for each IIR filter
#define IIR_B_COEFFICIENT_COUNT 11                          //This is how many 'B' coefficients there are per IIR filter
#define IIR_FILTER_LAST_B_INDEX (IIR_B_COEFFICIENT_COUNT-1) //This is the index of the last coefficient for each IIR filter
#define FIR_FILTER_TAP_COUNT 81                             //The length of the FIR filter
#define FIR_FILTER_LAST_INDEX 80                            //The index of the last coefficient in the FIR filter
#define FILTER_XQUEUE_SIZE FIR_FILTER_TAP_COUNT             //This is the size of the xQueue. It is equal to the FIR filter length.
#define FILTER_YQUEUE_SIZE IIR_B_COEFFICIENT_COUNT          //This is the size of the yQueue. It is equal to the number of B-coefficients.
#define FILTER_ZQUEUE_SIZE IIR_A_COEFFICIENT_COUNT          //This is the size of each zQueue. It is equal to the number of A-coefficients.
#define FILTER_OUTPUTQUEUE_SIZE 2000                        //This is the size of the outputQueue. It holds 200ms of data at a sampling rate of 10kHz.
#define FILTER_OUTPUTQUEUE_OLDVAL_INDEX 0                   //This is the index of the oldest value in the outputQueue. The value at this index is skipped over when forceComputeFromScratch=true and used when forceComputeFromScratch=false.
#define FILTER_OUTPUTQUEUE_NEWVAL_INDEX FILTER_OUTPUTQUEUE_SIZE //This is the index of the most recent value pushed onto the outputQueue
#define FILTER_OUTPUTQUEUE_OLDVAL_OFFSET 1                  //This is the offset required because we are keeping the oldest value of the outputQueue on the queue itself. We use this to skip over it and to allocate the true size of the queue.
#define FILTER_NAME_SIZE 50                                 //This is the size we allocate for the names of each filter
#define FILTER_INIT_VAL 0.0                                 //This is an initializing value used for double-type variables
#define DECIMATION_VALUE 10                                 //This is how much decimation we are performing. A decimation factor of 10 means we are down-sampling from 100kHz to 10kHz.
#define RESET 0                                             //An initializing value used for int-type variables

static queue_t x_queue;                                         //the input to the FIR filter
static queue_t y_queue;                                         //the output of the FIR filter and one of the inputs to the IIR filters
static queue_t z_queue[FILTER_IIR_FILTER_COUNT];                //both an output and input for each IIR filter
static queue_t output_queue[FILTER_IIR_FILTER_COUNT];           //Output for each IIR filter used to calculate power
static double current_power_vals[FILTER_IIR_FILTER_COUNT];      //The most recently calculated power values for each IIR filter

const static double firCoefficients[FIR_FILTER_TAP_COUNT] = {   //The coefficients for the FIR Filter. These are used to perform the anti-aliasing as we down-sample.
5.3751585173668532e-04,
4.1057821244099187e-04,
2.3029811615433415e-04,
-1.0022421255268634e-05,
-3.1239220025873498e-04,
-6.6675539469892989e-04,
-1.0447674325821804e-03,
-1.3967861519587053e-03,
-1.6537303925483089e-03,
-1.7346382015025667e-03,
-1.5596484449522630e-03,
-1.0669170920384048e-03,
-2.3088291222664008e-04,
9.2146384892950099e-04,
2.2999023467067978e-03,
3.7491095163012366e-03,
5.0578112742500408e-03,
5.9796411037508039e-03,
6.2645305736409576e-03,
5.6975395969924630e-03,
4.1403678436423832e-03,
1.5696670848600943e-03,
-1.8940691237627923e-03,
-5.9726960144609528e-03,
-1.0235869785695425e-02,
-1.4127694707478473e-02,
-1.7013744396319821e-02,
-1.8244737113263923e-02,
-1.7230507462687290e-02,
-1.3515937014932726e-02,
-6.8495092580338254e-03,
2.7646568253820039e-03,
1.5039019355364032e-02,
2.9404269200373489e-02,
4.5042275861018187e-02,
6.0948410493762414e-02,
7.6017645500231018e-02,
8.9145705550443280e-02,
9.9334411853457705e-02,
1.0578952596388017e-01,
1.0800000000000000e-01,
1.0578952596388017e-01,
9.9334411853457705e-02,
8.9145705550443280e-02,
7.6017645500231018e-02,
6.0948410493762414e-02,
4.5042275861018187e-02,
2.9404269200373489e-02,
1.5039019355364032e-02,
2.7646568253820039e-03,
-6.8495092580338254e-03,
-1.3515937014932726e-02,
-1.7230507462687290e-02,
-1.8244737113263923e-02,
-1.7013744396319821e-02,
-1.4127694707478473e-02,
-1.0235869785695425e-02,
-5.9726960144609528e-03,
-1.8940691237627923e-03,
1.5696670848600943e-03,
4.1403678436423832e-03,
5.6975395969924630e-03,
6.2645305736409576e-03,
5.9796411037508039e-03,
5.0578112742500408e-03,
3.7491095163012366e-03,
2.2999023467067978e-03,
9.2146384892950099e-04,
-2.3088291222664008e-04,
-1.0669170920384048e-03,
-1.5596484449522630e-03,
-1.7346382015025667e-03,
-1.6537303925483089e-03,
-1.3967861519587053e-03,
-1.0447674325821804e-03,
-6.6675539469892989e-04,
-3.1239220025873498e-04,
-1.0022421255268634e-05,
2.3029811615433415e-04,
4.1057821244099187e-04,
5.3751585173668532e-04};

const static double iirACoefficientConstants[FILTER_IIR_FILTER_COUNT][IIR_A_COEFFICIENT_COUNT] = {  //The A-coefficients used for each IIR filter. Each row corresponds to a different filter.
{-5.9637727070164015e+00, 1.9125339333078248e+01, -4.0341474540744173e+01, 6.1537466875368821e+01, -7.0019717951472188e+01, 6.0298814235238872e+01, -3.8733792862566290e+01, 1.7993533279581058e+01, -5.4979061224867651e+00, 9.0332828533799547e-01},
{-4.6377947119071443e+00, 1.3502215749461572e+01, -2.6155952405269755e+01, 3.8589668330738348e+01, -4.3038990303252632e+01, 3.7812927599537133e+01, -2.5113598088113793e+01, 1.2703182701888094e+01, -4.2755083391143520e+00, 9.0332828533800291e-01},
{-3.0591317915750960e+00, 8.6417489609637634e+00, -1.4278790253808875e+01, 2.1302268283304372e+01, -2.2193853972079314e+01, 2.0873499791105537e+01, -1.3709764520609468e+01, 8.1303553577932188e+00, -2.8201643879900726e+00, 9.0332828533800769e-01},
{-1.4071749185996747e+00, 5.6904141470697471e+00, -5.7374718273676217e+00, 1.1958028362868873e+01, -8.5435280598354382e+00, 1.1717345583835918e+01, -5.5088290876998407e+00, 5.3536787286077372e+00, -1.2972519209655518e+00, 9.0332828533799414e-01},
{8.2010906117760141e-01, 5.1673756579268559e+00, 3.2580350909220819e+00, 1.0392903763919172e+01, 4.8101776408668879e+00, 1.0183724507092480e+01, 3.1282000712126603e+00, 4.8615933365571822e+00, 7.5604535083144497e-01, 9.0332828533799658e-01},
{2.7080869856154512e+00, 7.8319071217995688e+00, 1.2201607990980744e+01, 1.8651500443681620e+01, 1.8758157568004549e+01, 1.8276088095999022e+01, 1.1715361303018897e+01, 7.3684394621253499e+00, 2.4965418284511904e+00, 9.0332828533800436e-01},
{4.9479835250075892e+00, 1.4691607003177602e+01, 2.9082414772101060e+01, 4.3179839108869331e+01, 4.8440791644688879e+01, 4.2310703962394342e+01, 2.7923434247706432e+01, 1.3822186510471010e+01, 4.5614664160654357e+00, 9.0332828533799958e-01},
{6.1701893352279846e+00, 2.0127225876810336e+01, 4.2974193398071684e+01, 6.5958045321253451e+01, 7.5230437667866596e+01, 6.4630411355739852e+01, 4.1261591079244127e+01, 1.8936128791950534e+01, 5.6881982915180291e+00, 9.0332828533799803e-01},
{7.4092912870072398e+00, 2.6857944460290135e+01, 6.1578787811202247e+01, 9.8258255839887312e+01, 1.1359460153696298e+02, 9.6280452143026082e+01, 5.9124742025776392e+01, 2.5268527576524203e+01, 6.8305064480743081e+00, 9.0332828533799969e-01},
{8.5743055776347692e+00, 3.4306584753117889e+01, 8.4035290411037053e+01, 1.3928510844056814e+02, 1.6305115418161620e+02, 1.3648147221895786e+02, 8.0686288623299745e+01, 3.2276361903872115e+01, 7.9045143816244696e+00, 9.0332828533799636e-01}
};

const static double iirBCoefficientConstants[FILTER_IIR_FILTER_COUNT][IIR_B_COEFFICIENT_COUNT] = {  //The B-coefficients used for each IIR filter. Each row corresponds to a different filter.
{9.0928532700696364e-10, -0.0000000000000000e+00, -4.5464266350348181e-09, -0.0000000000000000e+00, 9.0928532700696362e-09, -0.0000000000000000e+00, -9.0928532700696362e-09, -0.0000000000000000e+00, 4.5464266350348181e-09, -0.0000000000000000e+00, -9.0928532700696364e-10},
{9.0928698045638776e-10, 0.0000000000000000e+00, -4.5464349022819393e-09, 0.0000000000000000e+00, 9.0928698045638787e-09, 0.0000000000000000e+00, -9.0928698045638787e-09, 0.0000000000000000e+00, 4.5464349022819393e-09, 0.0000000000000000e+00, -9.0928698045638776e-10},
{9.0928689298097020e-10, 0.0000000000000000e+00, -4.5464344649048510e-09, 0.0000000000000000e+00, 9.0928689298097020e-09, 0.0000000000000000e+00, -9.0928689298097020e-09, 0.0000000000000000e+00, 4.5464344649048510e-09, 0.0000000000000000e+00, -9.0928689298097020e-10},
{9.0928696329154157e-10, 0.0000000000000000e+00, -4.5464348164577090e-09, 0.0000000000000000e+00, 9.0928696329154180e-09, 0.0000000000000000e+00, -9.0928696329154180e-09, 0.0000000000000000e+00, 4.5464348164577090e-09, 0.0000000000000000e+00, -9.0928696329154157e-10},
{9.0928648497937087e-10, 0.0000000000000000e+00, -4.5464324248968538e-09, 0.0000000000000000e+00, 9.0928648497937076e-09, 0.0000000000000000e+00, -9.0928648497937076e-09, 0.0000000000000000e+00, 4.5464324248968538e-09, 0.0000000000000000e+00, -9.0928648497937087e-10},
{9.0928645119506948e-10, -0.0000000000000000e+00, -4.5464322559753479e-09, -0.0000000000000000e+00, 9.0928645119506958e-09, -0.0000000000000000e+00, -9.0928645119506958e-09, -0.0000000000000000e+00, 4.5464322559753479e-09, -0.0000000000000000e+00, -9.0928645119506948e-10},
{9.0928343368482748e-10, -0.0000000000000000e+00, -4.5464171684241375e-09, -0.0000000000000000e+00, 9.0928343368482750e-09, -0.0000000000000000e+00, -9.0928343368482750e-09, -0.0000000000000000e+00, 4.5464171684241375e-09, -0.0000000000000000e+00, -9.0928343368482748e-10},
{9.0929508683806034e-10, 0.0000000000000000e+00, -4.5464754341903021e-09, 0.0000000000000000e+00, 9.0929508683806042e-09, 0.0000000000000000e+00, -9.0929508683806042e-09, 0.0000000000000000e+00, 4.5464754341903021e-09, 0.0000000000000000e+00, -9.0929508683806034e-10},
{9.0926783827278939e-10, 0.0000000000000000e+00, -4.5463391913639461e-09, 0.0000000000000000e+00, 9.0926783827278922e-09, 0.0000000000000000e+00, -9.0926783827278922e-09, 0.0000000000000000e+00, 4.5463391913639461e-09, 0.0000000000000000e+00, -9.0926783827278939e-10},
{9.0906302220838671e-10, 0.0000000000000000e+00, -4.5453151110419338e-09, 0.0000000000000000e+00, 9.0906302220838675e-09, 0.0000000000000000e+00, -9.0906302220838675e-09, 0.0000000000000000e+00, 4.5453151110419338e-09, 0.0000000000000000e+00, -9.0906302220838671e-10}
};
 
// Filtering routines for the laser-tag project.
// Filtering is performed by a two-stage filter, as described below.
 
// 1. First filter is a decimating FIR filter with a configurable number of taps and decimation factor.
// 2. The output from the decimating FIR filter is passed through a bank of 10 IIR filters. The
// characteristics of the IIR filter are fixed.
 
/*********************************************************************************************************
****************************************** Main Filter Functions *****************************************
**********************************************************************************************************/
 
//Helper function which initializes an x-queue and fills it with 0's
void initXQueue(){
    queue_init(&x_queue, FILTER_XQUEUE_SIZE, "X_QUEUE");//initialize queue in memory, with correct size and name
    filter_fillQueue(&x_queue, FILTER_INIT_VAL);        //fill the queue with 0's
}

//Helper function which initializes a y-queue and fills it with 0's
void initYQueue(){
    queue_init(&y_queue, FILTER_YQUEUE_SIZE, "Y_QUEUE");//initialize queue in memory, with correct size and name
    filter_fillQueue(&y_queue, FILTER_INIT_VAL);        //fill the queue with 0's
}

//Helper function which initializes all the z-queues and initializes them with 0's
void initZQueues(){
    for (uint8_t i = RESET; i < FILTER_IIR_FILTER_COUNT; i++)           //Loop through the z-queue for each IIR filter
    {
        char filter_name[FILTER_NAME_SIZE];                         //generate a name for the queue
        sprintf(filter_name, "Z_QUEUE_%u", i);
        queue_init(&(z_queue[i]), FILTER_ZQUEUE_SIZE, filter_name); //initialize queue in memory, with correct size and name
        filter_fillQueue(&(z_queue[i]), FILTER_INIT_VAL);           //fill the queue with 0's
    }
}

//Helper function which initializes all the output-queues and initializes them with 0's
void initOutputQueues(){
    for (uint8_t i = RESET; i < FILTER_IIR_FILTER_COUNT; i++)                       //Loop through the output-queue for each IIR filter
    {
        char filter_name[FILTER_NAME_SIZE];                                     //generate a name for the queue
        sprintf(filter_name, "OUTPUT_QUEUE_%u", i);								//puts the output queue name into the string filter_name
        queue_init(&(output_queue[i]), FILTER_OUTPUTQUEUE_SIZE + FILTER_OUTPUTQUEUE_OLDVAL_OFFSET, filter_name);   //initialize queue in memory, with correct size and name
        filter_fillQueue(&(output_queue[i]), FILTER_INIT_VAL);                  //fill the queue with 0's
    }
}

// Must call this prior to using any filter functions.
void filter_init()
{
    initXQueue();       // create x,y,z's, and output's queue's
    initYQueue();       // and initialize the queues with their own helper functions
    initZQueues();      // and set the values in the queues to 0
    initOutputQueues();												
    for (uint8_t i = RESET; i < FILTER_IIR_FILTER_COUNT; i++)   //Loop through the array which keeps track of the most recently calculated power value
    {
        current_power_vals[i] = FILTER_INIT_VAL;                //Initialize the most recently calculated power for each IIR filter to 0.
    }
}
 


// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x)
{
    queue_overwritePush(&x_queue, x);    //Force push the value onto the x-queue
}
 
// Fills a queue with the given fillValue. For example,
// if the queue is of size 10, and the fillValue = 1.0,
// after executing this function, the queue will contain 10 values
// all of them 1.0.
void filter_fillQueue(queue_t* q, double fillValue)
{
    for(queue_index_t i = RESET; i < queue_size(q); i++){   //loop for the size of the given queue
       queue_overwritePush(q, fillValue);                   //Overwrite the given fillValue onto the queue
    }
}
 
// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter()
{
    double filtered_val = FILTER_INIT_VAL;                      //Initialize the output value
    for (queue_index_t i = RESET; i < FIR_FILTER_TAP_COUNT; i++)//Loop through each value in the coeffecient array and XQueue
    {
        filtered_val += firCoefficients[i] * queue_readElementAt(&x_queue, FIR_FILTER_LAST_INDEX - i);  //Convolve
    }
    queue_overwritePush(&y_queue, filtered_val);                //Push the filtered value onto the yQueue
    return filtered_val;                                        //Return the filtered value
}
 
// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber)
{
    double output = FILTER_INIT_VAL;                                //Initialize the output value to 0
    for (queue_index_t i = RESET; i < IIR_B_COEFFICIENT_COUNT; i++) //Loop through each value in the B coefficient array and yQueue
    {
        output += iirBCoefficientConstants[filterNumber][i] * queue_readElementAt(&y_queue, IIR_FILTER_LAST_B_INDEX - i);                   //Multiply and sum to the total
    }
    for (queue_index_t i = RESET; i < IIR_A_COEFFICIENT_COUNT; i++) //Loop through each value in the A coefficient array and zQueue
    {
        output -= iirACoefficientConstants[filterNumber][i] * queue_readElementAt(&(z_queue[filterNumber]), IIR_FILTER_LAST_A_INDEX - i);   //Multiply and subtract from the total
    }
    queue_overwritePush(&(z_queue[filterNumber]), output);          //Push the output onto the z-queue for future iterations of iirFilter()
    queue_overwritePush(&(output_queue[filterNumber]), output);     //Push the output onto the outputQueue for future calculations of power
    return output;                                                  //Return the filtered output value
}
 
// Use this to compute the power for values contained in an outputQueue.
// If force == true, then recompute power by using all values in the outputQueue.
// This option is necessary so that you can correctly compute power values the first time.
// After that, you can incrementally compute power values by:
// 1. Keeping track of the power computed in a previous run, call this prev-power.
// 2. Keeping track of the oldest outputQueue value used in a previous run, call this oldest-value.
// 3. Get the newest value from the power queue, call this newest-value.
// 4. Compute new power as: prev-power - (oldest-value * oldest-value) + (newest-value * newest-value).
// Note that this function will probably need an array to keep track of these values for each
// of the 10 output queues.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch, bool debugPrint)
{
    double power = FILTER_INIT_VAL;                     // Initialize the power variable for calculation
    queue_t* q = &(output_queue[filterNumber]);         // Get the address of the queue we want to access
    if (forceComputeFromScratch)                        // If we need to compute from scratch
    {
        for (queue_index_t i = FILTER_OUTPUTQUEUE_OLDVAL_OFFSET; i < queue_size(q); i++)  // Loop through each queue value, skipping the old value
        {
            queue_data_t power_val = queue_readElementAt(q, i); //Get the power value from the output queue at the index i
            power += power_val * power_val;                     //Take each squared value, and add it to the sum
        }
    }
    else                                                //Otherwise, if we are NOT computing from scratch
    {
        power = current_power_vals[filterNumber];                           //Get the current power value
        queue_data_t new_element = queue_readElementAt(q, FILTER_OUTPUTQUEUE_NEWVAL_INDEX); //Get the output most recently stored
        queue_data_t old_element = queue_readElementAt(q, FILTER_OUTPUTQUEUE_OLDVAL_INDEX); //Get the oldest output
        power -= old_element*old_element;                                   //Subtract the oldest output squared (the one that was recently popped off)
        power += new_element*new_element;                                   //Add on the newest output squared (the one that was most recently pushed on)
    }
    current_power_vals[filterNumber] = power;
    return power;   //Return the calculated power
}
 
// Returns the last-computed output power value for the IIR filter [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber)
{
   return current_power_vals[filterNumber];
}
 
// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared array
// so that they can be accessed from outside the filter software by the detector.
// Remember that when you pass an array into a C function, changes to the array within
// that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[])
{
    for(uint8_t i = RESET; i < FILTER_IIR_FILTER_COUNT ; i++){//takes an array that was passed in and copies the contents of the power queue we want to remain private to the passed in queue
        powerValues[i] = current_power_vals[i];
    }
}
 
// Using the previously-computed power values that are current stored in currentPowerValue[] array,
// Copy these values into the normalizedArray[] argument and then normalize them by dividing
// all of the values in normalizedArray by the maximum power value contained in currentPowerValue[].
void filter_getNormalizedPowerValues(double normalizedArray[], uint16_t* indexOfMaxValue)
{
    double largest_val = FILTER_INIT_VAL;						//sets the initial value to 0
    for(uint16_t i = RESET; i < FILTER_IIR_FILTER_COUNT; i++){	//sets up the for loop to iterate
        normalizedArray[i] = current_power_vals[i];				//copies the contents from current_power_vals to normalizedArray
        if(normalizedArray[i] > largest_val){ 					//finds the largest value in the array so far
            largest_val = normalizedArray[i];					//copies the value to the variable
        }
    }

    for(uint16_t i = RESET; i < FILTER_IIR_FILTER_COUNT; i++){ 	//runs through the copied array
        normalizedArray[i] = normalizedArray[i]/largest_val;	//normalizes the largest value to be 1 and sets everything else to a fraction thereof
    }
}
 
/*********************************************************************************************************
********************************** Verification-assisting functions. *************************************
********* Test functions access the internal data structures of the filter.c via these functions. ********
*********************** These functions are not used by the main filter functions. ***********************
**********************************************************************************************************/
 
// Returns the array of FIR coefficients.
const double* filter_getFirCoefficientArray()
{
    return firCoefficients;
}
 
// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount()
{
    return FIR_FILTER_TAP_COUNT;
}
 
// Returns the array of coefficients for a particular filter number.
const double* filter_getIirACoefficientArray(uint16_t filterNumber)
{
    return iirACoefficientConstants[filterNumber];
}
 
// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount()
{
    return IIR_A_COEFFICIENT_COUNT;
}
 
// Returns the array of coefficients for a particular filter number.
const double* filter_getIirBCoefficientArray(uint16_t filterNumber)
{
    return iirBCoefficientConstants[filterNumber];
}
 
// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount()
{
    return IIR_B_COEFFICIENT_COUNT;
}
 
// Returns the size of the yQueue.
uint32_t filter_getYQueueSize()
{
    return FILTER_YQUEUE_SIZE;
}
 
// Returns the decimation value.
uint16_t filter_getDecimationValue()
{
    return DECIMATION_VALUE;
}
 
// Returns the address of xQueue.
queue_t* filter_getXQueue()
{
    return &x_queue;
}
 
// Returns the address of yQueue.
queue_t* filter_getYQueue()
{
    return &y_queue;
}
 
// Returns the address of zQueue for a specific filter number.
queue_t* filter_getZQueue(uint16_t filterNumber)
{
    return &(z_queue[filterNumber]); 
}
 
// Returns the address of the IIR output-queue for a specific filter-number.
queue_t* filter_getIirOutputQueue(uint16_t filterNumber)
{
    return &(output_queue[filterNumber]);
}
