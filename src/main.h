typedef struct thread
{
	pthread_t thread;
	pthread_cond_t  thread_ready;
	pthread_mutex_t thread_lock;
	CLIENT *c;
	ANODE *owner;
} THREAD;

typedef struct shenhe_server
{
	int thread_num;//线程总数
	ARRAY *thread;
	pthread_mutex_t thread_lock;
	
    ARRAY *client;
    ARRAY *clientWait;//等待处理的客户端队列
	pthread_mutex_t client_lock;
	pthread_mutex_t client_wait_lock;
	NODE *dict_pinyin;
	NODE *dict_replace;
	short reload;//重新加载配置
	long accept_num;
} SERVER;

void readFromClient(aeEventLoop *el,int fd);
void writeResultToClient(aeEventLoop *el, int fd, void *privdata, int mask);
void *clientThread(void *arg);

