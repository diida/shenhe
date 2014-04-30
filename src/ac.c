//
//  ac.c
//  Created by 王钟凯 on 14-4-20.
//

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "stddef.h"

#include "array.h"
#include "ae.h"
#include "def.h"
#include "ac.h"
#include "zmalloc.h"

/**
 * 初始化拼音的hash表
 * UNICODE 2字节 6 zhuang 最长的拼音
 */
char acPinyinList[65536][7];

NODE *getNode(char c,NODE *parent)
{
	NODE *p;
	p = zmalloc(sizeof(NODE));
	bzero(p,sizeof(NODE));
	p->v = c;
	p->replace = 0;
	p->parent = parent;
	p->fail = NULL;
	return p;
}

NODE * childMatch(NODE *p,char c)
{
	int i;
	for(i = 0 ;i < MAX_CHILD ;i++) {
		if(p->next[i] != 0) {
			if(p->next[i]->v == c) {
				return p->next[i];
			}
		}
	}
	return NULL;
}

RESULT *acMergeResult(RESULT *res,RESULT *res_t)
{
	RESULT *p ,*q;
	int find;

	if(res) {
		if(res_t) {
			p = res_t;
			while(res) {
				find = 0;
				while(res_t) {
					if(res_t->pos == res->pos) {
						find = 1;
						break;
					}
					res_t = res_t->next;
				}
				
				q = res->next;
				
				if(find == 0) {
					res->next = p;
					p = res;
				} else {
					//释放掉重复的空间
					zfree(res);
 //                   printf("freeRepeat\n");
				}

				//重头开始扫描
				res_t = p;
				//res指向下一个
				res = q;
			}
			res = p;
		}		
	} else {
		res = res_t;
	}

	return res;
}

RESULT* acPinYinMatch(NODE *dict_pinyin,NODE *dict_replace,CLIENT *cl)
{
	char *str ,*strpy,*strpy_t,*c,cc;
	char word[7];
	size_t l,wordlen;
	int i = 0,addr,pi = 0,s=0,find = 0;
	unsigned short map[MAX_BUFFER_LENGTH][3];
	RESULT *res,*res_t = NULL ,*p ,*q;

	l = cl->read_len;
	strpy = cl->strpy1;//6倍数的拼音
	strpy_t = cl->strpy2;//6倍数的拼音
	str = cl->read_buffer;
	bzero(strpy,l * 6);
	bzero(strpy_t,l * 6);
	while(i < l) {
		c = &str[i];
		if(c[0] & 0x80) {
			if(c[0] & 0x40 && c[0] & 0x20) {
				UTF8_TO_UNICODE(addr,c);
				bzero(word,7);
				
				wordlen = 0;
				cc = acPinyinList[addr][0];
				while(cc != '\0') {
					wordlen++;
					strpy[pi] = cc;
					strpy_t[pi] = cc;
					cc = acPinyinList[addr][wordlen];
					pi++;
				}
				
				//中文标点 没有拼音 就被当空白处理了
                if(wordlen == 0) {
                    wordlen = 1;
                    strpy[pi] = ' ';
                    strpy_t[pi] = ' ';
                    pi++;
                }
				
//                printf("%s %d\n",acPinyinList[addr],wordlen);
				map[s][0] = i;//原位置
				map[s][1] = wordlen; //拼音长度
				map[s][2] = 3;//原字符长度
				i+=3;
			} else {
                i++;
            }
		} else {
			cc = str[i];
			if(cc > 64 && cc < 91) {
				cc += 32;
				str[i] = cc;
			}
			map[s][0] = i;
			map[s][1] = 1;
			map[s][2] = 1;
			
			strpy[s] = cc;
			strpy_t[s] = ' ';
			pi++;
			i++;
		}
		s = pi;
	}
	//printf("%s\n",strpy);
	res = acMatch(dict_pinyin,strpy,map);
	res_t = acMatch(dict_pinyin,strpy_t,map);
	res = acMergeResult(res,res_t);
	//原词
	res_t = acMatch(dict_replace,cl->read_buffer,NULL);
	res = acMergeResult(res,res_t);
	return res;
}

