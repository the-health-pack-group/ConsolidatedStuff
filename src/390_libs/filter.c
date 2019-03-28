
#include "filter.h"
#include <stdio.h>

#define QUEUE_ARRAY_SIZE 10
#define FIR_COEFF_COUNT 81
#define IIR_A_COEFF_COUNT 10
#define IIR_B_COEFF_COUNT 11


#define X_QUEUE_SIZE 81
#define Y_QUEUE_SIZE 11
#define Z_QUEUE_SIZE 10
#define OUTPUT_QUEUE_SIZE 2000

#define OLDEST_OUTPUT_ELEMENT 0
#define NEWEST_OUTPUT_ELEMENT OUTPUT_QUEUE_SIZE -1

#define QUEUE_INIT_VALUE 0
#define QUEUE_SIZE_OFFSET 1 // used when we subtract 1

static queue_t xQueue;
static queue_t yQueue;


static queue_t zQueueArray[QUEUE_ARRAY_SIZE];
static queue_t outputQueueArray[QUEUE_ARRAY_SIZE];
static double currentPowerValue[QUEUE_ARRAY_SIZE];

//We use this array to optimize power computation
static double oldestOutputValue[QUEUE_ARRAY_SIZE];

const static double fir_coeffs[FIR_COEFF_COUNT] = {
   6.0546138291252597e-04,
   5.2507143315267811e-04,
   3.8449091272701525e-04,
   1.7398667197948182e-04,
  -1.1360489934931548e-04,
  -4.7488111478632532e-04,
  -8.8813878356223768e-04,
  -1.3082618178394971e-03,
  -1.6663618496969908e-03,
  -1.8755700366336781e-03,
  -1.8432363328817916e-03,
  -1.4884258721727399e-03,
  -7.6225514924622853e-04,
   3.3245249132384837e-04,
   1.7262548802593762e-03,
   3.2768418720744217e-03,
   4.7744814146589041e-03,
   5.9606317814670249e-03,
   6.5591485566565593e-03,
   6.3172870282586493e-03,
   5.0516421324586546e-03,
   2.6926388909554420e-03,
  -6.7950808883015244e-04,
  -4.8141100026888716e-03,
  -9.2899200683230643e-03,
  -1.3538595939086505e-02,
  -1.6891587875325020e-02,
  -1.8646984919441702e-02,
  -1.8149697899123560e-02,
  -1.4875876924586697e-02,
  -8.5110608557150517e-03,
   9.8848931927316319e-04,
   1.3360421141947857e-02,
   2.8033301291042201e-02,
   4.4158668590312596e-02,
   6.0676486642862550e-02,
   7.6408062643700314e-02,
   9.0166807112971648e-02,
   1.0087463525509034e-01,
   1.0767073207825099e-01,
   1.1000000000000000e-01,
   1.0767073207825099e-01,
   1.0087463525509034e-01,
   9.0166807112971648e-02,
   7.6408062643700314e-02,
   6.0676486642862550e-02,
   4.4158668590312596e-02,
   2.8033301291042201e-02,
   1.3360421141947857e-02,
   9.8848931927316319e-04,
  -8.5110608557150517e-03,
  -1.4875876924586697e-02,
  -1.8149697899123560e-02,
  -1.8646984919441702e-02,
  -1.6891587875325020e-02,
  -1.3538595939086505e-02,
  -9.2899200683230643e-03,
  -4.8141100026888716e-03,
  -6.7950808883015244e-04,
   2.6926388909554420e-03,
   5.0516421324586546e-03,
   6.3172870282586493e-03,
   6.5591485566565593e-03,
   5.9606317814670249e-03,
   4.7744814146589041e-03,
   3.2768418720744217e-03,
   1.7262548802593762e-03,
   3.3245249132384837e-04,
  -7.6225514924622853e-04,
  -1.4884258721727399e-03,
  -1.8432363328817916e-03,
  -1.8755700366336781e-03,
  -1.6663618496969908e-03,
  -1.3082618178394971e-03,
  -8.8813878356223768e-04,
  -4.7488111478632532e-04,
  -1.1360489934931548e-04,
   1.7398667197948182e-04,
   3.8449091272701525e-04,
   5.2507143315267811e-04,
   6.0546138291252597e-04
};

