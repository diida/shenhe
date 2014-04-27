#if __WORDSIZE == 64
typedef long int	intptr_t;
#else
typedef int	intptr_t;
#endif

#if __WORDSIZE == 64
typedef long int addr_t;
#else
typedef int	addr_t;
#endif