// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "internal/time_utils.h"
#include "internal/time_utils_consts.h"
#include "message_schema_consts.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* DATETIME_FORMAT = "%FT%TZ";

time_t TimeUtils_GetCurrentTime() {
    return time(NULL);
}

int32_t TimeUtils_GetTimeDiff(time_t end, time_t beginning) {
    return (int32_t)(difftime(end, beginning) * 1000);
}

bool TimeUtils_GetTimeAsString(time_t* currentTime, char* output, uint32_t* outputSize) {
    struct tm currentLocalTime;
    if (localtime_r(currentTime, &currentLocalTime) == NULL) {
        return false;
    }
    *outputSize = strftime(output, *outputSize, DATETIME_FORMAT, &currentLocalTime);
    return true;
}

bool TimeUtils_GetLocalTimeAsUTCTimeAsString(time_t* currentLocalTime, char* output, uint32_t* outputSize) {
    struct tm currentUTCTime; 
    if (gmtime_r(currentLocalTime, &currentUTCTime) == NULL) {
        return false;
    }
    *outputSize = strftime(output, *outputSize, DATETIME_FORMAT, &currentUTCTime);
    return true;
}

/**
 * @brief   Parse the date section of an ISO 8601 duration string to miliseconds
 * 
 * @param   dateString          Date string to parse.
 * @param   resultOut           Out param. allocated buffer for the converted time span.
 *  
 * @retrn true on success. false otherwise.
 */
bool TimeUtils_ParseDateString(const char* dateString, uint32_t* resultOut) {
    if (dateString == NULL) {
        return false;
    }

    if (resultOut == NULL) {
        return false;
    }

    uint32_t result = 0;
    uint32_t temp = 0;
    char specifier = '\0';

    if (sscanf(dateString, "%u%c", &temp, &specifier) != 2) {
        return false;
    }

    if (specifier == 'Y') {
        dateString = strchr(dateString, 'Y') + 1;
        if (temp > MAX_YEARS_IN_UINT32) {
            // Overflow guard
            return false;
        }
        if (dateString[0] == 'T' || dateString[0] == '\0') {
            return true;
        }
        if (sscanf(dateString, "%u%c", &temp, &specifier) != 2) {
            return false;
        }
    }

    if (specifier == 'M') {
        dateString = strchr(dateString, 'M') + 1;
        if (temp > MAX_MONTHS_IN_UINT32) {
            // Overflow guard
            return false;
        }
        result = temp * MILLISECONDS_IN_A_MONTH;
        if (dateString[0] == 'T' || dateString[0] == '\0') {
            *resultOut = result;
            return true;
        }
        if (sscanf(dateString, "%u%c", &temp, &specifier) != 2) {
            return false;
        }
    }

    if (specifier == 'W') {
        dateString = strchr(dateString, 'W') + 1;
        if (temp > MAX_WEEKS_IN_UINT32) {
            // Overflow guard
            return false;
        }
        if (!Utils_AddUnsignedIntsWithOverflowCheck(&result, result, temp * MILLISECONDS_IN_A_WEEK)) {
            return false;
        }
        if (dateString[0] == 'T' || dateString[0] == '\0') {
            *resultOut = result;
            return true;
        }
        if (sscanf(dateString, "%u%c", &temp, &specifier) != 2) {
            return false;
        }
    }

    if (specifier == 'D') {
        dateString = strchr(dateString, 'D') + 1;
        if (temp > MAX_DAYS_IN_UINT32) {
            // Overflow guard
            return false;
        }
        if (!Utils_AddUnsignedIntsWithOverflowCheck(&result, result, temp * MILLISECONDS_IN_A_DAY)) {
            return false;
        }
        if (dateString[0] == 'T' || dateString[0] == '\0') {
            *resultOut = result;
            return true;
        }
    }

    return false;
}

/**
 * @brief   Parse the time section of an ISO 8601 duration string to miliseconds
 * 
 * @param   timeString          Time string to parse.
 * @param   resultOut           Out param. allocated buffer for the converted time span.
 *  
 * @retrn true on success. false otherwise.
 */
