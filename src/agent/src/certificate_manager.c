// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "certificate_manager.h"

#include <stdlib.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/pkcs12.h>

#include "agent_errors.h"
#include "os_utils/file_utils.h"
#include "utils.h"

#define MAX_CERT 2000
#define MAX_KEY 5000

static const char* SUPPORTED_FORMAT_PEM = ".pem";
static const char* SUPPORTED_FORMAT_PKCS12 = ".pfx";

/**
 * @brief   retreives the certificate and the private key from a pem file.
 * 
 * @param   filePath        the certificate path.
 * @param   certificate     out param. the returned certificate.
 * @param   privateKey      out param. the private key.
 * 
 * @return true on success, false otherwise.
 */
static bool CertificateManager_LoadFromFileUsingPemFormat(const char* filePath, char** certificate, char** privateKey);

/**
 * @brief   retreives the certificate and the private key from a pfx file.
 * 
 * @param   filePath        the certificate path.
 * @param   certificate     out param. the returned certificate.
 * @param   privateKey      out param. the private key.
 * 
 * @return true on success, false otherwise.
 */
static bool CertificateManager_LoadFromFileUsingPkcs12Format(const char* filePath, char** certificate, char** privateKey);

/**
 * @brief   retreives the certificate from X509 structure.
 * 
 * @param   certificate  the X509 certificate.
 * 
 * @return  the certificate, in pem format (base 64).
 */
static char* CertificateManager_X509CertificateToPemFormat(X509* certificate);

/**
 * @brief   retreives the private key from EVP_PKEY structure. assumption: there is no passphrase.
 * 
 * @param   privateKey  the private key in EVP_PKEY format.
 * 
 * @return  the private key, in pem format (base 64).
 */
static char* CertificateManager_PrivateKeyToPemFormat(EVP_PKEY* privateKey);

/**
 * @brief   Opens the given file path as a binary in read mode
 * 
 * @param   filePath  path to the file to open
 * 
 * @return  file handle
 */
static FILE* CertificateManager_OpenFileForRead(const char* file);

static char* CertificateManager_X509CertificateToPemFormat(X509* certificate) {
    BIO* bio = NULL;
    char* pem = NULL;
    size_t bufferLength = 0;

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL) {
        goto cleanup;
    }

    if (PEM_write_bio_X509(bio, certificate) == 0) {
        goto cleanup;
    }

    bufferLength = BIO_get_mem_data(bio, NULL);

    pem = (char*) malloc(bufferLength + 1);
    if (pem == NULL) {
        goto cleanup;
    }

    memset(pem, 0, bufferLength + 1);
    BIO_read(bio, pem, bufferLength);

cleanup:
    if (bio != NULL) {
        BIO_free(bio);
    }

    return pem;
}

static char* CertificateManager_PrivateKeyToPemFormat(EVP_PKEY* privateKey) {
    BIO* bio = NULL;
    char* pem = NULL;
    size_t bufferLength = 0;

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL) {
        goto cleanup;
    }

    if (PEM_write_bio_PrivateKey(bio, privateKey, 0, NULL, 0, NULL, NULL) == 0) {
        goto cleanup;
    }

    bufferLength = BIO_get_mem_data(bio, NULL);

    pem = (char*) malloc(bufferLength + 1);
    if (pem == NULL) {
        goto cleanup;
    }

    memset(pem, 0, bufferLength + 1);
    BIO_read(bio, pem, bufferLength);

cleanup:
    if (bio != NULL) {
        BIO_free(bio);
    }

    return pem;
}

static bool CertificateManager_LoadFromFileUsingPemFormat(const char* filePath, char** certificate, char** privateKey) {
    bool result = true;
    X509* x509Certificate = NULL;
    EVP_PKEY* evpPrivateKey = NULL;
    FILE* certificateFile = CertificateManager_OpenFileForRead(filePath);
    if (certificateFile == NULL) {
        result = false;
        goto cleanup;
    }

    x509Certificate = PEM_read_X509(certificateFile, NULL, NULL, NULL);
    if (x509Certificate == NULL) {
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_FORMAT, "Couldn't parse certificate");
        result = false;
        goto cleanup;
    }
    *certificate = CertificateManager_X509CertificateToPemFormat(x509Certificate);

    evpPrivateKey = PEM_read_PrivateKey(certificateFile, NULL, NULL, NULL);
    if (evpPrivateKey == NULL) {
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_FORMAT, "Couldn't read private key from certificate");
        result = false;
        goto cleanup;
    }
    *privateKey = CertificateManager_PrivateKeyToPemFormat(evpPrivateKey);

