#include "info.h"

/*
User 구조체배열, Room 구조배열이 있다.
sd(소켓 디스크립터), rsd(룸 소켓 디스크립터), rcnt(룸 개수), usernum(유저 숫자), ruser(룸 유저숫자), 뮤텍스 변수가 들어있고 server.c에 필요한 함수들이 들어가있다.
*/
User list[100];
Room rlist[100];

int sd;
int rsd;
int rcnt = 0;
int usernum = 0;
int rusernum = 0;
pthread_mutex_t usermutex, roommutex;

void SigExit(int signo);
void *ChatStart(void *user);
void *DMessage(void *user);
void *Notice(void *user);
void Whisper(User user, char *rbuf);
void ClientExit(User user);
void *thmain(void *room);
void FileSend(User user, char *rbuf);
void FileSendALL(User user, char *rbuf);
void Comnd(User user, char *rbuf);
