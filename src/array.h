typedef struct array_node
{
    struct array_node *next;
    struct array_node *prev;
    int val;
    void *data;
} ANODE;

typedef struct array
{
    ANODE *head;
    ANODE *end;
    int length;
} ARRAY;
void arrayUnshift(ARRAY *arr,ANODE *node,pthread_mutex_t *lock);
ANODE *arrayShift(ARRAY *arr,pthread_mutex_t *lock);
void arrayPush(ARRAY *arr,ANODE *node,pthread_mutex_t *lock);
ANODE *arrayPop(ARRAY *arr,pthread_mutex_t *lock);
ARRAY *arrayCreate();
ANODE *arrayNodeCreate(int val,void *data);
