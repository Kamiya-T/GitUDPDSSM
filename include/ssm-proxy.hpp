#ifndef _SSM_PROXY_H_
#define _SSM_PROXY_H_

#include "ssm.hpp"
#include <netinet/in.h>
#include "Thread.hpp"

void test();

#define SERVER_PORT 8080	   /* サーバ用PORT */
#define SERVER_IP 0x00000000UL /* サーバ用待ち受けIP */
#define BUFFER_SIZE 1024	   /* バッファバイト数 */

/* クライアントからの接続を待つサーバの情報を表現する構造体 */
struct TCPSERVER_INFO
{
	int wait_socket;		 /* サーバ待ち受け用ソケット */
	sockaddr_in server_addr; /* サーバ待ち受け用アドレス */
};

/* クライアントとの接続に関する情報を保存する構造体 */
struct TCPCLIENT_INFO
{
	int data_socket;		 /* クライアントとの通信用ソケット */
	sockaddr_in client_addr; /* クライアントのアドレス */
};

struct BUFFERDATA
{
	SSM_tid tid;
	time_t	time;
};

class ProxyServer;

class DataCommunicator : public Thread
{
private:
	TCPSERVER_INFO server;
	TCPCLIENT_INFO client;

	char *mData;
	uint64_t mDataSize;
	uint64_t ssmHeaderSize;
	uint64_t mFullDataSize;
	PROXY_open_mode mType;
	uint32_t thrdMsgLen;

	char *buf;

	bool isTCP;

	SSMApiBase *pstream;
	ProxyServer *proxy;

	bool sopen();
	bool rwait();
	bool UDPrwait();
	bool UDPsopen();
	bool sclose();

	bool receiveTMsg(thrd_msg *tmsg);
	bool sendTMsg(thrd_msg *tmsg);

	bool deserializeTmsg(thrd_msg *tmsg);
	bool serializeTmsg(thrd_msg *tmsg);

	void handleData();
	void handleRead();
	void handleBuffer();

	bool sendBulkData(char *buf, uint64_t size);
	bool receiveData();

public:
	DataCommunicator() = delete;
	DataCommunicator(uint16_t nport, char *mData, uint64_t d_size, uint64_t h_size,
					 SSMApiBase *pstream, PROXY_open_mode type, ProxyServer *proxy, bool isTCP = true);
	~DataCommunicator();

	void *run(void *args);
	void handleBufferRead();
};

class ProxyServer
{
private:
	TCPSERVER_INFO server;
	TCPCLIENT_INFO client;
	uint16_t nport; // センサデータ受信用のポート番号.子プロセスが生成されるたびにインクリメントしていく

	char *mData;			// データ用
	uint64_t mDataSize;		// データサイズ
	uint64_t ssmHeaderSize;	// データにつけるヘッダのサイズ
	uint64_t mFullDataSize; // mDataSize + ヘッダのサイズ
	char *mProperty;
	uint64_t mPropertySize;
	PROXY_open_mode mType;

	DataCommunicator *com;

	SSMApiBase stream; // real stream
	uint32_t dssmMsgLen;

	bool open();
	bool wait();

	void setSSMType(PROXY_open_mode mode);

	int receiveMsg(ssm_msg *msg, char *buf);
	int sendMsg(int cmd_type, ssm_msg *msg);

	void setupSigHandler();
	static void catchSignal(int signo);

public:
	ProxyServer();
	~ProxyServer();
	bool init();
	bool run();
	bool server_close();
	bool client_close();
	void handleCommand();

	int readInt(char **p);
	uint64_t readLong(char **p);
	double readDouble(char **p);
	void readRawData(char **p, char *d, int len);

	void writeInt(char **p, int v);
	void writeLong(char **p, uint64_t v);
	void writeDouble(char **p, double v);
	void writeRawData(char **p, char *d, int len);

	void deserializeMessage(ssm_msg *msg, char *buf);
};

#endif
