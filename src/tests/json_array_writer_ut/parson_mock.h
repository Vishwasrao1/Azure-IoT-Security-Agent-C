// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "json/json_writer.h"

MOCKABLE_FUNCTION(, JSON_Value*, json_value_init_array);
MOCKABLE_FUNCTION(, JSON_Array*, json_value_get_array, const JSON_Value*, value);
MOCKABLE_FUNCTION(, void, json_value_free, JSON_Value*, value);
MOCKABLE_FUNCTION(, JSON_Status, json_array_append_value, JSON_Array*, array, JSON_Value*, value);
MOCKABLE_FUNCTION(, char *, json_serialize_to_string, const JSON_Value*, value);
MOCKABLE_FUNCTION(, size_t, json_array_get_count, const JSON_Array*, array);
