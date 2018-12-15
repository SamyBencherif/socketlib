//
//  sslfunctions.h
//  JKNetworking
//
//  Created by JK2Designs on 12/12/18.
//  Copyright © 2018 JK2Touch. All rights reserved.
//
#define ssl_on 1
#ifdef ssl_on
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#endif
#ifdef ssl_on
SSL_CTX* cCtx = NULL;
SSL_CTX* sCtx = NULL;
bool sslInitialized = false;
void initOpenSSL(){
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
}
void initServerSSL(const char* chainFile, const char* keyFile){
    
}
bool loadCertificates(const char* chainFile, const char* keyFile, SSL_CTX* ctx){
    if(SSL_CTX_use_certificate_file(ctx, chainFile, SSL_FILETYPE_PEM) <= 0){
        printf("An error occurred!\n");
        return false;
    }
    if(keyFile){
        printf("Using server configuration!\n");
        if(SSL_CTX_use_PrivateKey_file(ctx, keyFile, SSL_FILETYPE_PEM) <= 0){
            return false;
        }
        if(!SSL_CTX_check_private_key(ctx)){
            return false;
        }
    }
    
    return true;
}
bool initClientSSL(const char* chainfile){
    if(!cCtx){
        cCtx = SSL_CTX_new(SSLv23_client_method());
        if(!cCtx){
            printf("An error occurred initializing client ssl\n");
            return false;
        }
        loadCertificates(chainfile, NULL, cCtx);
    }else{
        printf("SSL already initialized!\n");
        return false;
    }
    return true;
}
#endif
