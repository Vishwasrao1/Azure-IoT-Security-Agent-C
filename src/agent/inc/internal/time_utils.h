// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

/**
 * @brief Returns the current time.
 * 
 * @return The current time as time_t struct.
 */
MOCKABLE_FUNCTION(, time_t, TimeUtils_GetCurrentTime);

/**
 * @brief Returns the diff between time beginning to time end, in milliseconds.
 * 
 * @param   end         Higher bound of the time interval whose length is calculated.
 * @param   beginning   Lower bound of the time interval whose length is calculated.
 * 
 * @return The diff between time beginning to time end, in milliseconds.
 */
MOCKABLE_FUNCTION(, int32_t, TimeUtils_GetTimeDiff, time_t, end, time_t, beginning);

/**
 * @brief Converts the given time to string.
 * 
 * @param   currentTime     The current time to convert.
 * @param   output          Out param. pre-allocated buffer that will contain the formated time.
 * @param   outputSize      In out param. When function is called it should be the size of the output buffer.
 *                          On return this should be the size of the formated time.
 * 
 * @retrn true on success. false otherwise.
 */
MOCKABLE_FUNCTION(, bool, TimeUtils_GetTimeAsString, time_t*, currentTime, char*, output, uint32_t*, outputSize);

/**
 * @brief Converts the current local time (with timezone) to string.
 * 
 * @param   output          Out param. pre-allocated buffer that will contain the formated time.
 * @param   outputSize      In out param. When function is called it should be the size of the output buffer.
 *                          On return this should be the size of the formated time.
 * 
 * @retrn true on success. false otherwise.
 */
MOCKABLE_FUNCTION(, bool, TimeUtils_GetLocalTimeAsString, char*, output, uint32_t*, outputSize);

/**
 * @brief Converts the current UTC time (timezone GTM) to string.
 * 
 * @param   output          Out param. pre-allocated buffer that will contain the formated time.
 * @param   outputSize      In out param. When function is called it should be the size of the output buffer.
 *                          On return this should be the size of the formated time.
 *  
 * @retrn true on success. false otherwise.
 */
MOCKABLE_FUNCTION(, bool, TimeUtils_GetUTCTimeAsString, char*, output, uint32_t*, outputSize);

/**
 * @brief Converts the given local time to a UTC time and returns the string representation of the UTC time.
 * 
 * @param   currentLocalTime     The current time to convert.
 * @param   output               Out param. pre-allocated buffer that will contain the formated time.
 * @param   outputSize           In out param. When function is called it should be the size of the output buffer.
 *                               On return this should be the size of the formated time.
 *  
 * @retrn true on success. false otherwise.
 */
MOCKABLE_FUNCTION(, bool, TimeUtils_GetLocalTimeAsUTCTimeAsString, time_t*, currentLocalTime, char*, output, uint32_t*, outputSize);

/**
 * @brief   converts ISO 8601 duration string to miliseconds
 * 
 * @param   duration             duration string to parse.
 * @param   milisecOut           Out param. allocated buffer for the converted time span.
 *  
 * @retrn true on success. false otherwise.
 */
MOCKABLE_FUNCTION(, bool, TimeUtils_ISO8601DurationStringToMilliseconds, const char*, duration, uint32_t*, milisecOut)

/**
 * @brief   converts milliseconds to ISO 8601 duration string, while greedily maxing out large time units first.
 * 
 * @param   milisec              miliseconds to convert.
 * @param   milisecOut           Out param. allocated buffer for the converted time span.
 * @param   len                  buffer length.
 *  
 * @retrn true on success. false otherwise.
 */
MOCKABLE_FUNCTION(, bool, TimeUtils_MillisecondsToISO8601DurationString, uint32_t, milisec, char*, duration, uint32_t, len);

#endif //TIME_UTILS_H