const static double iir_a_coeffs[FILTER_FREQUENCY_COUNT][IIR_A_COEFF_COUNT] = {
    { -5.9637727070164015e+00, 1.9125339333078248e+01, -4.0341474540744173e+01, 6.1537466875368821e+01, -7.0019717951472188e+01, 6.0298814235238872e+01, -3.8733792862566290e+01, 1.7993533279581058e+01, -5.4979061224867651e+00, 9.0332828533799547e-01 },
    { -4.6377947119071443e+00, 1.3502215749461572e+01, -2.6155952405269755e+01, 3.8589668330738348e+01, -4.3038990303252632e+01, 3.7812927599537133e+01, -2.5113598088113793e+01, 1.2703182701888094e+01, -4.2755083391143520e+00, 9.0332828533800291e-01 },
    { -3.0591317915750960e+00, 8.6417489609637634e+00, -1.4278790253808875e+01, 2.1302268283304372e+01, -2.2193853972079314e+01, 2.0873499791105537e+01, -1.3709764520609468e+01, 8.1303553577932188e+00, -2.8201643879900726e+00, 9.0332828533800769e-01 },
    { -1.4071749185996747e+00, 5.6904141470697471e+00, -5.7374718273676217e+00, 1.1958028362868873e+01, -8.5435280598354382e+00, 1.1717345583835918e+01, -5.5088290876998407e+00, 5.3536787286077372e+00, -1.2972519209655518e+00, 9.0332828533799414e-01},
    { 8.2010906117760141e-01, 5.1673756579268559e+00, 3.2580350909220819e+00, 1.0392903763919172e+01, 4.8101776408668879e+00, 1.0183724507092480e+01, 3.1282000712126603e+00, 4.8615933365571822e+00, 7.5604535083144497e-01, 9.0332828533799658e-01},
    { 2.7080869856154512e+00, 7.8319071217995688e+00, 1.2201607990980744e+01, 1.8651500443681620e+01, 1.8758157568004549e+01, 1.8276088095999022e+01, 1.1715361303018897e+01, 7.3684394621253499e+00, 2.4965418284511904e+00, 9.0332828533800436e-01},
    { 4.9479835250075892e+00, 1.4691607003177602e+01, 2.9082414772101060e+01, 4.3179839108869331e+01, 4.8440791644688879e+01, 4.2310703962394342e+01, 2.7923434247706432e+01, 1.3822186510471010e+01, 4.5614664160654357e+00, 9.0332828533799958e-01},
    { 6.1701893352279846e+00, 2.0127225876810336e+01, 4.2974193398071684e+01, 6.5958045321253451e+01, 7.5230437667866596e+01, 6.4630411355739852e+01, 4.1261591079244127e+01, 1.8936128791950534e+01, 5.6881982915180291e+00, 9.0332828533799803e-01},
    { 7.4092912870072398e+00, 2.6857944460290135e+01, 6.1578787811202247e+01, 9.8258255839887312e+01, 1.1359460153696298e+02, 9.6280452143026082e+01, 5.9124742025776392e+01, 2.5268527576524203e+01, 6.8305064480743081e+00, 9.0332828533799969e-01},
    { 8.5743055776347692e+00, 3.4306584753117889e+01, 8.4035290411037053e+01, 1.3928510844056814e+02, 1.6305115418161620e+02, 1.3648147221895786e+02, 8.0686288623299745e+01, 3.2276361903872115e+01, 7.9045143816244696e+00, 9.0332828533799636e-01}
};

