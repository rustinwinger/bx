#ifndef FIXED_H
#define FIXED_H

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

/**
 *@brief Type definition for 8.8 fixed-point format
 */
typedef int16_t Fixed88;

/**
 *@brief Macro to read the lower byte from a WORD
 *@param x The WORD to read from
 */
#define READ_LOWER_BYTE_FROM_WORD(x) (*((volatile uint8_t*)(volatile void*)(x)))

/**
 *@brief Macro to read the upper byte from a WORD
 *@param x The WORD to read from
 */
#define READ_UPPER_BYTE_FROM_WORD(x) (*((volatile uint8_t*)(volatile void*)((x) + 1)))

/**
 *@brief Macro to convert a float to fixed-point format
 *@param floating The floating-point number to convert
 */
#define float_to_fixed(floating) ((Fixed88)((floating) * (1 << 8) + 0.5f))

/**
 *@brief Macro to convert a fixed-point number to a float
 *@param fixed The fixed-point number to convert
 */
#define fixed_to_float(fixed) ((float)(fixed) / (1 << 8))

/**
 *@brief Macro to define the fixed-point value of PI
 */
#define FIXED_PI (float_to_fixed(M_PI))

/**
 *@brief Macro to define the maximum value for fixed-point numbers
 */
#define FIXED_MAX (0x7FFF)

/**
 *@brief Macro to define the minimum value for fixed-point numbers
 */
#define FIXED_MIN ((int16_t)0x8000)

/**
 *@brief Macro to convert an integer to fixed-point format
 *@param n The integer to convert
 */
#define INT_TO_FIXED(n) ((n) << 8)

/**
 *@brief Converts an integer to fixed-point format
 *@param n Integer value to convert
 *@return Fixed-point representation of n
 */
extern inline Fixed88 intToFixed(int8_t n) {
  return n << 8;
}

/**
 *@brief Converts a fixed-point value to an integer
 *@param n Fixed-point value to convert
 *@return Integer representation of n
 */
extern inline int8_t fixedToInt(Fixed88 n) {
  return n >> 8;
}

/**
 *@brief Adds two fixed-point numbers
 *@param n First number
 *@param j Second number
 *@return Sum of n and j
 */
extern inline Fixed88 fixedAdd(Fixed88 n, Fixed88 j) {
  return n + j;
}

/**
 *@brief Subtracts two fixed-point numbers
 *@param n First number
 *@param j Second number
 *@return Difference between n and j
 */
extern inline Fixed88 fixedSub(Fixed88 n, Fixed88 j) {
  return n - j;
}

/**
 *@brief Multiplies two fixed-point numbers
 *@param n First number
 *@param j Second number
 *@return Product of n and j
 */
extern inline Fixed88 fixedMul(Fixed88 n, Fixed88 j) {
  int32_t r = (int32_t)n * (int32_t)j;
  return (Fixed88)(r >> 8);
}

/**
 *@brief Divides two fixed-point numbers with full accuracy which requires 32-bit division
 *@param n Numerator
 *@param j Denominator
 *@return Quotient of n and j
 */
extern inline Fixed88 fixedDiv(Fixed88 n, Fixed88 j) {
  if (j == 0) {
    return (n > 0) ? FIXED_MAX : (n < 0) ? FIXED_MIN : 0;
  }
  int32_t r = ((int32_t)n << 8) / j;
  return (Fixed88)r;
}

/**
 *@brief Fast division of two fixed-point numbers, can use 16-bit divider
 *@param n Numerator
 *@param j Denominator
 *@return Quotient of n and j with less accuracy due to 16-bit division
 */
extern inline Fixed88 fastDiv(Fixed88 n, Fixed88 j) {
  if (j == 0) {
    return (n > 0) ? FIXED_MAX : (n < 0) ? FIXED_MIN : 0;
  }
  int16_t r = (n / (j >> 4)) << 4;
  return r;
}

/**
 *@brief Performs linear interpolation between two fixed-point numbers
 *@param start The start value
 *@param end The end value
 *@param time The interpolation parameter
 *@return The interpolated result
 */
extern inline Fixed88 lerp(Fixed88 start, Fixed88 end, Fixed88 time) {
  Fixed88 result = fixedAdd(start, fixedMul(fixedSub(end, start), time));
  return result;
}

#endif //FIXED_H
