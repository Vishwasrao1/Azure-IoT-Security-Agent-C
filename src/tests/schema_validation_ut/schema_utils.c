// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "schema_utils.h"
#include "message_serializer.h"

#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 4096

char* to_full_path(char* path)
{
	char* dest = malloc(BUFFER_SIZE);
	memset(dest, 0, BUFFER_SIZE);

	char basePath[BUFFER_SIZE] = {0};
	if (readlink("/proc/self/exe", basePath, sizeof(basePath)) != -1)
	{
		dirname(basePath);
		snprintf(dest, BUFFER_SIZE, "%s/%s", basePath, path);
	}

	return dest;
}

SchemaValidationResult validate_schema(SyncQueue* eventQueue)
{
	SchemaValidationResult result = SCHEMA_VALIDATION_OK;

 	char* messageJsonString = NULL;

    SyncQueue* queues[] = {eventQueue};
    if(MESSAGE_SERIALIZER_OK != MessageSerializer_CreateSecurityMessage(queues, 1, (void**)&messageJsonString))
	{
		result = SCHEMA_VALIDATION_ERROR;
		goto cleanup;
	}

	char* tempPath = to_full_path("temp.json");
	FILE* jsonFile = fopen(tempPath, "w");
	if(jsonFile == NULL)
	{
		result = SCHEMA_VALIDATION_ERROR;
		goto cleanup;
	}

	if(fprintf(jsonFile, "%s", messageJsonString) < 0)
	{
		result = SCHEMA_VALIDATION_ERROR;
		goto cleanup;
	}

	if(fclose(jsonFile) == EOF)
	{
		result = SCHEMA_VALIDATION_ERROR;
		goto cleanup;
	}

	char* referenceWildcard = to_full_path("message*.json");

	char* messageRootPath = to_full_path("messageRoot.json");
	char commandBuffer[BUFFER_SIZE];
	snprintf(commandBuffer, sizeof(commandBuffer), "ajv test -s \"%s\" -r \"%s\" -d \"%s\" --valid", messageRootPath, referenceWildcard, tempPath);

	int status = system(commandBuffer);
	int retcode = WEXITSTATUS(status);
	if(retcode != 0)
	{
		result = SCHEMA_VALIDATION_ERROR;
		fprintf(stderr, "json failed schema validation:\n%s\n", messageJsonString);
		goto cleanup;
	}

cleanup:

	free(messageJsonString);
	free(tempPath);
	free(messageRootPath);
	free(referenceWildcard);

	return result;
}