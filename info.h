#include "chat.h"
/*
ä�����α׷��� ��������� �⺻���� ������ ������ �ִ�.
����� �ּ� IP, ��Ʈ����, �޽��� ����, �г��� �̸� ���� define���� ������ ���.
User ����ü�� usd(���� ���� ��ũ����), unum(���� ��ȣ), rsd(�� ���� ��ũ����), admin(��������), unick(���� �̸�)�� ������ �����Ѵ�.
Room ����ü�� rsd(�� ���� ��ũ����), usd(���� ���� ��ũ����), ucnt(���� ����)�� ������ �����Ѵ�.
*/
#define MAX_NICK_LEN    30
#define MAX_MESSAGE_LEN    1024

#define SERV_IP    "127.0.0.1"
#define SERV_PORT    25650

typedef struct _user {
	int usd;
	int unum;
	int rsd;
	int admin;
	char unick[MAX_NICK_LEN];
}User;

typedef struct _room {
	int rsd;
	int usd[100];
	int ucnt;
}Room;