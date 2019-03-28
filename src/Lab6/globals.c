#include "globals.h"


// The length of the sequence.
// The static keyword means that globals_sequenceLength can only be accessed
// by functions contained in this file.
static uint16_t globals_SequenceLength = 0;  // The length of the sequence.

//Global Sequence
static uint8_t globals_Sequence [GLOBALS_MAX_FLASH_SEQUENCE] = {0};

//Global sequence iteration length
static uint16_t globals_SequenceIterationLength = 0;


//Sets the sequence to a random value and length
void globals_setSequence(const uint8_t sequence[], uint16_t length)
{
	//Loop until the size of the sequence is complete
    for(uint8_t i = 0; i < length; i++)
    {
		//Set the sequence to the passed in random sequence
        globals_Sequence[i] = sequence[i]; 
    }

	//Update sequence length
    globals_SequenceLength = length; 

}

// This returns the value of the sequence at the index.
uint8_t globals_getSequenceValue(uint16_t index)
{
    return globals_Sequence[index];
}

// Retrieve the sequence length.
uint16_t globals_getSequenceLength()
{
    return globals_SequenceLength;
}

//Sets the current sequence iteration length
void globals_setSequenceIterationLength(uint16_t length)
{
    globals_SequenceIterationLength = length;
}

//Returns current sequence iteration length
uint16_t globals_getSequenceIterationLength()
{
    return globals_SequenceIterationLength;
}


