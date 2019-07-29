// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_bool.h"
#include "umocktypes_charptr.h"

#include "internal/time_utils.h"
#include "internal/time_utils_consts.h"

bool TimeUtils_ParseDateString(const char* dateString, uint32_t* resultOut);
bool TimeUtils_ParseTimeString(const char* timeString, uint32_t* resultOut);

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(time_utils_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();
    REGISTER_UMOCK_ALIAS_TYPE(uint32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(int32_t, int);
    REGISTER_UMOCK_ALIAS_TYPE(time_t, int);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();
    TEST_MUTEX_DESTROY(test_serialize_mutex);
     
}

TEST_FUNCTION_INITIALIZE(method_init)
{
    umock_c_reset_all_calls();
}

TEST_FUNCTION(TimeUtils_GetLocalTimeAsUTCTimeAsString_ShouldReturnIso8601FormattedString)
{
    time_t time = 1552453485;
    uint32_t size = 256;
    char buffer[size];
    memset(buffer, 0, size);
    TimeUtils_GetLocalTimeAsUTCTimeAsString(&time, buffer, &size);
    ASSERT_ARE_EQUAL(char_ptr, "2019-03-13T05:04:45Z", buffer);
}

TEST_FUNCTION(TimeUtils_GetTimeAsString_ShouldReturnIso8601FormattedString)
{
    setenv("TZ", "PST8PDT", 1);
    tzset();

    time_t time = 1552453485;
    uint32_t size = 256;
    char buffer[size];
    memset(buffer, 0, size);
    TimeUtils_GetTimeAsString(&time, buffer, &size);
    ASSERT_ARE_EQUAL(char_ptr, "2019-03-12T22:04:45Z", buffer);
}

TEST_FUNCTION(TimeUtils_ParseDateStringFullExpectSuccess)
{
    uint32_t expected = MILLISECONDS_IN_A_MONTH + 2 * MILLISECONDS_IN_A_WEEK + 3 * MILLISECONDS_IN_A_DAY;
    uint32_t actual = 0;
    const char* duration = "1M2W3D";

    bool result = TimeUtils_ParseDateString(duration, &actual);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(uint32_t, expected, actual);
}

TEST_FUNCTION(TimeUtils_ParseDateStringPartialExpectSuccess)
{
    uint32_t expected = MILLISECONDS_IN_A_MONTH + 2 * MILLISECONDS_IN_A_DAY;
    uint32_t actual = 0;
    const char* duration = "1M2D";

    bool result = TimeUtils_ParseDateString(duration, &actual);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(uint32_t, expected, actual);
}

TEST_FUNCTION(TimeUtils_ParseDateStringInvalidFormatExpectFailure)
{
    uint32_t actual = 0;
    const char* duration = "1M3Y2D";

    bool result = TimeUtils_ParseDateString(duration, &actual);

    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(TimeUtils_ParseDateStringOverflowExpectFailure)
{
    uint32_t actual = 0;
    const char* duration = "1M30D";

    bool result = TimeUtils_ParseDateString(duration, &actual);

    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(TimeUtils_ParseTimeStringFullExpectSuccess)
{
    uint32_t expected = MILLISECONDS_IN_AN_HOUR + 2 * MILLISECONDS_IN_A_MINUTE + 3 * MILLISECONDS_IN_A_SECOND;
    uint32_t actual = 0;
    const char* duration = "1H2M3S";

    bool result = TimeUtils_ParseTimeString(duration, &actual);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(uint32_t, expected, actual);
}

TEST_FUNCTION(TimeUtils_ParseTimeStringPartialExpectSuccess)
{
    uint32_t expected = MILLISECONDS_IN_AN_HOUR + 2 * MILLISECONDS_IN_A_SECOND;
    uint32_t actual = 0;
    const char* duration = "1H2S";

    bool result = TimeUtils_ParseTimeString(duration, &actual);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(uint32_t, expected, actual);
}

TEST_FUNCTION(TimeUtils_ParseTimeStringInvalidFormatExpectFailure)
{
    uint32_t actual = 0;
    const char* duration = "3M2H";

    bool result = TimeUtils_ParseTimeString(duration, &actual);

    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(TimeUtils_ParseTimeStringOverflowExpectFailure)
{
    uint32_t actual = 0;
    const char* duration = "1193H71582M";

    bool result = TimeUtils_ParseTimeString(duration, &actual);

    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(TimeUtils_ISO8601DurationStringToMillisecondsFullExpectSuccess)
{
    uint32_t expected = MILLISECONDS_IN_A_DAY + 2 * MILLISECONDS_IN_AN_HOUR + 3 * MILLISECONDS_IN_A_MINUTE + 4 * MILLISECONDS_IN_A_SECOND;
    uint32_t actual = 0;
    const char* duration = "P0Y0M0W1DT2H3M4S";

    bool result = TimeUtils_ISO8601DurationStringToMilliseconds(duration, &actual);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(uint32_t, expected, actual);
}

TEST_FUNCTION(TimeUtils_ISO8601DurationStringToMillisecondsOnlyDateExpectSuccess)
{
    uint32_t expected = MILLISECONDS_IN_A_WEEK + MILLISECONDS_IN_A_DAY;
    uint32_t actual = 0;
    const char* duration = "P1W1D";

    bool result = TimeUtils_ISO8601DurationStringToMilliseconds(duration, &actual);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(uint32_t, expected, actual);
}

TEST_FUNCTION(TimeUtils_ISO8601DurationStringToMillisecondsOnlyTimeExpectSuccess)
{
    uint32_t expected = MILLISECONDS_IN_AN_HOUR + 2 * MILLISECONDS_IN_A_MINUTE + 3 * MILLISECONDS_IN_A_SECOND;
    uint32_t actual = 0;
    const char* duration = "PT1H2M3S";

    bool result = TimeUtils_ISO8601DurationStringToMilliseconds(duration, &actual);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(uint32_t, expected, actual);
}

TEST_FUNCTION(TimeUtils_ISO8601DurationStringToMillisecondsOnlyMinutesExpectSuccess)
{
    uint32_t expected = 2 * MILLISECONDS_IN_A_MINUTE;
    uint32_t actual = 0;
    const char* duration = "PT2M";

    bool result = TimeUtils_ISO8601DurationStringToMilliseconds(duration, &actual);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(uint32_t, expected, actual);
}

TEST_FUNCTION(TimeUtils_ISO8601DurationStringToMillisecondsOnlyMonthsExpectFailure)
{
    uint32_t actual = 0;
    const char* duration = "P2M";

    bool result = TimeUtils_ISO8601DurationStringToMilliseconds(duration, &actual);

    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(TimeUtils_ISO8601DurationStringToMillisecondsEmptyExpectFailure)
{
    uint32_t actual = 0;
    const char* duration = "PT";

    bool result = TimeUtils_ISO8601DurationStringToMilliseconds(duration, &actual);

    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(TimeUtils_ISO8601DurationStringToMillisecondsInvalidExpectFailure)
{
    uint32_t actual = 0;
    const char* duration = "P5S";

    bool result = TimeUtils_ISO8601DurationStringToMilliseconds(duration, &actual);

    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(TimeUtils_ISO8601DurationStringToMillisecondsEmptyTimeExpectFailure)
{
    uint32_t actual = 0;
    const char* duration = "P5DT";

    bool result = TimeUtils_ISO8601DurationStringToMilliseconds(duration, &actual);

    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(TimeUtils_MillisecondsToISO8601DurationStringExpectSuccess)
{
    uint32_t milisec = MILLISECONDS_IN_A_DAY + 2 * MILLISECONDS_IN_AN_HOUR + 3 * MILLISECONDS_IN_A_MINUTE + 4 * MILLISECONDS_IN_A_SECOND;
    const char* expected = "P1DT2H3M4S";
    char actual[DURATION_MAX_LENGTH];

    bool result = TimeUtils_MillisecondsToISO8601DurationString(milisec, actual, DURATION_MAX_LENGTH);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, expected, actual);
}

TEST_FUNCTION(TimeUtils_MillisecondsToISO8601DurationStringOnlyDateExpectSuccess)
{
    uint32_t milisec = MILLISECONDS_IN_A_WEEK + 2 * MILLISECONDS_IN_A_DAY;
    const char* expected = "P1W2D";
    char actual[DURATION_MAX_LENGTH];

    bool result = TimeUtils_MillisecondsToISO8601DurationString(milisec, actual, DURATION_MAX_LENGTH);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, expected, actual);
}

TEST_FUNCTION(TimeUtils_MillisecondsToISO8601DurationStringOnlyTimeExpectSuccess)
{
    uint32_t milisec = MILLISECONDS_IN_AN_HOUR + 2 * MILLISECONDS_IN_A_MINUTE + 3 * MILLISECONDS_IN_A_SECOND;
    const char* expected = "PT1H2M3S";
    char actual[DURATION_MAX_LENGTH];

    bool result = TimeUtils_MillisecondsToISO8601DurationString(milisec, actual, DURATION_MAX_LENGTH);

    ASSERT_IS_TRUE(result);
    ASSERT_ARE_EQUAL(char_ptr, expected, actual);
}

TEST_FUNCTION(TimeUtils_MillisecondsToISO8601DurationSmallNumberExpectFailure)
{
    uint32_t milisec = 1;
    char actual[DURATION_MAX_LENGTH];

    bool result = TimeUtils_MillisecondsToISO8601DurationString(milisec, actual, DURATION_MAX_LENGTH);

    ASSERT_IS_FALSE(result);
}

TEST_FUNCTION(TimeUtils_MillisecondsToISO8601DurationSmallBufferExpectFailure)
{
    uint32_t milisec = MILLISECONDS_IN_A_DAY + 2 * MILLISECONDS_IN_AN_HOUR + 3 * MILLISECONDS_IN_A_MINUTE + 4 * MILLISECONDS_IN_A_SECOND;
    const char* expected = "P1DT2H3M4S";
    char actual[DURATION_MAX_LENGTH - 1];

    bool result = TimeUtils_MillisecondsToISO8601DurationString(milisec, actual, DURATION_MAX_LENGTH - 1);

    ASSERT_IS_FALSE(result);
}

END_TEST_SUITE(time_utils_ut)
