#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "ae.h"
#include "anet.h"

#include "stddef.h"
#include <netinet/in.h>
#include <errno.h>
#include "zmalloc.h"
#include <pthread.h>
#include "array.h"
#include "def.h"
#include "ac.h"
#include "main.h"

int strpos(char *haystack, char *needle)
{
   char *p = strstr(haystack, needle);
   if (p)
      return p - haystack;
   return NOT_FOUND;
}
int mypow(int x,int y)
{
	int base = x;
	if(x == 0) return 0;
	if(y == 0) return 1;
	
	y--;
	while(y--) {
		x =x * base;
	}
	return x;
}

pthread_t thread;
SERVER *server;
NODE *acRoot;

/**
 * 应该要带配置文件
 */
SERVER *createServer()
{
    int i = 0;
    CLIENT *c;
	THREAD *t;
	ANODE *node;
	
	SERVER *s = (SERVER *)zmalloc(sizeof(SERVER));
	bzero(s,sizeof(SERVER));
    s->client = arrayCreate();
    s->thread = arrayCreate();
	
    pthread_mutex_init(&s->client_lock,NULL);
    pthread_mutex_init(&s->thread_lock,NULL);
	
    for(;i<MAX_THREAD_NUM;i++) {
		t = (THREAD *)zmalloc(sizeof(THREAD));    
		pthread_mutex_init(&t->thread_lock,NULL);
		pthread_cond_init(&t->thread_ready,NULL);
		
		pthread_create(&t->thread,NULL,clientThread,t);
		node = arrayNodeCreate(0,t);
		t->owner = node;
		arrayPush(s->thread,node);
    }
	
    for(i = 0;i<MAX_CLIENT_NUM;i++) {
        c = (CLIENT *)zmalloc(sizeof(CLIENT));       
		node = arrayNodeCreate(0,c);
		c->owner = node;
        arrayPush(s->client, node);
    }
	return s;
}

void freeClient(CLIENT *c)
{
	aeDeleteFileEvent(c->el,c->fd,AE_READABLE);
	aeDeleteFileEvent(c->el,c->fd,AE_WRITABLE);
	close(c->fd);
	//返还客户端
	pthread_mutex_lock(&server->client_lock);
	arrayPush(server->client,c->owner);
	pthread_mutex_unlock(&server->client_lock);
}
//内网访问限制
int validIp(char *ip)
{
	char *iplist[]={"192.168.","10.","127.0.0.1"};
	int i;
	for(i =2;i>=0;i--) {
		if(strpos(ip, iplist[i])==0) {
			return 1;
		}
	}
	return 0;
}

void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
    int cport, cfd;
    char cip[INET6_ADDRSTRLEN];
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);
    REDIS_NOTUSED(privdata);
	char *err;
	//printf("Accepted:");
    cfd = anetTcpAccept(err, fd, cip, sizeof(cip), &cport);
    if (cfd == ANET_ERR) {
        printf("Accepting client connection: %s\n",cip);
        return;
    }
    //printf("%s:%d FD:%d \n", cip, cport,cfd);
	//是否内网访问
	if(!validIp(cip)) {
		close(cfd);
		return;
	}
	anetNonBlock(NULL,cfd);
	anetEnableTcpNoDelay(NULL,cfd);
	readFromClient(el,cfd);
}



void threadEnd()
{
	server->thread_num--;
	pthread_exit(NULL);
}

void clientWriteBuffer(CLIENT *c,char *str)
{
	if(str != NULL) {
		strcpy(c->write_buffer,str);
	}
	
	c->unsend_len = strlen(c->write_buffer);
	c->send_len = 0;
}

void writeToClient(CLIENT *c,char *str)
{
	clientWriteBuffer(c,str);
	if (aeCreateFileEvent(c->el,c->fd,AE_WRITABLE,
		writeResultToClient, c) == AE_ERR)
	{
		printf("write event faild\n");
		freeClient(c);
	}
}

void *clientThread(void *arg)
{
	THREAD *t = (THREAD *) arg;
	CLIENT *c;
	pthread_mutex_lock(&t->thread_lock);
	
	while(1) {	
		pthread_cond_wait(&t->thread_ready,&t->thread_lock);
		
		c = t->c;
		if(c != NULL) {
			//读取长度
			if(strlen(c->read_buffer) == 0) {
				writeToClient(c,ERROR_EMPTY_CONTENT);
			} else {
				//RESULT *res = acMatch(acRoot,c->read_buffer);
				RESULT *res = acPinYinMatch(acRoot,c);
				acFillResult(c->write_buffer,res);
				acFreeResult(res);
				writeToClient(c,NULL);
			}
		}

		//返回队列
		pthread_mutex_lock(&server->thread_lock);
		arrayPush(server->thread,t->owner);
		pthread_mutex_unlock(&server->thread_lock);
		//释放锁继续等待
		//pthread_mutex_unlock(&t->thread_lock);
	}
}

