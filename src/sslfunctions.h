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
#define FAIL -1

#endif

#ifdef ssl_on
SSL_CTX* cCtx = NULL;
SSL_CTX* sCtx = NULL;
bool sslInitialized = false;

void initOpenSSL()
{
    if(!sslInitialized){
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        cCtx = SSL_CTX_new(SSLv23_client_method());
        sCtx = SSL_CTX_new(SSLv23_server_method());
    }
}
bool LoadCertificates(char* CertFile, char* KeyFile)
{
    if(SSL_CTX_use_certificate_file(sCtx,CertFile,SSL_FILETYPE_PEM) <= 0){
        return false;   
    }
    if(SSL_CTX_use_PrivateKey_file(sCtx,KeyFile,SSL_FILETYPE_PEM) <= 0){
        return false;
    }
    if(!SSL_CTX_check_private_key(sCtx)){
        return false;
    }
    return true;
}

void shutdownSSL()
{
    if(cCtx){
        SSL_CTX_free(cCtx);
    }
    if(sCtx){
        SSL_CTX_free(sCtx);
    }
}
/** Fetches the server certificates
 
 @param ssl ssl struct
 */

void ShowCerts(SSL* ssl)
{   X509 *cert;
    char *line;
    
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("Info: No client certificates configured.\n");
}
void wrapServer(COMM* handle)
{
    if(sCtx)
    {
        handle->ssl = SSL_new(sCtx);
        SSL_set_fd(handle->ssl,handle->fd);
        if(SSL_accept(handle->ssl) == FAIL){
        
        }else{
            ShowCerts(handle->ssl);
        }
    }
}
void freeConnection(COMM* handle){
    if(handle->ssl){
        SSL_free(handle->ssl);
        handle->ssl = NULL;
    }
}

/**
 Wraps a file descriptor into an ssl socket

 @param handle Communication handle for the socket
 */
void wrapClient(COMM* handle){
    if(cCtx){
        handle->ssl = SSL_new(cCtx);
        if(!handle->ssl){
            printf("Could not get ssl from context.\n");
            return;
        }
        SSL_set_fd(handle->ssl,handle->fd);
        if(SSL_connect(handle->ssl) == FAIL){
            
        }else{
            ShowCerts(handle->ssl);
        }
    }
}

//bool loadCertificates(const char* chainFile, const char* keyFile, SSL_CTX* ctx){
//    if(SSL_CTX_use_certificate_file(ctx, chainFile, SSL_FILETYPE_PEM) <= 0){
//        printf("An error occurred!\n");
//        return false;
//    }
//    if(keyFile){
//        printf("Using server configuration!\n");
//        if(SSL_CTX_use_PrivateKey_file(ctx, keyFile, SSL_FILETYPE_PEM) <= 0){
//            return false;
//        }
//        if(!SSL_CTX_check_private_key(ctx)){
//            return false;
//        }
//    }
//
//    return true;
//}

#endif
