// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "collectors/firewall_collector.h"

#include <stdlib.h>

#include "json/json_array_writer.h"
#include "json/json_object_writer.h"
#include "logger.h"
#include "message_schema_consts.h"
#include "os_utils/linux/iptables/iptables_iterator.h"
#include "os_utils/linux/iptables/iptables_rules_iterator.h"
#include "os_utils/process_info_handler.h"
#include "utils.h"

#define BUFFER_MAX_SIZE 300

static const char FIREWALL_ALLOW_RULE[] = "Allow";
static const char FIREWALL_DENY_RULE[] = "Deny";
static const char FIREWALL_OTHER_RULE[] = "Other";

static const char FIREWALL_INPUT_CHAIN[] = "INPUT";
static const char FIREWALL_OUTPUT_CHAIN[] = "OUTPUT";

static const char FIREWALL_DIRECTION_IN[] = "In";
static const char FIREWALL_DIRECTION_OUT[] = "Out";

/**
 * @brief Iterates the chains and writes the rules of each chain.
 * 
 * @param   rulesPayloadArray   The json array writer of the payload array.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult FirewallCollector_IterateChains(JsonArrayWriterHandle rulesPayloadArray);

/**
 * @brief Iterates the rules of the chain and write the,.
 * 
 * @param   rulesPayloadArray   The json array writer of the payload array.
 * @param   chainIterator       The chain iterator.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult FirewallCollector_IterateRules(JsonArrayWriterHandle rulesPayloadArray, IptablesIteratorHandle chainIterator);

/**
 * @brief Writes the rule to the payload array.
 * 
 * @param   rulesPayloadArray   The json array writer of the payload array.
 * @param   rulesIterator       The rules iterator.
 * @param   priority            The rules priority.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult FirewallCollector_WriteRules(JsonArrayWriterHandle rulesPayloadArray, IptablesRulesIteratorHandle rulesIterator, uint32_t priority);

/**
 * @brief Checks the iptables result. In case the result is IPTABLES_OK writes the value with the given key,
 *        In case the result is IPTABLES_NO_DATA the function will not write anything and will return EVENT_COLLECTOR_OK.
 *        On any other case the function will fail.
 * 
 * @param   iptablesResult      The return value of the iptables rule iterator function.
 * @param   ruleWriter          The json object writer of the curent rule.
 * @param   keyName             The key name of the given value.
 * @param   value               The value to write.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult FirewallCollector_WriteRuleStringElement(IptablesResults iptablesResult, JsonObjectWriterHandle ruleWriter, const char* keyName, const char* value);

/**
 * @brief Writes the action to the rule writer.
 *        The action will be serialized according to the schema - in case of allow\deny the value will be written, other wise
 *        the "Other" value will be written and the extra data may be written in the "extra details" section
 * 
 * @param   iptablesResult   The return value of the iptables function.
 * @param   ruleWriter       The object writer to write with.
 * @param   actionType       The action of the rule.
 * @param   value            The extra value.
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult FirewallCollector_WriteAction(IptablesResults iptablesResult, JsonObjectWriterHandle ruleWriter, IptablesActionType actionType, const char* value);

/**
 * @brief Writes the chain policy rule to the payload array.
 * 
 * @param   rulesPayloadArray   The json array writer of the payload array.
 * @param   chainIterator       The chain iterator.
 * @param   priority            The rules priority.
 * 
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult FirewallCollector_WritePolicyRule(JsonArrayWriterHandle rulesPayloadArray, IptablesIteratorHandle chainIterator, uint32_t rulePriority);

/**
 * @brief Writes the chain name and the direction elements to the rule writer.
 *        Chain name is always written under the ChainName element.
 *        in case the cahin name is INPUT direction is In, in case of the chain name is OUTPUT direction is Out
 *        otherwise don't write Direction property
 * 
 * @param   ruleWriter       The object writer to write with.
 * @param   chainName        The name of the chain to be writtes
 * 
 * @return EVENT_COLLECTOR_OK on success, EVENT_COLLECTOR_EXCEPTION otherwise.
 */
EventCollectorResult FirewallCollector_WriteRuleDirectionAndChainElements(JsonObjectWriterHandle ruleWriter, const char* chainName);

