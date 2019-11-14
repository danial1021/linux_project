#include "info.h"

/*
User ����ü�迭, Room �����迭�� �ִ�.
sd(���� ��ũ����), rsd(�� ���� ��ũ����), rcnt(�� ����), usernum(���� ����), ruser(�� ��������), ���ؽ� ������ ����ְ� server.c�� �ʿ��� �Լ����� ���ִ�.
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
