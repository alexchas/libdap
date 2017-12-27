/*
 Copyright (c) 2017-2018 (c) Project "DeM Labs Inc" https://github.com/demlabsinc
  All rights reserved.

 This file is part of DAP (Deus Applications Prototypes) the open source project

    DAP (Deus Applicaions Prototypes) is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DAP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with any DAP based project.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef _ENC_H_
#define _ENC_H_
#include <stddef.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define SA_ENC_TYPE_1 0x01
#define RSA_KEY_LENGTH 4096
#define PUB_EXP     3

struct enc_key;

typedef enum enc_data_type{ENC_DATA_TYPE_RAW, ENC_DATA_TYPE_B64, ENC_KEY_TYPE_RSA} enc_data_type_t;

typedef struct rsa_session_key {
    RSA* server_key;
    RSA* client_public_key;
    time_t last_time_use_key;
} rsa_key_t;

extern int enc_init();

/// BASE64
extern size_t enc_base64_decode(const char * in, size_t in_size,void * out);
extern size_t enc_base64_encode(const void * in, size_t in_size,char * out);
///

/// AES
#include "common.h"
struct enc_key;

extern size_t enc_rsa_decode(struct enc_key* key, const void * in, size_t in_size,void * out);
extern size_t enc_rsa_encode(struct enc_key* key, void * in, size_t in_size,void * out);

extern void setRsaPubKeyFromString(char *str_key, size_t strLen, struct enc_key * key);
extern size_t getStringPubKeyFromRsa(RSA *key, char **out);
extern size_t getStringPrivateKeyFromRsa(RSA *key, char **out);


extern void enc_aes_key_new(struct enc_key * key);
extern void enc_aes_key_create(struct enc_key * key, const char *password_string);
extern void enc_aes_key_delete(struct enc_key *key);
extern size_t enc_aes_decode(struct enc_key* key, const void * in, size_t in_size,void * out);
extern size_t enc_aes_encode(struct enc_key* key, const void * in, size_t in_size,void * out);


size_t enc_code(struct enc_key * key, const void * buf, const size_t buf_size, void * buf_out, enc_data_type_t data_type_out);
size_t enc_decode(struct enc_key * key, const void * buf, const size_t buf_size, void * buf_out, enc_data_type_t data_type_in);


#endif