const static double iir_b_coeffs[FILTER_FREQUENCY_COUNT][IIR_B_COEFF_COUNT] = {
    {9.0928532700696364e-10, -0.0000000000000000e+00, -4.5464266350348181e-09, -0.0000000000000000e+00, 9.0928532700696362e-09, -0.0000000000000000e+00, -9.0928532700696362e-09, -0.0000000000000000e+00, 4.5464266350348181e-09, -0.0000000000000000e+00, -9.0928532700696364e-10},
    {9.0928698045638776e-10, 0.0000000000000000e+00, -4.5464349022819393e-09, 0.0000000000000000e+00, 9.0928698045638787e-09, 0.0000000000000000e+00, -9.0928698045638787e-09, 0.0000000000000000e+00, 4.5464349022819393e-09, 0.0000000000000000e+00, -9.0928698045638776e-10 },
    {9.0928689298097020e-10, 0.0000000000000000e+00, -4.5464344649048510e-09, 0.0000000000000000e+00, 9.0928689298097020e-09, 0.0000000000000000e+00, -9.0928689298097020e-09, 0.0000000000000000e+00, 4.5464344649048510e-09, 0.0000000000000000e+00, -9.0928689298097020e-10 },
    {9.0928696329154157e-10, 0.0000000000000000e+00, -4.5464348164577090e-09, 0.0000000000000000e+00, 9.0928696329154180e-09, 0.0000000000000000e+00, -9.0928696329154180e-09, 0.0000000000000000e+00, 4.5464348164577090e-09, 0.0000000000000000e+00, -9.0928696329154157e-10 },
    {9.0928648497937087e-10, 0.0000000000000000e+00, -4.5464324248968538e-09, 0.0000000000000000e+00, 9.0928648497937076e-09, 0.0000000000000000e+00, -9.0928648497937076e-09, 0.0000000000000000e+00, 4.5464324248968538e-09, 0.0000000000000000e+00, -9.0928648497937087e-10 },
    {9.0928645119506948e-10, -0.0000000000000000e+00, -4.5464322559753479e-09, -0.0000000000000000e+00, 9.0928645119506958e-09, -0.0000000000000000e+00, -9.0928645119506958e-09, -0.0000000000000000e+00, 4.5464322559753479e-09, -0.0000000000000000e+00, -9.0928645119506948e-10 },
    {9.0928343368482748e-10, -0.0000000000000000e+00, -4.5464171684241375e-09, -0.0000000000000000e+00, 9.0928343368482750e-09, -0.0000000000000000e+00, -9.0928343368482750e-09, -0.0000000000000000e+00, 4.5464171684241375e-09, -0.0000000000000000e+00, -9.0928343368482748e-10 },
    {9.0929508683806034e-10, 0.0000000000000000e+00, -4.5464754341903021e-09, 0.0000000000000000e+00, 9.0929508683806042e-09, 0.0000000000000000e+00, -9.0929508683806042e-09, 0.0000000000000000e+00, 4.5464754341903021e-09, 0.0000000000000000e+00, -9.0929508683806034e-10 },
    {9.0926783827278939e-10, 0.0000000000000000e+00, -4.5463391913639461e-09, 0.0000000000000000e+00, 9.0926783827278922e-09, 0.0000000000000000e+00, -9.0926783827278922e-09, 0.0000000000000000e+00, 4.5463391913639461e-09, 0.0000000000000000e+00, -9.0926783827278939e-10 },
    {9.0906302220838671e-10, 0.0000000000000000e+00, -4.5453151110419338e-09, 0.0000000000000000e+00, 9.0906302220838675e-09, 0.0000000000000000e+00, -9.0906302220838675e-09, 0.0000000000000000e+00, 4.5453151110419338e-09, 0.0000000000000000e+00, -9.0906302220838671e-10 }
};









void initXQueues() {

    queue_init(&(xQueue), X_QUEUE_SIZE,"xQueue");
    for (uint32_t j=0; j<X_QUEUE_SIZE; j++)
        queue_overwritePush(&(xQueue), QUEUE_INIT_VALUE);
}


void initYQueues() {

    queue_init(&(yQueue), Y_QUEUE_SIZE,"yQueue");
    for (uint32_t j=0; j<Y_QUEUE_SIZE; j++)
        queue_overwritePush(&(yQueue), QUEUE_INIT_VALUE);
}




void initZQueues() {
  for (uint32_t i=0; i<FILTER_FREQUENCY_COUNT; i++) {
    queue_init(&(zQueueArray[i]), Z_QUEUE_SIZE,"zQueue");
    for (uint32_t j=0; j<Z_QUEUE_SIZE; j++)
     queue_overwritePush(&(zQueueArray[i]), QUEUE_INIT_VALUE);
  }
}

void initOutputQueues() {
  for (uint32_t i=0; i<FILTER_FREQUENCY_COUNT; i++) {
    queue_init(&(outputQueueArray[i]), OUTPUT_QUEUE_SIZE,"outputQueue");
    for (uint32_t j=0; j<OUTPUT_QUEUE_SIZE; j++)
     queue_overwritePush(&(outputQueueArray[i]), QUEUE_INIT_VALUE);
  }
}


// Must call this prior to using any filter functions.
void filter_init()
{

    initXQueues();
    initYQueues();
    initZQueues();
    initOutputQueues();

}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x)
{
    queue_overwritePush(&(xQueue), x);
}

