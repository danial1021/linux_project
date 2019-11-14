#include "info.h"

/*
User구조체 배열이있다.
User 구조체배열, Room 구조배열이 있다.
sd(소켓 디스크립터), rsd(룸 소켓 디스크립터), usernum(유저 숫자), 뮤텍스 변수가 들어있고 client.c 에 필요한 함수들이 들어가있다.
*/
User list[100];

int sd;
int rsd;
int rcnt = 0;
int usernum = 0;
pthread_mutex_t usermutex;

void SigExit(int signo);
int SockSetting(char *ip, int port);
void ChatStart(int ssd);
void *RecvM(void *user);
void *SendM(void *user);
void *thrdmain(void *us);