#include "info.h"

/*
User����ü �迭���ִ�.
User ����ü�迭, Room �����迭�� �ִ�.
sd(���� ��ũ����), rsd(�� ���� ��ũ����), usernum(���� ����), ���ؽ� ������ ����ְ� client.c �� �ʿ��� �Լ����� ���ִ�.
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