void commandProcess(CLIENT *c)
{
	THREAD *t;
	ANODE *node;
	pthread_mutex_lock(&server->thread_lock);
	node = arrayShift(server->thread);
	pthread_mutex_unlock(&server->thread_lock);

	if(node == NULL) {
		writeToClient(c,"097 没有足够的线程处理");	
	} else {
		t = (THREAD *)node->data;
		//修改客户端数据
		t->c = c;
		//唤醒线程
		pthread_cond_signal(&t->thread_ready);
	}
}

void writeResultToClient(aeEventLoop *el, int fd, void *privdata, int mask) 
{
	REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);
    REDIS_NOTUSED(privdata);
	int nwrite = 0,l,writelen;
	CLIENT *c = (CLIENT *) privdata;
	//printf("write FD :%d unsend_len %d \n",c->fd,c->unsend_len);
	if(c->unsend_len > 0) {
		writelen = c->unsend_len;
		writelen = (writelen > SHENHE_IOBUF_LEN) ? SHENHE_IOBUF_LEN:writelen;
		errno = 0;
		nwrite = write(c->fd,c->write_buffer + c->send_len,writelen);
		c->send_len += nwrite;
		c->unsend_len -= nwrite;

		if(c->unsend_len > 0) {
			//printf("writelen: %d unsend %d errno:%d\n",nwrite,c->unsend_len,errno);
			return;
		}
	}
	freeClient(c);
}

void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask) 
{
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);
    REDIS_NOTUSED(privdata);
	
	CLIENT *c = (CLIENT*) privdata;
	int nread,i;
	size_t readlen,len =0;
	readlen = strlen(c->read_buffer);
	errno = 0;
	nread = read(fd, c->read_buffer + readlen, SHENHE_IOBUF_LEN);
	while(nread >0) {
		readlen += nread;
		if(c->read_len == 0 ) {
			if(readlen >= 10) {
				for(i = 9;i>0;i--) {
					len += (c->read_buffer[i]- 48) *  mypow(10 , (9 - i));
				}
				if(len >= MAX_BUFFER_LENGTH) {
					freeClient(c);
					return ;
				}
				//printf("len : %d",len);
				c->read_len = len;
			}
		}
		
		if(nread == 0 && readlen == c->read_len) {
			break;
		}
		errno = 0;
		nread = read(fd, c->read_buffer + readlen, SHENHE_IOBUF_LEN);
		//printf("nread:%d readlen:%d errno:%d\n",nread,readlen,errno);
	}
	
	if (nread == -1) {
		if (errno == EAGAIN) {
			if(c->read_len != 0 && readlen >= c->read_len) {
				
			} else {
				return;
			}
        } else {
			freeClient(c);
            return;
        }
	}
	
	//处理业务逻辑
	aeDeleteFileEvent(c->el,c->fd,AE_READABLE);
	commandProcess(c);
}
void initClient(CLIENT *c)
{
	bzero(c->read_buffer,MAX_BUFFER_LENGTH);
	bzero(c->write_buffer,MAX_BUFFER_LENGTH);
	c->read_len = 0;
	c->send_len = 0;
	c->unsend_len = 0;
}

void readFromClient(aeEventLoop *el,int fd)
{
	
	//CLIENT *c = (CLIENT *)zmalloc(sizeof(CLIENT));
	//选择一个预先申请好的客户端
	CLIENT *c;
	ANODE *node;
	pthread_mutex_lock(&server->client_lock);
	node = arrayShift(server->client);
	pthread_mutex_unlock(&server->client_lock);

	if(node == NULL) {
		//客户端数量不够
		close(fd);
	} else {
		c = (CLIENT *) node->data;
		initClient(c);
		c->el = el;
		c->fd = fd;
		
		if (aeCreateFileEvent(el,fd,AE_READABLE,
		readQueryFromClient, c) == AE_ERR)
		{
			printf("Error on read event\n");
			freeClient(c);
		}
	}
}

void initThread()
{
    //初始化客户端
}

int main()
{
	char *err;
	int port = 8615,fd;
	aeEventLoop *el = aeCreateEventLoop(MAX_CLIENT);
	server = createServer();
	acRoot = acInit();
	
	fd = anetTcpServer(err, port, NULL, SHENHE_TCP_BACKLOG);
	if (aeCreateFileEvent(el, fd, AE_READABLE,
            acceptTcpHandler,NULL) == AE_ERR)
            {
                printf("Unrecoverable error creating fd file event \n");
				return 1;
            }
	printf("Server start %d\n" ,fd);
	aeMain(el);
	aeDeleteEventLoop(el);
	return 0;
}
