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
	pthread_mutex_t client_lock;
	NODE *dict_pinyin;
	NODE *dict_replace;
} SERVER;

void readFromClient(aeEventLoop *el,int fd);
void writeResultToClient(aeEventLoop *el, int fd, void *privdata, int mask);
void *clientThread(void *arg);

