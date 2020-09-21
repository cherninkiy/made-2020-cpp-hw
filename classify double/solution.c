#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>



/**
 * Library-level functions.
 * You should use them in the main sections.
 */

uint64_t convertToUint64 (double number) {
    return *((uint64_t *)(&number));
}

double convertToDouble (uint64_t number) {
    return *((double *)(&number));
}

bool getBit (const uint64_t number, const uint8_t index) {
    return (number >> index) & 1 == 1;
}

uint64_t setBit (const uint64_t number, const uint8_t index) {
    return number | (1ull << index);
}

bool checkSignBit(uint64_t number) {
    return getBit(number, 63);
}

bool checkQuietBit(uint64_t number) {
    return getBit(number, 51);
}

bool checkBitsAny(uint64_t number, uint64_t mask) {
    return (number & mask) != 0ull;
}

bool checkBitsAll(uint64_t number, uint64_t mask) {
    return (number & mask) == mask;
}

const uint64_t EXPONENT_BITS = 0x7FF0000000000000;
const uint64_t FRACTION_BITS = 0x000FFFFFFFFFFFFF;


/**
 * Checkers here:
 */

bool checkForPlusZero (uint64_t number) {
    return number == 0ull;
}

bool checkForMinusZero (uint64_t number) {
    return number == 0x8000000000000000;
}

bool checkForPlusInf (uint64_t number) {
    return !checkSignBit(number) && checkBitsAll(number, EXPONENT_BITS)
        && !checkBitsAny(number, FRACTION_BITS);
}

bool checkForMinusInf (uint64_t number) {
    return checkSignBit(number) && checkBitsAll(number, EXPONENT_BITS)
        && !checkBitsAny(number, FRACTION_BITS);
}

bool checkForPlusNormal (uint64_t number) {
    return !checkSignBit(number) && checkBitsAny(number, EXPONENT_BITS) 
        && !checkBitsAll(number, EXPONENT_BITS);
}

bool checkForMinusNormal (uint64_t number) {
    return checkSignBit(number) && checkBitsAny(number, EXPONENT_BITS) 
        && !checkBitsAll(number, EXPONENT_BITS);
}

bool checkForPlusDenormal (uint64_t number) {
    return !checkSignBit(number) && !checkBitsAny(number, EXPONENT_BITS);
}

bool checkForMinusDenormal (uint64_t number) {
    return checkSignBit(number) && !checkBitsAny(number, EXPONENT_BITS);
}

bool checkForSignalingNan (uint64_t number) {
    return checkBitsAll(number, EXPONENT_BITS) && !checkQuietBit(number) 
        && checkBitsAny(number, FRACTION_BITS);
}

bool checkForQuietNan (uint64_t number) {
    return checkBitsAll(number, EXPONENT_BITS) && checkQuietBit(number);
}


void classify (double number) {
    if (checkForPlusZero(convertToUint64(number))) {
        printf("Plus zero\n");
    }

    else if (checkForMinusZero(convertToUint64(number))) {
        printf("Minus zero\n");
    }

    else if (checkForPlusInf(convertToUint64(number))) {
        printf("Plus inf\n");
    }

    else if (checkForMinusInf(convertToUint64(number))) {
        printf("Minus inf\n");
    }

    else if (checkForPlusNormal(convertToUint64(number))) {
        printf("Plus normal\n");
    }

    else if (checkForMinusNormal(convertToUint64(number))) {
        printf("Minus normal\n");
    }

    else if (checkForPlusDenormal(convertToUint64(number))) {
        printf("Plus Denormal\n");
    }

    else if (checkForMinusDenormal(convertToUint64(number))) {
        printf("Minus Denormal\n");
    }

    else if (checkForSignalingNan(convertToUint64(number))) {
        printf("Signailing NaN\n");
    }

    else if (checkForQuietNan(convertToUint64(number))) {
        printf("Quiet NaN\n");
    }

    else {
        printf("Error.\n");
    }
}
