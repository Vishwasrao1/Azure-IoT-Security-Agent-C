// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IPTABLES_MULTIPORT_H
#define IPTABLES_MULTIPORT_H

#include <libiptc/libiptc.h>
#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/linux/iptables/iptables_def.h"

/**
 * @brief Check the given match to see if there are any src\dest ports (according to the given flag).
 *        If such ports were found it writes them to the given buffer.
 * 
 * @param   match         The given match. Should be of type multiport.
 * @param   isSrcPorts    A flag which indicates the kinds of port this function should check (src\dest).
 * @param   buffer        The buffer to serialize the ports to.
 * @param   bufferSize    The size of the given buffer, in bytes.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA if no data was found or IPTABLES_EXCEPTION otherwise.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesMultiport_GetPorts, struct xt_entry_match*, match, bool, isSrcPorts, char*, buffer, uint32_t, bufferSize);

#endif //IPTABLES_MULTIPORT_H