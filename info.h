#include "chat.h"
/*
채팅프로그램을 만들기위한 기본적인 정보를 가지고 있다.
사용할 주소 IP, 포트정보, 메시지 길이, 닉네임 이름 등을 define으로 정의해 줬다.
User 구조체는 usd(유저 소켓 디스크립터), unum(유저 번호), rsd(룸 소켓 디스크립터), admin(계정권한), unick(유저 이름)의 정보를 보관한다.
Room 구조체는 rsd(룸 소켓 디스크립터), usd(유저 소켓 디스크립터), ucnt(유저 개수)의 정보를 보관한다.
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