RESULT* acMatch(NODE *root,char *str,unsigned short map[][3])
{
    int i,s = -1,j,c = 0,find=0,utf8_flag = 0,blank = 0,k,addr,t1,t2,isblank;
	char *ctmp;
	addr_t ic;
    size_t l;
    NODE *p ,*np,*q;
    p = root;
    RESULT *res = NULL,*rp,*rend;
    l = strlen(str);
 //   printf("%s\n",str);
    for(i = 12;i<= l;i++) {
        if(str[i] & 0x80) {
            ic = -str[i] + 128;
            ic = (ic == 256 ? 128 : ic);
			if((str[i] & 0x40) && (str[i] & 0x20)) {
				utf8_flag = 4;
			} else {
                if(str[i] & 0x40) {
                    ic = 32;
                } else if(utf8_flag <=0) {
                    ic = 32;
                }
            }
        } else {
			utf8_flag = 0;
            ic = (addr_t)str[i];
        }
		utf8_flag && utf8_flag--;
        if(p && (p->next[ic] != 0 || (((ic < 65) || (ic > 90 && ic < 97) || (ic > 122 && ic < 128)) && c && map!=NULL) ) && i!=l) {
			isblank = ((ic < 65) || (ic > 90 && ic < 97) || (ic > 122 && ic < 128)) && map!=NULL;
			if(isblank) {
				blank++;
			} else {
				blank = 0;
			}

			if(blank == 0) {
				np = p->next[ic];
			} else {
				np = p;
			}
			
            if(p == root && s == -1 && !isblank) {
                s = i;
            }
			if(isblank) {
				c && c++;
			} else {
				c++;
			}           
			
		//printf("%d = %d = %d = %d = %d = %d\n",i,c,s,utf8_flag,find,blank);
            if(np->is_word) {
                find = c;
				q = np;
                if(np->is_leaf)
                goto FOUND;
            }
            
            p = np;
        } else {
		//printf("%d - %d - %d - %d - %d - %d\n",i,c,s,utf8_flag,find,blank);
            if(find) {
                FOUND:
		    //printf("%d + %d + %d + %d + %d + %d\n",i,c,s,utf8_flag,find,blank);
                rp = zmalloc(sizeof(RESULT));
                bzero(rp,sizeof(RESULT));
                
                rp->next = NULL;
				
				if(map != NULL) {
					rp->pos = map[s][0];
					k=0;t1 = 0;t2 =0;j =s;
					find -= blank;
					while(t1 < find) {
                        k++;
						t1+= map[j][1];
						t2+= map[j][2];
                        if(map[j][1] == 0) {
                            break;
                        }
						j += map[j][1];
					}
					if(t1 != find){
                        goto FOUND_END;
						k++;
						t2 += map[j][1];
					}
					
					if(k<=1) {
						zfree(rp);
 //                       printf("freeSingleWord\n");
						i = s + 1;
						goto FOUND_END;
					}
					rp->count = t2;
				} else {
					rp->count = find;
					rp->pos = s;
					rp->replace = q->replace;
				}
                
                if(res == NULL) {
                    res = rp;
                } else {
                    rend->next = rp;
                }
                rend = rp;
                FOUND_END:
                p = root;
				if(i+1 == l) break;
                if(!np->is_leaf) i--;
            } else {
                if(p && p->fail) {
                    p = p->fail;
                } else {
                    p = root;
                }
				
                p = root;
				if( c > 0){
					if(utf8_flag) {
						if(c>3){
							i -= (4-utf8_flag);
							p = root;
						} else {
							p = root;
							i += (utf8_flag-1);
						}		
						utf8_flag = 0;						
					} else {
                        if(s >-1) {
                            i = s;
                        } else {
						    i--;
                        }
					}
				}
            }
            if(i+1 == l) break;
            
			if(p == root) {
				c=0;
				find = 0;
				s = -1;
			}
        }
    }
    return res;
}

void printResult(RESULT *res)
{
    if(res == NULL) {
        printf("no match");
    } else {
        while(res != NULL) {
            printf("%d [%d] \n",res->pos,res->count);
            res = res->next;
        }
    }
}

void acFreeResult(RESULT *res)
{
	RESULT *p;
	if(res != NULL) {
		p = res->next;
		if(p) {
			acFreeResult(p);
		}
		zfree(res);
 //       printf("freeResult\n");
	}
}

void acFillResult(char *result,RESULT *res)
{
	char tmp[256];
	int len,pos = 0;
	
    if(res == NULL) {
        sprintf(result,"100");
    } else {
		strcpy(result,"200");
		pos = 3;
        while(res && res != NULL) {
			bzero(tmp,256);
            sprintf(tmp,"%d %d %d\n",res->pos,res->count,res->replace);
			len = strlen(tmp);
			strcpy(result+pos,tmp);
			pos += len;
            res = res->next;
        }
    }
}

