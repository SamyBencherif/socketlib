// Connects the client to a specified host and port. The host must be a propper ip address. (Hostnames must
// be resolved before passing it to this function)
int client_connect(struct client_sock* client, const char* host, int port){
    if(!sock_state_valid(&client->handle)){
        return INVALIDSTATE;
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(client->prot, host, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    bool connected = connect(client->handle.fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == 0;
    if(connected){
        if(client->ssl){
            initOpenSSL();
            wrapClient(&client->handle);
        }
    }else{
        printf("Failed to connect.\n");
        return -1;
    }
    return 1;
}