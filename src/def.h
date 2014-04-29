#define SHENHE_TCP_BACKLOG       511     /* TCP listen backlog */
#define SHENHE_IOBUF_LEN         (1024*16)

#define MAX_CLIENT      10000
#define MAX_BUFFER_LENGTH      65535
#define MAX_THREAD_NUM      16
#define MAX_CLIENT_NUM      32
#define REDIS_NOTUSED(V) ((void) V)
#define ERROR_TOO_MANY_THREAD "097"
#define ERROR_EMPTY_CONTENT "098"
#define ERROR_WRONG_COMMAND "099"

#define ERROR_INVALID_word "200" //有错误信息
#define ERROR_NONE "100"

#define NOT_FOUND -1

#if __WORDSIZE == 64
typedef long int addr_t;
#else
typedef int	addr_t;
#endif

typedef struct client{
	char read_buffer[MAX_BUFFER_LENGTH];
	char write_buffer[MAX_BUFFER_LENGTH];
	char strpy1[MAX_BUFFER_LENGTH * 6];//普通
	char strpy2[MAX_BUFFER_LENGTH * 6];//拼音
	int unsend_len;//未发送数据长度
	int send_len;//未发送数据长度
	aeEventLoop *el;
	int read_len;
	int fd;
	int cmd;
	pthread_t thread;
	pthread_attr_t thread_attr;
	ANODE *owner;
} CLIENT;

