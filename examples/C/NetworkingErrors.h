//
//  JKNetworkingErrors.h
//  JKNetworking
//
//  Created by JK2Designs on 12/13/18.
//  Copyright © 2018 JK2Touch. All rights reserved.
//

#ifndef JKNetworkingErrors_h
#define JKNetworkingErrors_h
/*!Represents errors encountered by the underlying socket implementation.*/
enum Errors{
    WRITEERR,/*!< Failure to write to a connection. The connection may be closed or it's receive buffer may be full.*/
    RECVERR,/*!< Failure to receive data from a connection. The connection may be closed or nothing was sent.*/
    CONNINT,/*!< The remote connection was closed unexpectedly. The remote connection may have encountered an unrecoverable error.*/
    CONNFAIL,/*!< Failure to connect to the host. The remote host might be down, or the system could not reserve a file descriptor for the connection.*/
    INVALIDSTATE/*!< The socket was in an invalid state to perform the specified action. Usually this implies the socket is closed.*/
};
#endif /* JKNetworkingErrors_h */