// Fills a queue with the given fillValue. For example,
// if the queue is of size 10, and the fillValue = 1.0,
// after executing this function, the queue will contain 10 values
// all of them 1.0.
void filter_fillQueue(queue_t* q, double fillValue)
{
    for(queue_index_t i =0; i<queue_size(q); i++)
    {
        queue_overwritePush(q, fillValue);
    }
}

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter()
{
  double val = 0;
  for(queue_index_t i =0; i<X_QUEUE_SIZE; i++)
  {
      val += (queue_readElementAt(&xQueue,X_QUEUE_SIZE -i -QUEUE_SIZE_OFFSET) * fir_coeffs[i]);
  }

  queue_overwritePush(&(yQueue),val);

  return val;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber)
{
    double val = 0;

    for(queue_index_t i =0; i< Y_QUEUE_SIZE; i++)
    {
        val += iir_b_coeffs[filterNumber][i] * queue_readElementAt(&(yQueue),Y_QUEUE_SIZE - i - QUEUE_SIZE_OFFSET);
    }

    for(queue_index_t j =0; j< Z_QUEUE_SIZE; j++)
    {
        val -= iir_a_coeffs[filterNumber][j] * queue_readElementAt(&(zQueueArray[filterNumber]),Z_QUEUE_SIZE - j- QUEUE_SIZE_OFFSET);
    }

    queue_overwritePush(&(zQueueArray[filterNumber]),val);

    queue_overwritePush(&(outputQueueArray[filterNumber]),val); // We added this one!!

    return val;


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
    if(forceComputeFromScratch)
    {
        double power = 0;
        for(queue_index_t i =0; i<OUTPUT_QUEUE_SIZE; i++)
        {
            double current_power = queue_readElementAt(&(outputQueueArray[filterNumber]),i);
            power += (current_power*current_power);
        }
        currentPowerValue[filterNumber] = power;

    }

    //Optimized version
    else
    {
        // Here we are removing the square of the oldest output value from the currentPowerValue array
        double oldest = (oldestOutputValue[filterNumber]);
        currentPowerValue[filterNumber] -= (oldest*oldest);

        // Here we are adding the square of the newest output value from the currentPowerValue array.
        double newest = queue_readElementAt(&(outputQueueArray[filterNumber]),NEWEST_OUTPUT_ELEMENT);
        currentPowerValue[filterNumber] += (newest*newest);
    }

    //Oldest value
    oldestOutputValue[filterNumber] = queue_readElementAt(&(outputQueueArray[filterNumber]),OLDEST_OUTPUT_ELEMENT);
    return filter_getCurrentPowerValue(filterNumber);
}

// Returns the last-computed output power value for the IIR filter [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber)
{
    return currentPowerValue[filterNumber];
}

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared array
// so that they can be accessed from outside the filter software by the detector.
// Remember that when you pass an array into a C function, changes to the array within
// that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[])
{
    for (queue_index_t i =0; i<QUEUE_ARRAY_SIZE; i++)
    {
        powerValues[i] = currentPowerValue[i];
    }
}

// Using the previously-computed power values that are current stored in currentPowerValue[] array,
// Copy these values into the normalizedArray[] argument and then normalize them by dividing
// all of the values in normalizedArray by the maximum power value contained in currentPowerValue[].
void filter_getNormalizedPowerValues(double normalizedArray[], uint16_t* indexOfMaxValue)
{

    //Store maxValue so we can use it later
    double maxValue = 0;

    //For each element in the power array
    for (queue_index_t i =0; i<QUEUE_ARRAY_SIZE; i++)
    {
        //Copy values into normalized array
        normalizedArray[i] = currentPowerValue[i];

        //Keep track of max value
        if (maxValue < normalizedArray[i]) {
            maxValue = normalizedArray[i];
            //Write the index of the max value to the provided location
            *indexOfMaxValue = i;
        }
    }

    //Normalize the values
    for (queue_index_t i =0; i<QUEUE_ARRAY_SIZE; i++)
    {
        normalizedArray[i] /= maxValue;
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
   return fir_coeffs;
}

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount()
{
    return FIR_COEFF_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double* filter_getIirACoefficientArray(uint16_t filterNumber)
{
    return iir_a_coeffs[filterNumber];
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount()
{
    return IIR_A_COEFF_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double* filter_getIirBCoefficientArray(uint16_t filterNumber)
{
    return iir_b_coeffs[filterNumber];
}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount()
{
    return IIR_B_COEFF_COUNT;
}

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize()
{
 return Y_QUEUE_SIZE;
}

// Returns the decimation value.
uint16_t filter_getDecimationValue()
{
    return FILTER_FIR_DECIMATION_FACTOR;
}

// Returns the address of xQueue.
queue_t* filter_getXQueue()
{
    return &xQueue;
}

// Returns the address of yQueue.
queue_t* filter_getYQueue()
{

    return &yQueue;
}

// Returns the address of zQueue for a specific filter number.
queue_t* filter_getZQueue(uint16_t filterNumber)
{
    return &(zQueueArray[filterNumber]);
}

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t* filter_getIirOutputQueue(uint16_t filterNumber)
{
    return &(outputQueueArray[filterNumber]);
}

//void filter_runTest();




























