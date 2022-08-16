#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "ssm-remote.hpp"

RemoteServer::RemoteServer()
{
    //std::cout << "Remote Server" << std::endl;
}

RemoteServer::~RemoteServer()
{
    //std::cout << "~Remote Server" << std::endl;
}

bool RemoteServer::init()
{
    isock = -1;

    memset(&this->client, 0, sizeof(this->client));
    this->client.data_socket = -1;

    memset(&this->server, 0, sizeof(this->server));
    this->server.wait_socket = -1;
    this->server.server_addr.sin_family = AF_INET;
    this->server.server_addr.sin_addr.s_addr = htonl(RMOTE_IP);
    this->server.server_addr.sin_port = htons(RMOTE_PORT);

    return this->open();
}

bool RemoteServer::open()
{
    this->server.wait_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int flag = 1;
    int ret = setsockopt(this->server.wait_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
    ret = setsockopt(this->server.wait_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));
    if (ret == -1)
    {
        fprintf(stderr, "Remote Server open failed");
        exit(1);
    }

    if (this->server.wait_socket == -1)
    {
        perror("open socket error");
        return false;
    }

    if (bind(this->server.wait_socket, (struct sockaddr *)&this->server.server_addr,
             sizeof(this->server.server_addr)) == -1)
    {
        perror("server bind failed");
        return false;
    }

    if (listen(this->server.wait_socket, 5) == -1)
    {
        perror("server listen failed");
        return false;
    }

    return true;
}

bool RemoteServer::wait()
{
    memset(&this->client, 0, sizeof(this->client));
    this->client.data_socket = -1;
    for (;;)
    {
        socklen_t client_addr_len = sizeof(this->client.client_addr);
        this->client.data_socket = accept(this->server.wait_socket,
                                          (struct sockaddr *)&this->client.client_addr, &client_addr_len);
        if (this->client.data_socket != -1) // success
            break;
        if (errno == EINTR)
            continue;;
        perror("server open accept error");
        return false;
    }    
    return true;
}

// do anything as you like
bool RemoteServer::setup() {
    sleep(3);
    return true;
}
bool RemoteServer::run() {
    sleep(3);
    return true;
}

bool RemoteServer::stop() {
    sleep(3);
    return true;
}
bool RemoteServer::startrecord() {
    sleep(3);
    return true;
}
bool RemoteServer::stoprecord() {
    sleep(3);
    return true;
}
bool RemoteServer::unsetup() {
    sleep(3);
    return true;
}

bool RemoteServer::start()
{
    while (this->wait()) {
        if (setup()) {
            Message msg;
            msg.type = AC_CONNECT;
            sendMessage(&msg);
            std::cout << "send ACK_CONNECT" << std::endl;
        } else {
            client_close(); 
            break;
        }
        std::cout << "start handle request" << std::endl;
        openMsgStream(); // open connection to client
        this->handleRequest();
        this->client_close(); 
        closeMsgStream(); // close connection to client
    }
    server_close();
    return true;
}


bool RemoteServer::client_close() {
    if (this->client.data_socket != -1) {
        close(this->client.data_socket);
        this->client.data_socket = -1;        
    }
    return true;
}

bool RemoteServer::server_close() {
    if (this->server.wait_socket != -1) {
        close(this->server.wait_socket);
        this->server.wait_socket = -1;
    }
    return true;
}

bool RemoteServer::closeMsgStream() {
    if (this->isock != -1) {
        close(this->isock);
        this->isock = -1;
    }
    return true;
}

uint16_t RemoteServer::readShort(uint8_t *buf) {
    //uint8_t v1 = buf[0];
    //uint8_t v2 = buf[1];
    return (uint16_t)(buf[0] << 8 | buf[1]);
    //return (uint16_t)(v1 << 8 | v2);
}

void RemoteServer::writeShort(uint8_t *buf, short sv) {
    buf[0] = (sv >> 8) & 0xff;
    buf[1] = sv & 0xff;
}



void RemoteServer::deserializeMessage(Message* msg, uint8_t *buf) {
    std::cout << "start deserialize" << std::endl;
    int idx = 0;
    msg->type = readShort(&buf[idx]);
    idx += 2;
    msg->arg = readShort(&buf[idx]);    
}

