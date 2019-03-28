/*
 * timerIsr.c
 *
 *  Created on: Jan 2, 2015
 *      Author: hutch
 */

#include "transmitter.h"
#include "supportFiles/interrupts.h"
#include "src/390_libs/queue.h"
#include "trigger.h"
#include "lockoutTimer.h"
#include "hitLedTimer.h"
#include <stdio.h>

// Keep track of how many times isr_function() is called.
//static uint64_t isr_totalXadcSampleCount = 0;

// This implements a dedicated buffer for storing values from the ADC
// until they are read and processed by detector().
// adcBuffer_t is similar to a queue.
#define ADC_BUFFER_SIZE 100000
typedef struct {
    uint32_t indexIn;   // New values go here.
    uint32_t indexOut;  // Pull old values from here.
    uint32_t data[ADC_BUFFER_SIZE];  // Store values here.
    uint32_t elementCount;  // Number of values contained in adcBuffer_t.
} adcBuffer_t;

// This is the instantiation of adcBuffer.
static adcBuffer_t adcBuffer;

// Init adcBuffer.
void adcBufferInit() {
    adcBuffer.indexIn = 0;
    adcBuffer.indexOut = 0;
    adcBuffer.elementCount = 0;
}

// Init everything in isr.
void isr_init() {
    adcBufferInit();  // init the local adcBuffer.
}

// Implemented as a fixed-size circular buffer.
// indexIn always points to an empty location (by definition).
// indexOut always points to the oldest element.
// buffer is empty if indexIn == indexOut. Buffer is full if incremented indexIn == indexOut
void isr_addDataToAdcBuffer(uint32_t adcData) {
    if (adcBuffer.elementCount < (ADC_BUFFER_SIZE-1)) // Increment the element count unless you are already full.
        adcBuffer.elementCount++;
    adcBuffer.data[adcBuffer.indexIn] = adcData;                    // write,
    adcBuffer.indexIn = (adcBuffer.indexIn + 1) % ADC_BUFFER_SIZE;  // then increment.
    if (adcBuffer.indexIn == adcBuffer.indexOut) {                  // If you are now pointing at the out pointer,
        adcBuffer.indexOut = (adcBuffer.indexOut + 1) % ADC_BUFFER_SIZE;  // move the out pointer up (essentially a pop).
    }
}

// Removes a single item from the ADC buffer.
// Does not signal an error if the ADC buffer is currently
// emptu. Simply returns a default value of 0 if the buffer is currently empty.
uint32_t isr_removeDataFromAdcBuffer() {
    uint32_t returnValue = 0;
    if (adcBuffer.indexIn == adcBuffer.indexOut)  // Just return 0 if empty.
        return 0;
    else {
        returnValue = adcBuffer.data[adcBuffer.indexOut];  // Not empty, get the return value from buffer.
        adcBuffer.indexOut = (adcBuffer.indexOut + 1) % ADC_BUFFER_SIZE;  // increment the out index.
        adcBuffer.elementCount--;  // One less element.
    }
    return returnValue;
}

// Functional interface to access element count.
uint32_t isr_adcBufferElementCount() {
    return adcBuffer.elementCount;
}

void isr_function() {
    uint32_t adcData = interrupts_getAdcData();
    isr_addDataToAdcBuffer(adcData);
    transmitter_tick();
    trigger_tick();
    lockoutTimer_tick();
    hitLedTimer_tick();
}
