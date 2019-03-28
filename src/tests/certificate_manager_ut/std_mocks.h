// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <stdlib.h>

MOCKABLE_FUNCTION(, FILE*, fopen, const char*, path, const char*, mode);
MOCKABLE_FUNCTION(, int, fclose, FILE*, fd)