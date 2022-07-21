#ifndef _SSM_REMOTE_H_
#define _SSM_REMOTE_H_

#include "ssm-proxy.hpp"

#define RMOTE_PORT 10800
#define RMOTE_IP 0x00000000UL

enum {
    RM_NULL = 0,
    RM_START = 1,    
    RM_STOP = 2,
    RM_DISCONNECT = 3,
    AC_CONNECT = 10,
    AC_START = 11,
    AC_STOP = 12,
    AC_DISCONNECT = 13,    
};

struct Message {
    uint16_t type;
    uint16_t arg;
};

class RemoteServer
{
private:
    TCPSERVER_INFO server;
    TCPCLIENT_INFO client;

    bool open();
    bool wait();
    uint16_t readShort(uint8_t *buf);
    void writeShort(uint8_t *buf, short v);


    void deserializeMessage(Message* msg, uint8_t *buf);    
    int receiveMessage(Message *msg, uint8_t *buf, int msg_len);
    int sendMessage(Message *msg);

    bool setup();
    void handleRequest();
    bool client_close();
    bool server_close();

public:
    RemoteServer();
    ~RemoteServer();
    bool init();
    bool start();    
    void disconnect();
    
};

#endif