cleanup:
    if (certificateFile != NULL) {
        fclose(certificateFile);
    }
    if (x509Certificate != NULL) {
        X509_free(x509Certificate);
    }
    if (evpPrivateKey != NULL) {
        EVP_PKEY_free(evpPrivateKey);
    }

    if (!result) {
        if (*certificate != NULL) {
            free(*certificate);
            *certificate = NULL;
        }
        if (*privateKey != NULL) {
            free(*privateKey);
            *privateKey = NULL;
        }
    }

    return result;
}

static bool CertificateManager_LoadFromFileUsingPkcs12Format(const char* filePath, char** certificate, char** privateKey) {
    bool result = true;
    PKCS12* pkcs12Certificate = NULL;
    EVP_PKEY* evpPrivateKey = NULL;
    X509* x509Certificate = NULL;

    FILE* certificateFile = CertificateManager_OpenFileForRead(filePath);
    if (certificateFile == NULL) {
        result = false;
        goto cleanup;
    }

    d2i_PKCS12_fp(certificateFile, &pkcs12Certificate);
    if (pkcs12Certificate == NULL) {
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_FORMAT, "Couldn't parse certificate");
        result = false;
        goto cleanup;
    }

    if (PKCS12_parse(pkcs12Certificate, NULL, &evpPrivateKey, &x509Certificate, NULL) != 1) {
        AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_FORMAT, "Couldn't parse certificate");
        result = false;
        goto cleanup;
    }

    *certificate = CertificateManager_X509CertificateToPemFormat(x509Certificate);
    *privateKey = CertificateManager_PrivateKeyToPemFormat(evpPrivateKey);

cleanup:
    if (certificateFile != NULL) {
        fclose(certificateFile);
    }

    if (x509Certificate != NULL) {
        X509_free(x509Certificate);
    }
    if (evpPrivateKey != NULL) {
        EVP_PKEY_free(evpPrivateKey);
    }
    if (pkcs12Certificate != NULL) {
        PKCS12_free(pkcs12Certificate);
    }

    if (!result) {
        if (*certificate != NULL) {
            free(*certificate);
            *certificate = NULL;
        }
        if (*privateKey != NULL) {
            free(*privateKey);
            *privateKey = NULL;
        }
    }

    return result;
}

bool CertificateManager_LoadFromFile(const char* filePath, char** certificate, char** privateKey) {

    char* extension = strrchr(filePath, '.');
    if (extension == NULL) {
        return false;
    }

    if (Utils_UnsafeAreStringsEqual(extension, SUPPORTED_FORMAT_PEM, false)) {
        return CertificateManager_LoadFromFileUsingPemFormat(filePath, certificate, privateKey);
    }

    if (Utils_UnsafeAreStringsEqual(extension, SUPPORTED_FORMAT_PKCS12, false)) {
        return CertificateManager_LoadFromFileUsingPkcs12Format(filePath, certificate, privateKey);
    }

    AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_FORMAT, "Certificate of type %s, is not supported", extension);
    return false;
}

static FILE* CertificateManager_OpenFileForRead(const char* filePath) {
    FILE* fd = NULL;
    FileResults fileResult = FileUtils_OpenFile(filePath, "rb", &fd);
    if (fileResult != FILE_UTILS_OK) {
        if (fileResult == FILE_UTILS_FILE_NOT_FOUND) {
            AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_NOT_EXIST, "File not found in path: %s", filePath);
        } else if (fileResult == FILE_UTILS_NOPERM) {
            AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_FILE_PERMISSIONS, "Couldn't open file in path: %s, check permissions", filePath);
        } else {
            AgentErrors_LogError(ERROR_IOT_HUB_AUTHENTICATION, SUBCODE_OTHER, "Unexpected error while opening file: %s", filePath);
        }
    }
    
    return fd;
}