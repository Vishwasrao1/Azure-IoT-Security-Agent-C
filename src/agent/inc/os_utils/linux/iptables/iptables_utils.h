// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IPTABLES_UTILS_H
#define IPTABLES_UTILS_H

#include <stdbool.h>
#include <stdint.h>

#include "macro_utils.h"
#include "umock_c_prod.h"

#include "os_utils/linux/iptables/iptables_def.h"

/**
 * @brief Formats the given ip to standart ip/subnet notation (with option to invert the value) and writes it to the given buffer.
 * 
 * @param   ip           The ip.
 * @param   mask         The ip mask.
 * @param   invert       A flge which indicates whether the ip should be inverted (adding !).
 * @param   buffer       The buffer to write the ip to.
 * @param   bufferSize   The size of the outout buffer.
 * 
 * @return IPTABLES_OK in case of success, IPTABLES_EXCEPTOIN otherwise.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesUtils_FormatIp, uint32_t, ip, uint32_t, mask, bool, invert, char*, buffer, uint32_t, bufferSize);

/**
 * @brief Formats the protocol and writes it to the given buffer.
 * 
 * @param   protocol     The protocol code.
 * @param   invert       A flge which indicates whether the ip should be inverted (adding !).
 * @param   buffer       The buffer to write the ip to.
 * @param   bufferSize   The size of the outout buffer.
 * 
 * @return IPTABLES_OK in case of success, IPTABLES_EXCEPTOIN otherwise.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesUtils_FormatProtocol, uint16_t, protocol, bool, invert, char*, buffer, uint32_t, bufferSize);

/**
 * @brief Formats the given ranged port (with option to invert the value) and writes it to the given buffer.
 *        If lowPart is 0 and high part is 65535 the function will return IPTABLES_NO_DATA (since this is similare to the
 *        case that the range is not given).
 * 
 * @param   lowPort      The low part of the range.
 * @param   highPort     The high part of the range.
 * @param   invert       A flge which indicates whether the ip should be inverted (adding !).
 * @param   buffer       The buffer to write the ip to.
 * @param   bufferSize   The size of the outout buffer.
 * 
 * @return IPTABLES_OK on success, IPTABLES_NO_DATA in case the wanted data isn't defines for this rule or IPTABLES_EXCEPTION in case of error.
 */
MOCKABLE_FUNCTION(, IptablesResults, IptablesUtils_FormatRangedPorts, uint16_t, lowPort, uint16_t, highPort, bool, invert, char*, buffer, uint32_t, bufferSize);

/**
 * @brief Get an action type from an iptables action
 * 
 * @param   action      iptables action
 * @param   actionType  out param the action type for the given action
 * 
 * @return IPTABLES_OK on success, IPTABLES_EXCEPTION in case of error
 */
MOCKABLE_FUNCTION(, IptablesResults ,IptablesUtils_GetActionTypeEnumFromActionString, const char*, action, IptablesActionType*, actionType);

#endif //IPTABLES_UTILS_H