int RemoteServer::receiveMessage(Message *msg, uint8_t *buf, int buf_len) {
    std::cout << "begin receive" << std::endl;
    int len = recv(this->client.data_socket, buf, buf_len, 0);
    std::cout << "received" << std::endl;
    if (len > 0) {        
        deserializeMessage(msg, buf);
    }
    return len;
}

int RemoteServer::sendMessage(Message* msg) {
    int len;
    int buf_len = sizeof(Message);
    uint8_t* buf = (uint8_t*)malloc(buf_len);
    int idx = 0;
    writeShort(&buf[idx], msg->type);
    idx += 2;
    writeShort(&buf[idx], msg->arg);
    
    if ((len = send(this->client.data_socket, buf, buf_len, 0)) == -1) {
        fprintf(stderr, "error happends\n");
    }
    if (buf) {
        free(buf);
    }
    return len;
}

bool RemoteServer::openMsgStream() {
    std::cout << "open msg stream" << std::endl;
    isock = socket(AF_INET, SOCK_STREAM, 0);

    int flag = 1;
    int ret = setsockopt(isock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
    if (ret == -1) {
        perror("client stream setsockopt\n");
        exit(1);
    }

    iserver.sin_family = AF_INET;
    iserver.sin_port = htons(RMOTE_IPORT);
    iserver.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(isock, (struct sockaddr *) &iserver, sizeof(iserver))) {
        fprintf(stderr, "openMsgStream error\n");
        return false;
    }
        
    return true;    
}

void RemoteServer::sendError(uint8_t* buf, size_t len) {
    uint8_t* msgbuf = (uint8_t*)malloc(len + 2);
    msgbuf[0] = (len >> 8) & 0xff;
    msgbuf[1] = len & 0xff;
    memcpy(&msgbuf[2], buf, len);
    if (send(isock, msgbuf, len+2, 0) == -1) {
        fprintf(stderr, "error in sendError");        
    }

    free(msgbuf);
}


void RemoteServer::handleRequest() {
    Message msg;
    int buf_len = sizeof(Message);
    uint8_t *buf = (uint8_t*)malloc(buf_len);
    
    while (true) {
        int len = receiveMessage(&msg, buf, buf_len);
        if (len == 0 )
            break;

        switch (msg.type) {
            case RM_START: {
                printf("RM_START\n");                
                run();
                msg.type = AC_START;
                sendMessage(&msg);
                break;
            }
            case RM_STOP: {
                printf("RM_STOP\n");
                stop();
                sendError((uint8_t*)"Hello", 5); // for debug
                msg.type = AC_STOP;
                sendMessage(&msg);
                break;
            }
            case RM_DISCONNECT: {
                printf("RM_DISCONNECT\n");
                unsetup();
                msg.type = AC_DISCONNECT;
                sendMessage(&msg);
                goto END_PROC;      
                break;
            }
            case RM_STARTRECORD: {
                printf("RM_STARTRECORD\n");
                startrecord();
                msg.type = AC_STARTRECORD;
                sendMessage(&msg);
                break;
            }
            case RM_STOPRECORD: {
                printf("RM_STOPRECORD\n");
                stoprecord();
                msg.type = AC_STOPRECORD;
                sendMessage(&msg);
                break;
            }
            
            default: {
                fprintf(stderr, "unknown message type\n");
                break;
            }
            break;
        }
    }

END_PROC:    
    if (buf) {
        free(buf);
    }
}

void RemoteServer::disconnect() {
    closeMsgStream();
    client_close();
    server_close();
}


static RemoteServer *rserver = nullptr;

static void catchSignal(int signo) {
    if (rserver) {
        std::cout << "disconnect" << std::endl;
        rserver->disconnect();
    }
    exit(1);
}

static void setSigHandler(RemoteServer *_rserver) {
    rserver = _rserver;
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = catchSignal;
    sigaction(SIGINT, &act, NULL);
}

int main(void)
{
    std::cout << "Remote Server Starts" << std::endl;
    RemoteServer rserver;
    rserver.init();
    setSigHandler(&rserver);
    //rserver.openMsgStream(); for test
    rserver.start();    

    return 0;
}