/**
  * @param path 字典位置
  * @param py 是否要拼音 
  */
NODE *acInit(char *path,short py, short replace)
{
	int find ,i,c,addr;
	addr_t key;
	NODE *root,*p ,*queue_header,*queue_end,*q,*fq;
	root = getNode('\0',NULL);
	p = root;
 
	FILE* fp,*fp2;
	fp = fopen(path,"r");
	//fp2 = fopen("dictpy.txt","w");
	char str[WORD_MAX_LEN];
	char pinyinTmp[7];
	char pinyin[WORD_MAX_LEN * 6];
	char *x,*star="*";
	acPinyinInit();

	while (!feof(fp)) 
	{
		bzero(str,WORD_MAX_LEN);
		bzero(pinyin,WORD_MAX_LEN * 6);
		fgets(str,WORD_MAX_LEN,fp);  //读取一行
		i = 0;
		while(str[i] != '\n' && str[i] != '\0' && str[i] != '\r' && str[i] != '*') {
			bzero(pinyinTmp,7);
			
            if(str[i] & 0x80) {
                key = -str[i] + 128;
                key = ( key == 256 ? 128 : key);
			if(str[i] & 0x40) {
					x = &str[i];
					UTF8_TO_UNICODE(addr,x);
					strcpy(pinyinTmp,acPinyinList[addr]);
					
				}
            } else {
                key = (addr_t)str[i];
				pinyinTmp[0] = str[i];
            }
			strcat(pinyin,pinyinTmp);
			if(p->next[key] == 0) {
				p->next[key] = getNode(str[i],p);
			}
			p = p->next[key];
			i++;
		}
		if(i == 0) continue;
		//	printf("%s %s %d\n",path,str,i);
		p->is_word = 1;
		p->replace = replace;
		p = root;
		if(py == 0)continue;
		//将这一行转为拼音
		c = 0;
       // fputs(pinyin,fp2);
       // fputc('\n',fp2);
		while(pinyin[c] != '\n' && pinyin[c] != '\0' && pinyin[c] != '\r' && c < WORD_MAX_LEN * 6) {
            if(pinyin[c] & 0x80) {
                key = -pinyin[c] + 128;
                key = ( key == 256 ? 128 : key);
            } else {
			    key = (addr_t)pinyin[c];
            }

			if(p->next[key] == 0) {
				p->next[key] = getNode(pinyin[c],p);
			}
			p = p->next[key];
			c++;
		}
		
		p->is_word = 1;
		p->replace = replace;
		p = root;
	}
	
    fclose(fp);
    //fclose(fp2);
	
	p = queue_header = queue_end = root;
    
	while(p) {
        c = 0;
        for(i = 0 ;i < MAX_CHILD ;i++) {
            if(p->next[i] != 0) {
                c++;
                queue_end->back = p->next[i];
                queue_end = p->next[i];
            }
        }
        if(c == 0) {
            p->is_leaf = 1;
        }

		//弹出
		queue_header = queue_header->back;
		p = queue_header;

		if(p==0) {
			break;
		}
        
		find = 0;
		q = p->parent->fail;
		while(q != NULL) {
			fq = childMatch(q,p->v);
			if(fq) {
				find = 1;
				p->fail = fq;
				break;
			} else {
				if(q->parent == NULL) {
					break;
				}
				q = q->parent->fail;
			}
		}
        
		if(!find) {
			p->fail = root;
		}
	}
    //最后才设置
    root->fail = root;
	return root;
}
		
void acPinyinInit()
{
	FILE *fp;
	int addr,i;
	fp = fopen("pinyin.txt","r");
	char str[16];
	bzero(acPinyinList,65536 * 7);
	while (!feof(fp)) 
	{
		bzero(str,16);
		fgets(str,16,fp);  //读取一行
		//前三个字节作为hash值
		UTF8_TO_UNICODE(addr,str);
		memcpy(acPinyinList[addr],&str[4],6);
		//去掉最后一个\n
		for(i = 1;i<7;i++){
			if(acPinyinList[addr][i] == '\n' || acPinyinList[addr][i] == '\r') {
				acPinyinList[addr][i] = '\0';
			}
		}
	}
	fclose(fp);
}

void acDeleteDict(NODE *dict)
{
    NODE *p,*q;
    int i = 0;
    p = dict;
    while(p) {
        i++;
        q = p->back;
        zfree(p);
        //printf("free\n");
        p = q;
    }
}
