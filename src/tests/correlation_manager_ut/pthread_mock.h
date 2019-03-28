// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "macro_utils.h"
#include "umock_c_prod.h"

#include <pthread.h>

typedef void (*keyDestroyer) (void *);

MOCKABLE_FUNCTION(, int, pthread_key_create, pthread_key_t*, __key, keyDestroyer, destroyerFunc);
MOCKABLE_FUNCTION(, void*, pthread_getspecific, pthread_key_t, __key);
MOCKABLE_FUNCTION(, int, pthread_setspecific, pthread_key_t, __key, const void*, __pointer);
MOCKABLE_FUNCTION(, int, pthread_key_delete, pthread_key_t, __key);