EventCollectorResult FirewallCollector_GetEvents(SyncQueue* queue) {

    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonObjectWriterHandle firewallRulesWriter = NULL;
    JsonArrayWriterHandle rulesPayloadArray = NULL;
    char* messageBuffer = NULL;

    if (JsonObjectWriter_Init(&firewallRulesWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (GenericEvent_AddMetadata(firewallRulesWriter, EVENT_PERIODIC_CATEGORY, FIREWALL_RULES_NAME, EVENT_TYPE_SECURITY_VALUE, FIREWALL_RULES_PAYLOAD_SCHEMA_VERSION) != EVENT_COLLECTOR_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonArrayWriter_Init(&rulesPayloadArray) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    
    result = FirewallCollector_IterateChains(rulesPayloadArray);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    result = GenericEvent_AddPayload(firewallRulesWriter, rulesPayloadArray); 
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    uint32_t messageBufferSize = 0;
    if (JsonObjectWriter_Serialize(firewallRulesWriter, &messageBuffer, &messageBufferSize) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (SyncQueue_PushBack(queue, messageBuffer, messageBufferSize) != QUEUE_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:

    if (result != EVENT_COLLECTOR_OK) {
        if (messageBuffer !=  NULL) {
            free(messageBuffer);
        }
    }

    if (rulesPayloadArray != NULL) {
        JsonArrayWriter_Deinit(rulesPayloadArray);
    }
    
    if (firewallRulesWriter != NULL) {
        JsonObjectWriter_Deinit(firewallRulesWriter);
    }

    return result;
}

EventCollectorResult FirewallCollector_IterateChains(JsonArrayWriterHandle rulesPayloadArray) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    IptablesIteratorHandle chainIterator = NULL;
    ProcessInfo info;
    bool processInfoWasSet = false;

    if (!ProcessInfoHandler_ChangeToRoot(&info)) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    processInfoWasSet = true;

    IptablesResults initIptablesResult = IptablesIterator_Init(&chainIterator);
    if (initIptablesResult == IPTABLES_NO_DATA) {
        Logger_Information("Iptables does not exist on this device.");
        result = EVENT_COLLECTOR_OK;
        goto cleanup;
    } else if (initIptablesResult != IPTABLES_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    IptablesResults iteratorResult = IptablesIterator_GetNext(chainIterator);
    while (iteratorResult == IPTABLES_ITERATOR_HAS_NEXT) {
        result = FirewallCollector_IterateRules(rulesPayloadArray, chainIterator);
        if (result != EVENT_COLLECTOR_OK) {
            goto cleanup;
        }
        iteratorResult = IptablesIterator_GetNext(chainIterator);
    }

    if (iteratorResult != IPTABLES_ITERATOR_NO_MORE_ITEMS) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (chainIterator != NULL) {
        IptablesIterator_Deinit(chainIterator);
    }

    if (processInfoWasSet) {
        ProcessInfoHandler_Reset(&info);
    }

    return result;
}

EventCollectorResult FirewallCollector_IterateRules(JsonArrayWriterHandle rulesPayloadArray, IptablesIteratorHandle chainIterator) {

    EventCollectorResult result = EVENT_COLLECTOR_OK;
    IptablesRulesIteratorHandle rulesIterator = NULL;

    if (IptablesIterator_GetRulesIterator(chainIterator, &rulesIterator) != IPTABLES_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    uint32_t rulePriority = 0;
    IptablesResults iteratorResult = IptablesRulesIterator_GetNext(rulesIterator);
    while (iteratorResult == IPTABLES_ITERATOR_HAS_NEXT) {
        result = FirewallCollector_WriteRules(rulesPayloadArray, rulesIterator, rulePriority);
        if (result != EVENT_COLLECTOR_OK) {
            goto cleanup;
        }
        rulePriority++;
        iteratorResult = IptablesRulesIterator_GetNext(rulesIterator);
    }

    if (iteratorResult != IPTABLES_ITERATOR_NO_MORE_ITEMS) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = FirewallCollector_WritePolicyRule(rulesPayloadArray, chainIterator, rulePriority);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

cleanup:
    if (rulesIterator != NULL) {
        IptablesRulesIterator_Deinit(rulesIterator);
    }

    return result;
}

EventCollectorResult FirewallCollector_WriteRules(JsonArrayWriterHandle rulesPayloadArray, IptablesRulesIteratorHandle rulesIterator, uint32_t priority) {
    char buffer[BUFFER_MAX_SIZE] = "";
    EventCollectorResult result = EVENT_COLLECTOR_OK;
    JsonObjectWriterHandle ruleWriter = NULL;

    if (JsonObjectWriter_Init(&ruleWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteBool(ruleWriter, FIREWALL_RULES_ENABLED_KEY, true) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    
    if(JsonObjectWriter_WriteInt(ruleWriter, FIREWALL_RULES_PRIORITY_KEY, priority) != JSON_WRITER_OK){
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup; 
    }

    const char* chainName = NULL;
    IptablesResults ruleResult = IptablesRulesIterator_GetChainName(rulesIterator, &chainName);
    if (ruleResult != IPTABLES_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    result = FirewallCollector_WriteRuleDirectionAndChainElements(ruleWriter, chainName);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    memset(buffer, 0, sizeof(buffer));
    ruleResult = IptablesRulesIterator_GetSrcIp(rulesIterator, buffer, sizeof(buffer));
    result = FirewallCollector_WriteRuleStringElement(ruleResult, ruleWriter, FIREWALL_RULES_SRC_ADDRESS_KEY, buffer);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    memset(buffer, 0, sizeof(buffer));
    ruleResult = IptablesRulesIterator_GetSrcPort(rulesIterator, buffer, sizeof(buffer));
    result = FirewallCollector_WriteRuleStringElement(ruleResult, ruleWriter, FIREWALL_RULES_SRC_PORT_KEY, buffer);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    memset(buffer, 0, sizeof(buffer));
    ruleResult = IptablesRulesIterator_GetDestIp(rulesIterator, buffer, sizeof(buffer));
    result = FirewallCollector_WriteRuleStringElement(ruleResult, ruleWriter, FIREWALL_RULES_DEST_ADDRESS_KEY, buffer);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    memset(buffer, 0, sizeof(buffer));
    ruleResult = IptablesRulesIterator_GetDestPort(rulesIterator, buffer, sizeof(buffer));
    result = FirewallCollector_WriteRuleStringElement(ruleResult, ruleWriter, FIREWALL_RULES_DEST_PORT_KEY, buffer);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    memset(buffer, 0, sizeof(buffer));
    ruleResult = IptablesRulesIterator_GetProtocol(rulesIterator, buffer, sizeof(buffer));
    result = FirewallCollector_WriteRuleStringElement(ruleResult, ruleWriter, FIREWALL_RULES_PROTOCOL_KEY, buffer);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }

    memset(buffer, 0, sizeof(buffer));
    IptablesActionType actionType = 0;
    ruleResult = IptablesRulesIterator_GetAction(rulesIterator, &actionType, buffer, sizeof(buffer));
    result = FirewallCollector_WriteAction(ruleResult, ruleWriter, actionType, buffer);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }
    
    if (JsonArrayWriter_AddObject(rulesPayloadArray, ruleWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (ruleWriter != NULL) {
        JsonObjectWriter_Deinit(ruleWriter);
    }

    return result;
}

EventCollectorResult FirewallCollector_WriteAction(IptablesResults iptablesResult, JsonObjectWriterHandle ruleWriter, IptablesActionType actionType, const char* value) {
    if (iptablesResult == IPTABLES_OK) {
        if (actionType == IPTABLES_ACTION_ALLOW) {
            if (JsonObjectWriter_WriteString(ruleWriter, FIREWALL_RULES_ACTION_KEY, FIREWALL_ALLOW_RULE) != JSON_WRITER_OK) {
                return EVENT_COLLECTOR_EXCEPTION;
            }
        } else if (actionType == IPTABLES_ACTION_DENY) {
            if (JsonObjectWriter_WriteString(ruleWriter, FIREWALL_RULES_ACTION_KEY, FIREWALL_DENY_RULE) != JSON_WRITER_OK) {
                return EVENT_COLLECTOR_EXCEPTION;
            }
        } else {
            if (JsonObjectWriter_WriteString(ruleWriter, FIREWALL_RULES_ACTION_KEY, FIREWALL_OTHER_RULE) != JSON_WRITER_OK) {
                return EVENT_COLLECTOR_EXCEPTION;
            }
            //FIXME: we need to add data to extraDetailes in the future (int case that this is not allow\deny).
        }
    } else if (iptablesResult != IPTABLES_NO_DATA) {
        return EVENT_COLLECTOR_EXCEPTION;
    }
    return EVENT_COLLECTOR_OK;
}

EventCollectorResult FirewallCollector_WriteRuleStringElement(IptablesResults iptablesResult, JsonObjectWriterHandle ruleWriter, const char* keyName, const char* value) {
    if (iptablesResult == IPTABLES_OK) {
        if (JsonObjectWriter_WriteString(ruleWriter, keyName, value) != JSON_WRITER_OK) {
            return EVENT_COLLECTOR_EXCEPTION;
        }
    } else if (iptablesResult != IPTABLES_NO_DATA) {
        return EVENT_COLLECTOR_EXCEPTION;
    }
    return EVENT_COLLECTOR_OK;
}

EventCollectorResult FirewallCollector_WriteRuleDirectionAndChainElements(JsonObjectWriterHandle ruleWriter, const char* chainName) {
    if (chainName == NULL) {
        return EVENT_COLLECTOR_EXCEPTION;
    }    

    if (JsonObjectWriter_WriteString(ruleWriter, FIREWALL_RULES_CHAIN_NAME_KEY, chainName) != JSON_WRITER_OK) {
        return EVENT_COLLECTOR_EXCEPTION;
    }

    const char* direction = NULL;
    if (Utils_UnsafeAreStringsEqual(chainName, FIREWALL_INPUT_CHAIN, true) == true) {
        direction = FIREWALL_DIRECTION_IN;
    } else if (Utils_UnsafeAreStringsEqual(chainName, FIREWALL_OUTPUT_CHAIN, true) == true) {
        direction = FIREWALL_DIRECTION_OUT;
    }

    if (chainName != NULL) {
        if (JsonObjectWriter_WriteString(ruleWriter, FIREWALL_RULES_DIRECTION_KEY, direction) != JSON_WRITER_OK) { 
            return EVENT_COLLECTOR_EXCEPTION;
        }
    } 

    return EVENT_COLLECTOR_OK;
}    

EventCollectorResult FirewallCollector_WritePolicyRule(JsonArrayWriterHandle rulesPayloadArray, IptablesIteratorHandle chainIterator, uint32_t rulePriority) {
    EventCollectorResult result = EVENT_COLLECTOR_OK;

    char buffer[BUFFER_MAX_SIZE] = "";
    JsonObjectWriterHandle ruleWriter = NULL;
    if (JsonObjectWriter_Init(&ruleWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

    if (JsonObjectWriter_WriteBool(ruleWriter, FIREWALL_RULES_ENABLED_KEY, true) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    
    if(JsonObjectWriter_WriteInt(ruleWriter, FIREWALL_RULES_PRIORITY_KEY, rulePriority) != JSON_WRITER_OK){
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup; 
    }

    const char* chainName = NULL;
    IptablesResults ruleResult = IptablesIterator_GetChainName(chainIterator, &chainName);
    if (ruleResult != IPTABLES_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }
    
    result = FirewallCollector_WriteRuleDirectionAndChainElements(ruleWriter, chainName);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }   

    memset(buffer, 0, sizeof(buffer));
    IptablesActionType actionType;
    ruleResult = IptablesIterator_GetPolicyAction(chainIterator, &actionType);
    result = FirewallCollector_WriteAction(ruleResult, ruleWriter, actionType, buffer);
    if (result != EVENT_COLLECTOR_OK) {
        goto cleanup;
    }
    
    if (JsonArrayWriter_AddObject(rulesPayloadArray, ruleWriter) != JSON_WRITER_OK) {
        result = EVENT_COLLECTOR_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (ruleWriter != NULL) {
        JsonObjectWriter_Deinit(ruleWriter);
    }

    return result;
}