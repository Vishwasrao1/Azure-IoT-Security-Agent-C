// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <unistd.h>

MOCKABLE_FUNCTION(, int, open, const char*, path, int, flag);
MOCKABLE_FUNCTION(, int, close, int, fd)
MOCKABLE_FUNCTION(, ssize_t, write, int, fd, const void*, buffer, size_t, size)
MOCKABLE_FUNCTION(, ssize_t, read, int, fd, void*, buffer, size_t, size)
