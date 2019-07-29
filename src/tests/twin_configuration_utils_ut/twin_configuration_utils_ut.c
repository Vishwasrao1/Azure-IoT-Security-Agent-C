// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "testrunnerswitcher.h"
#include "macro_utils.h"

#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_bool.h"

#include "twin_configuration_utils.h"
#include "json/json_object_reader.h"
#include "json/json_object_writer.h"

static TEST_MUTEX_HANDLE test_serialize_mutex;
static TEST_MUTEX_HANDLE g_dllByDll;
 MU_DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s",  MU_ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(twin_configuration_utils_ut)

TEST_SUITE_INITIALIZE(suite_init)
{
     

    test_serialize_mutex = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(test_serialize_mutex);

    umock_c_init(on_umock_c_error);

    umocktypes_charptr_register_types();
    umocktypes_bool_register_types();
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

TEST_FUNCTION(TwinConfigurationUtils_ReadUintConfig_ExpectSuccess)
{
    JsonObjectReaderHandle reader;
    uint32_t val = 0;
    const char* json = "{ \"key\" : { \"value\" : 1234 }}";
    JsonObjectReader_InitFromString(&reader, json);

    TwinConfigurationResult res = TwinConfigurationUtils_GetConfigurationUintValueFromJson(reader, "key", &val);
    
    ASSERT_ARE_EQUAL(int, TWIN_OK, res);
    ASSERT_ARE_EQUAL(int, 1234, val);
    JsonObjectReader_Deinit(reader);
}

TEST_FUNCTION(TwinConfigurationUtils_ReadUintConfig_NoValue_ExpectFail)
{
    JsonObjectReaderHandle reader;
    uint32_t val = 0;
    const char* json = "{ \"key\" : { \"NoValue\" : 1234 }}";
    JsonObjectReader_InitFromString(&reader, json);

    TwinConfigurationResult res = TwinConfigurationUtils_GetConfigurationUintValueFromJson(reader, "key", &val);
    
    ASSERT_ARE_EQUAL(int, TWIN_PARSE_EXCEPTION, res);
    JsonObjectReader_Deinit(reader);
}

TEST_FUNCTION(TwinConfigurationUtils_ReadUintConfig_ValueNotNested_ExpectFail)
{
    JsonObjectReaderHandle reader;
    uint32_t val = 0;
    const char* json = "{ \"key\" : 1234}";
    JsonObjectReader_InitFromString(&reader, json);

    TwinConfigurationResult res = TwinConfigurationUtils_GetConfigurationUintValueFromJson(reader, "key", &val);
    
    ASSERT_ARE_EQUAL(int, TWIN_PARSE_EXCEPTION, res);
    JsonObjectReader_Deinit(reader);
}

TEST_FUNCTION(TwinConfigurationUtils_ReadUintConfig_keyMissing_ExpectFail)
{
    JsonObjectReaderHandle reader;
    uint32_t val = 0;
    const char* json = "{ \"NoTheRightKey\" : {\"value\" :1234}}";
    JsonObjectReader_InitFromString(&reader, json);

    TwinConfigurationResult res = TwinConfigurationUtils_GetConfigurationUintValueFromJson(reader, "key", &val);
    
    ASSERT_ARE_EQUAL(int, TWIN_CONF_NOT_EXIST, res);
    JsonObjectReader_Deinit(reader);
}

TEST_FUNCTION(TwinConfigurationUtils_ReadTimeConfig_ExpectSuccess)
{
    JsonObjectReaderHandle reader;
    uint32_t val = 0;
    const char* json = "{ \"key\" : { \"value\" : \"PT1S\" }}";
    JsonObjectReader_InitFromString(&reader, json);

    TwinConfigurationResult res = TwinConfigurationUtils_GetConfigurationTimeValueFromJson(reader, "key", &val);
    
    ASSERT_ARE_EQUAL(int, TWIN_OK, res);
    ASSERT_ARE_EQUAL(int, 1000, val);
    JsonObjectReader_Deinit(reader);
}

TEST_FUNCTION(TwinConfigurationUtils_ReadStringConfig_ExpectSuccess)
{
    JsonObjectReaderHandle reader;
    char* str = NULL;
    const char* json = "{ \"key\" : { \"value\" : \"I'm a string\" }}";
    JsonObjectReader_InitFromString(&reader, json);

    TwinConfigurationResult res = TwinConfigurationUtils_GetConfigurationStringValueFromJson(reader, "key", &str);
    
    ASSERT_ARE_EQUAL(int, TWIN_OK, res);
    ASSERT_ARE_EQUAL(int, 0, strcmp("I'm a string", str));
    JsonObjectReader_Deinit(reader);
}

TEST_FUNCTION(TwinConfigurationUtils_ReadBoolConfig_ExpectSuccess)
{
    JsonObjectReaderHandle reader;
    bool val = false;
    const char* json = "{ \"key\" : { \"value\" : true }}";
    JsonObjectReader_InitFromString(&reader, json);

    TwinConfigurationResult res = TwinConfigurationUtils_GetConfigurationBoolValueFromJson(reader, "key", &val);
    
    ASSERT_ARE_EQUAL(int, TWIN_OK, res);
    ASSERT_ARE_EQUAL(int, 1, val);
    JsonObjectReader_Deinit(reader);
}

TEST_FUNCTION(TwinConfigurationUtils_WriteUint_ExpectSuccess)
{
    JsonObjectWriterHandle writer;
    JsonObjectWriter_Init(&writer);

    TwinConfigurationResult res = TwinConfigurationUtils_WriteUintConfigurationToJson(writer, "key", 1234);
    ASSERT_ARE_EQUAL(int, TWIN_OK, res);
    
    char* serTwin;
    uint32_t size;
    JsonObjectWriter_Serialize(writer, &serTwin, &size);
    JsonObjectWriter_Deinit(writer);


    JsonObjectReaderHandle reader;
    JsonObjectReader_InitFromString(&reader, serTwin);
    uint32_t val;
    res = TwinConfigurationUtils_GetConfigurationUintValueFromJson(reader, "key", &val);
    ASSERT_ARE_EQUAL(int, TWIN_OK, res);
    ASSERT_ARE_EQUAL(int, 1234, val);
    free(serTwin);
    JsonObjectReader_Deinit(reader);
}

TEST_FUNCTION(TwinConfigurationUtils_WriteString_ExpectSuccess)
{
    JsonObjectWriterHandle writer;
    JsonObjectWriter_Init(&writer);

    TwinConfigurationResult res = TwinConfigurationUtils_WriteStringConfigurationToJson(writer, "key", "String");
    ASSERT_ARE_EQUAL(int, TWIN_OK, res);
    
    char* serTwin;
    uint32_t size;
    JsonObjectWriter_Serialize(writer, &serTwin, &size);
    JsonObjectWriter_Deinit(writer);


    JsonObjectReaderHandle reader;
    JsonObjectReader_InitFromString(&reader, serTwin);
    char* str;
    res = TwinConfigurationUtils_GetConfigurationStringValueFromJson(reader, "key", &str);
    ASSERT_ARE_EQUAL(int, TWIN_OK, res);
    ASSERT_ARE_EQUAL(int, 0, strcmp("String", str));
    free(serTwin);
    JsonObjectReader_Deinit(reader);
}

TEST_FUNCTION(TwinConfigurationUtils_WriteBool_ExpectSuccess)
{
    JsonObjectWriterHandle writer;
    JsonObjectWriter_Init(&writer);

    TwinConfigurationResult res = TwinConfigurationUtils_WriteBoolConfigurationToJson(writer, "key", true);
    ASSERT_ARE_EQUAL(int, TWIN_OK, res);
    
    char* serTwin;
    uint32_t size;
    JsonObjectWriter_Serialize(writer, &serTwin, &size);
    JsonObjectWriter_Deinit(writer);


    JsonObjectReaderHandle reader;
    JsonObjectReader_InitFromString(&reader, serTwin);
    bool val;
    res = TwinConfigurationUtils_GetConfigurationBoolValueFromJson(reader, "key", &val);
    ASSERT_ARE_EQUAL(int, TWIN_OK, res);
    ASSERT_IS_TRUE(val);
    free(serTwin);
    JsonObjectReader_Deinit(reader);
}

END_TEST_SUITE(twin_configuration_utils_ut)
