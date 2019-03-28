// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <stdio.h>

MOCKABLE_FUNCTION(, FILE* ,popen, const char*, command, const char*, type);
MOCKABLE_FUNCTION(, int, pclose, FILE*, stream);
MOCKABLE_FUNCTION(, void, clearerr, FILE*, stream);
MOCKABLE_FUNCTION(, int, feof, FILE*, stream);
MOCKABLE_FUNCTION(, int, ferror, FILE*, stream);
MOCKABLE_FUNCTION(, size_t, fread, void*, ptr, size_t, size, size_t, nmemb, FILE*, stream);
