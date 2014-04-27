#define WORD_MAX_LEN 64
#define MAX_CHILD 256

int mw();
typedef char TINYINT;

typedef struct node
{
	TINYINT is_leaf;
    TINYINT is_word;
    int replace;//是否需要替换
	struct node *next[MAX_CHILD];
	struct node *parent;
	struct node *fail;
	struct node *back;
	char v;
} NODE;

typedef struct res
{
    int count;
    int pos;
	int replace;//是否需要替换
    struct res *next;
} RESULT;

//将utf8 转换 为 unicode 整数
#define UTF8_TO_UNICODE(addr,str) \
		addr = 0;\
		addr |= ((addr_t)str[0] & 0x0F);\
		addr = addr << 6;\
		addr = addr | ((addr_t)str[1] & 0x3F);\
		addr = addr << 6;\
		addr = addr | ((addr_t)str[2] & 0x3F);

NODE *acInit();
RESULT* acMatch(NODE *root,char *str,int map[][3]);
RESULT* acPinYinMatch(NODE *root,CLIENT *cl);

void acFillResult(char *result,RESULT *res);
void acFreeResult(RESULT *res);
void acPinyinInit();
