// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "os_utils/linux/audit/audit_control.h"
#include "utils.h"

#include <auparse.h>
#include <libaudit.h>
#include <stdlib.h>
#include <unistd.h>

const char AUDIT_CONTROL_ON_SUCCESS_FILTER[] = "success=1";
const char AUDIT_CONTROL_TYPE_EXECVE[] = "execve";
const char AUDIT_CONTROL_TYPE_EXECVEAT[] = "execveat";
const char AUDIT_CONTROL_TYPE_CONNECT[] = "connect";
const char AUDIT_CONTROL_TYPE_ACCEPT[] = "accept";

/*
 * @brief Check whether the host is 64 bit os or not.
 *  
 * @return true if the host is 64 machine.
 */
static bool is64BitMachine();

AuditControlResultValues AuditControl_Init(AuditControl* auditControl) {
    AuditControlResultValues result = AUDIT_CONTROL_OK;

    memset(auditControl, 0, sizeof(*auditControl));
    auditControl->processInfoWasSet = false;

    if (!ProcessInfoHandler_ChangeToRoot(&auditControl->processInfo)) {
        result = AUDIT_CONTROL_EXCEPTION;
        goto cleanup;
    }
    auditControl->processInfoWasSet = true;

    auditControl->audit = audit_open();
    if (auditControl->audit < 0) {
        result = AUDIT_CONTROL_EXCEPTION;
        goto cleanup;
    }

cleanup:
    if (result != AUDIT_CONTROL_OK) {
        AuditControl_Deinit(auditControl);
    }
    return result;
}

void AuditControl_Deinit(AuditControl* auditControl) {
    if (auditControl->audit > 0) {
        audit_close(auditControl->audit);
    }

    if (auditControl->processInfoWasSet) {
        ProcessInfoHandler_Reset(&auditControl->processInfo);
        auditControl->processInfoWasSet = false;
    }
}

AuditControlResultValues AuditControl_AddRule(AuditControl* auditControl, const char** msgTypeArray, size_t msgTypeArraySize, Architecture architecture, const char* extraFilter) {
    AuditControlResultValues result = AUDIT_CONTROL_OK;
    struct audit_rule_data* rule = NULL;
    char* msgTypeCopy = NULL;
    char* extraFilterCopy = NULL;

    rule = malloc(sizeof(struct audit_rule_data));
    memset(rule, 0, sizeof(struct audit_rule_data));

    int flags = AUDIT_FILTER_EXIT & AUDIT_FILTER_MASK;

    if (architecture == ARCHITECTURE_32) {
        const char SUPPORT_32_BIT_FILTER[] = "arch=b32";
        if (audit_rule_fieldpair_data(&rule, SUPPORT_32_BIT_FILTER, flags) != 0) { 
            result = AUDIT_CONTROL_EXCEPTION;
            goto cleanup;
        }
    } else if ((architecture == ARCHITECTURE_64) && is64BitMachine()) {
        const char SUPPORT_64_BIT_FILTER[] = "arch=b64";
        if (audit_rule_fieldpair_data(&rule, SUPPORT_64_BIT_FILTER, flags) != 0) { 
            result = AUDIT_CONTROL_EXCEPTION;
            goto cleanup;
        }
    }

    for (int i=0; i<msgTypeArraySize; i++) {
        if (Utils_CreateStringCopy(&msgTypeCopy, msgTypeArray[i]) == false) {
            result = AUDIT_CONTROL_EXCEPTION;
            goto cleanup;
        }
        
        if ((audit_rule_syscallbyname_data(rule, msgTypeCopy)) < 0) {
            result = AUDIT_CONTROL_EXCEPTION;
            goto cleanup;
        }
    }
    if (extraFilter != NULL) {
        if (Utils_CreateStringCopy(&extraFilterCopy, extraFilter) == false) {
            result = AUDIT_CONTROL_EXCEPTION;
            goto cleanup; 
        }
        if (audit_rule_fieldpair_data(&rule, extraFilterCopy, flags) != 0) { 
            result = AUDIT_CONTROL_EXCEPTION;
            goto cleanup;    
        }
    }

    if (audit_add_rule_data(auditControl->audit, rule, AUDIT_FILTER_EXIT, AUDIT_ALWAYS) <= 0) { 
        result = AUDIT_CONTROL_EXCEPTION;
        goto cleanup;    
    }

cleanup:
    if (extraFilterCopy != NULL) {
        free(extraFilterCopy);
    }

    if (msgTypeCopy != NULL) {
        free(msgTypeCopy);
    }

    if (rule != NULL) {
        free(rule);
    }

    return result;
}

static bool is64BitMachine() {
    long wordBits = sysconf(_SC_LONG_BIT);
    return wordBits == 64;
}
