// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef CERTIFICATE_MANAGER_H
#define CERTIFICATE_MANAGER_H

#include <stdbool.h>
#include "macro_utils.h"
#include "umock_c_prod.h"

/**
 * @brief   retreives the certificate and the private key from a given file.
 * 
 * @param   filePath                the certificate path.
 * @param   connectionString        out param. the returned certificate.
 * @param   connectionStringSize    out param. the private key.
 * 
 * @return true on success, false otherwise.
 */
MOCKABLE_FUNCTION(, bool, CertificateManager_LoadFromFile, const char*, filePath, char**, certificate, char**, privateKey);

#endif //CERTIFICATE_MANAGER_H