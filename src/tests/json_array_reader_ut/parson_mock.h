// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "json/json_reader.h"

MOCKABLE_FUNCTION(, JSON_Array*, json_object_dotget_array, const JSON_Object*, object, const char*, name);
MOCKABLE_FUNCTION(, size_t, json_array_get_count, const JSON_Array*, array);
MOCKABLE_FUNCTION(, JSON_Object*, json_array_get_object, const JSON_Array*, array, size_t, index);
MOCKABLE_FUNCTION(, int, json_object_dothas_value_of_type, const JSON_Object*, object, const char*, name, JSON_Value_Type, type);

