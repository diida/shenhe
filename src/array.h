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
void arrayUnshift(ARRAY *arr,ANODE *node);
ANODE *arrayShift(ARRAY *arr);
void arrayPush(ARRAY *arr,ANODE *node);
ANODE *arrayPop(ARRAY *arr);
ARRAY *arrayCreate();
ANODE *arrayNodeCreate(int val,void *data);