bool TimeUtils_ParseTimeString(const char* timeString, uint32_t* resultOut) {
    if (timeString == NULL) {
        return false;
    }

    if (resultOut == NULL) {
        return false;
    }

    uint32_t result = 0;
    uint32_t temp = 0;
    char specifier = '\0';

    if (sscanf(timeString, "%u%c", &temp, &specifier) != 2) {
        return false;
    }

    if (specifier == 'H') {
        timeString = strchr(timeString, 'H') + 1;
        if (temp > MAX_HOURS_IN_UINT32) {
            // Overflow guard
            return false;
        }
        if (!Utils_AddUnsignedIntsWithOverflowCheck(&result, result, temp * MILLISECONDS_IN_AN_HOUR)) {
            return false;
        }
        if (timeString[0] == '\0') {
            *resultOut = result;
            return true;
        }
        if (sscanf(timeString, "%u%c", &temp, &specifier) != 2) {
            return false;
        }
    }

    if (specifier == 'M') {
        timeString = strchr(timeString, 'M') + 1;
        if (temp > MAX_MINUTES_IN_UINT32) {
            // Overflow guard
            return false;
        }
        if (!Utils_AddUnsignedIntsWithOverflowCheck(&result, result, temp * MILLISECONDS_IN_A_MINUTE)) {
            return false;
        }
        if (timeString[0] == '\0') {
            *resultOut = result;
            return true;
        }
        if (sscanf(timeString, "%u%c", &temp, &specifier) != 2) {
            return false;
        }
    }

    if (specifier == 'S') {
        timeString = strchr(timeString, 'S') + 1;
        if (temp > MAX_SECONDS_IN_UINT32) {
            // Overflow guard
            return false;
        }
        if (!Utils_AddUnsignedIntsWithOverflowCheck(&result, result, temp * MILLISECONDS_IN_A_SECOND)) {
            return false;
        }
        if (timeString[0] == '\0') {
            *resultOut = result;
            return true;
        }
    }

    return false;
}

bool TimeUtils_ISO8601DurationStringToMilliseconds(const char* duration, uint32_t* milisecOut) {
    const char* dateString = NULL;
    const char* timeString = NULL;
    uint32_t result = 0, dateResult = 0, timeResult = 0;

    if (duration == NULL) {
        return false;
    }

    if (duration[0] != 'P') {
        return false;
    }

    dateString = duration + 1; 
    timeString = strchr(duration, 'T');

    if (timeString != NULL) {
        ++timeString;
    }

    if (timeString != dateString + 1) {
        if (!TimeUtils_ParseDateString(dateString, &dateResult)) {
            return false;
        }
    }

    if (timeString != NULL) {
        if (!TimeUtils_ParseTimeString(timeString, &timeResult)) {
            return false;
        }
    }

    if (!Utils_AddUnsignedIntsWithOverflowCheck(&result, dateResult, timeResult)) {
        return false;
    }

    if (result == 0) {
        return false;
    }

    *milisecOut = result;
    return true;
}

bool TimeUtils_MillisecondsToISO8601DurationString(uint32_t milisec, char* duration, uint32_t len) {
    if (milisec < MILLISECONDS_IN_A_SECOND) {
        return false;
    }

    if (len < DURATION_MAX_LENGTH) {
        return false;
    }


    if (!Utils_ConcatenateToString(&duration, &len, "P")) {
        return false;
    }

    uint32_t months = milisec / MILLISECONDS_IN_A_MONTH;
    milisec %= MILLISECONDS_IN_A_MONTH;
    if (months > 0) {
        if (!Utils_ConcatenateToString(&duration, &len, "%uM", months)) {
            return false;
        }
    }

    uint32_t weeks = milisec / MILLISECONDS_IN_A_WEEK;
    milisec %= MILLISECONDS_IN_A_WEEK;
    if (weeks > 0) {
        if (!Utils_ConcatenateToString(&duration, &len, "%uW", weeks)) {
            return false;
        }
    }

    uint32_t days = milisec / MILLISECONDS_IN_A_DAY;
    milisec %= MILLISECONDS_IN_A_DAY;
    if (days > 0) {
        if (!Utils_ConcatenateToString(&duration, &len, "%uD", days)) {
            return false;
        }
    }

    uint32_t hours = milisec / MILLISECONDS_IN_AN_HOUR;
    milisec %= MILLISECONDS_IN_AN_HOUR;
    uint32_t minutes = milisec / MILLISECONDS_IN_A_MINUTE;
    milisec %= MILLISECONDS_IN_A_MINUTE;
    uint32_t seconds = milisec / MILLISECONDS_IN_A_SECOND;

    if (hours > 0 || minutes > 0 || seconds > 0) {
        duration += sprintf(duration, "T");

        if (hours > 0) {
            if (!Utils_ConcatenateToString(&duration, &len, "%uH", hours)) {
                return false;
            }
        }

        if (minutes > 0) {
            if (!Utils_ConcatenateToString(&duration, &len, "%uM", minutes)) {
                return false;
            }
        }

        if (seconds > 0) {
            if (!Utils_ConcatenateToString(&duration, &len, "%uS", seconds)) {
                return false;
            }
        }
    }

    return true;
}