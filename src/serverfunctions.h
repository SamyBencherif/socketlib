// server functions
int start(struct server_sock* server, int backlog, int port){
    struct sockaddr_in serverAddr;
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = server->prot;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);
    int one = 1;
    setsockopt(server->handle.fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(bind(server->handle.fd,(struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0){
        return false;
    }
    if(listen(server->handle.fd,backlog) < 0){
        return false;
    }
    return true;
}

struct client_sock sock_acc(struct server_sock server)
{
    struct client_sock client;
    memset(&client,'0',sizeof(client));
    struct sockaddr clientaddr;
    memset(&clientaddr,'0',sizeof(clientaddr));
    COMM handle;
    client.handle = handle;
    memset(&client.handle, '0', sizeof(client.handle));
    
    socklen_t addr_len = sizeof(clientaddr);
    int fd = accept(server.handle.fd,&clientaddr,&addr_len);
    if(fd >= 0){
        client.handle.fd = fd;
        client.handle.ssl = NULL;
        // set prototype and socket type
        client.prot = server.prot;
        client.type = server.prot;
        // Set io functions
        client.receive = receive;
        client.sendall = send_buff;
        client.send_msg = send_msg;
        client.recv_msg = recv_msg;
        // disable connecting
        client.connect = NULL;
        client.close = close_socket;
        client.settimeout = settimeout;
        client.err = NULL;
        if(server.ssl == true){
            // set communication handle
            #ifdef ssl_on
            wrapServer(&client.handle);
            client.ssl = true;
            #endif   
        }
    }else{
        client.err = "Could not obtain file descriptor from the system.";
    }
    return client;
    
}