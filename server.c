#include "server.h"

int main()
{
	int room, i;

	signal(SIGINT, SigExit);

	pthread_mutex_init(&usermutex, NULL);
	pthread_mutex_init(&roommutex, NULL);

	printf("서버 갯수 입력 : \n"); //서버의 개수를 할당받고 thmain()함수를 호출한다.
	scanf("%d", &room);
	getchar();

	pthread_t ptr[256];

	for (i = 0; i<room; i++)
	{
		pthread_create(&ptr[i], NULL, thmain, &room);
		pthread_detach(ptr[i]);
	}

	while (1)
		pause();

	return 0;
}

void *thmain(void *room)
{
	pthread_mutex_lock(&roommutex); //쓰레드간에 충돌을 방지하기 위해 뮤텍스를 사용하였다.
	User user;// = *(User *)us;
	Room rm;

	int ssd;
	int port = SERV_PORT + rcnt;
	rm.ucnt = rcnt;

	ssd = socket(AF_INET, SOCK_STREAM, 0);
	//----> 기본적인 소켓 할당과정
	struct sockaddr_in servaddr = { 0 };
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (bind(ssd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
	{
		perror("bind error");
		close(ssd);
	}

	if (listen(ssd, 5) == -1)
	{
		perror("listen error");
		close(ssd);
	}

	printf("%d번 서버 구동\n", rcnt+1);

	rcnt++;
	//<------- 기본적인 소켓 할당과정
	pthread_mutex_unlock(&roommutex); //뮤텍스를 풀어준다.

	struct sockaddr_in cliaddr = { 0 };
	int clen = sizeof(cliaddr);
	pthread_t ptr;
	//user를 받고 user정보를 구조체에 등록해준다.
	while ((user.usd = accept(ssd, (struct sockaddr *)&cliaddr, &clen)))
	{
		if (user.usd == -1)
		{
			perror("accept");
			return 0;
		}

		pthread_mutex_lock(&usermutex);

		user.unum = usernum;

		user.rsd = ssd;
		rm.rsd = ssd;
		rm.usd[rm.ucnt] == user.usd;

		rlist[rm.ucnt] = rm;
		pthread_create(&ptr, NULL, ChatStart, &user); //ChatStart() 실행한다.
		pthread_detach(ptr);

		usernum++;
		rm.ucnt++;

		pthread_mutex_unlock(&usermutex);
	}

	return 0;
}

void *ChatStart(void *user)
{
	User us = *(User *)user;
	char nick[MAX_NICK_LEN] = "";
	char root[6] = "";
	pthread_t ptr[2];

	send(us.usd, "서버 접속성공!!!\n학번을 입력하세요.\n", 100, 0); //접속 문구를 client에게 전달
	recv(us.usd, nick, sizeof(nick), 0); //userID를 할당받는다.
	nick[strlen(nick) - 1] = '\0';
	printf("%s가 접속하였습니다.\n", nick);
	strcpy(us.unick, nick);
	recv(us.usd, root, sizeof(root), 0); //계정 종류를 받는다 (1 = admin, 0 = user)
	puts(root);
	if(!strncmp(root,"admin",6))
		us.admin =1;
	else us.admin =0;
	
	
	list[us.unum] = us;

	pthread_create(&ptr[0], NULL,DMessage, &us);
	pthread_create(&ptr[1], NULL, Notice, &us);
	pthread_join(ptr[0], NULL);
	pthread_join(ptr[1], NULL);
}

void *DMessage(void *user)
{
	User us = *(User *)user;
	int i;
	char *whisp;
	char rbuf[MAX_MESSAGE_LEN];
	char sbuf[MAX_MESSAGE_LEN];

	while (recv(us.usd, rbuf, sizeof(rbuf), 0) >0)
	{
		printf("%s :  %s\n", us.unick, rbuf);
		if (!strncmp(rbuf, "/w", 2)) // /w문구가 있으면 귓속말을 보내주는 Whisper()를 실행
		{
			Whisper(us, rbuf);
			memset(rbuf, 0, sizeof(rbuf));
			continue;
		}
		else if (!strncmp(rbuf, "/f", 2)) // /f 문구가 파일을 admin에게 보대주는 FileSend()를 실행
		{
			FileSend(us, rbuf);
			memset(rbuf, 0, sizeof(rbuf));
			continue;
		}
		else if (!strncmp(rbuf, "/p", 2) && us.admin) // /p 문구가 있으면 user들에게 파일을 뿌리는 FileSendALL()를 실행 admin만 실행할 수 있다.
		{
			FileSendALL(us, rbuf);
			memset(rbuf, 0, sizeof(rbuf));
			continue;
		}
		else if (!strncmp(rbuf, "/c", 2) && us.admin) // /c 문구가 명령어를 실행시키는 있으면 Comnd()를 실행 admin만 실행할 수 있다.
		{
			Comnd(us, rbuf);
			memset(rbuf, 0, sizeof(rbuf));
			continue;
		}
		else
		{
			sprintf(sbuf, "%s : %s", us.unick, rbuf);  //아무런 특수 문자열이 없으면 그대로 출력
			for (i = 0; i<usernum; i++)
			{
				if (list[i].usd == us.usd)
					continue;
				if (list[i].rsd == us.rsd)
					send(list[i].usd, sbuf, sizeof(sbuf), 0);
			}
		}

		memset(rbuf, 0, sizeof(rbuf));
		memset(sbuf, 0, sizeof(sbuf));
	}

	ClientExit(us);
}

void FileSend(User user, char *rbuf)
{

	char *fio;
	char* buf;
	char sbuf[1024];
	char filesize[1024];
	int size;
	int i;
	strtok(rbuf, " "); //받은 문자열들을 " "로 분리시킨다.
	fio = strtok(NULL, " ");
	fio[strlen(fio) - 1] = '\0';


	recv(user.usd, filesize, sizeof(filesize), 0); //파일크기 정보를 먼저 받는다.
	
	size = atoi(filesize);
	buf = malloc(size + 1); //받은 파일 크기만큼 buf를 할당한다.
	memset(buf, 0x00, size );
	read(user.usd, buf, size ); //파일을 받는다.
	sprintf(sbuf, "/f %s", fio);
	for (i = 0; i<usernum; i++) 
	{
		if (list[i].admin) //admin을 찾아서 파일을 보냄
		{
			send(list[i].usd, sbuf, strlen(sbuf), 0); // /f 문구를 먼저 전송
			sleep(1);
			send(list[i].usd, filesize, sizeof(filesize), 0); // 파일크기정보 전송
			sleep(1);
			write(list[i].usd, buf, size); //파일 정보 전송
			sleep(1);
			printf("Admin에게 파일전송 성공!\n");
			
		}
	}
	memset(buf, 0x00, size );
	free(buf);
	
}

void FileSendALL(User user, char *rbuf)
{
	char *fio;
	char* buf;
	char sbuf[1024];
	char filesize[1024];
	int size;
	int i,count;

	strtok(rbuf, " ");
	fio = strtok(NULL, " ");
	fio[strlen(fio) - 1] = '\0';


	recv(user.usd, filesize, sizeof(filesize), 0); //파일크기 정보를 먼저 받는다.

	size = atoi(filesize);
	buf = malloc(size + 1); //받은 파일 크기만큼 buf를 할당한다.
	memset(buf, 0x00, size);
	read(user.usd, buf, size); //파일을 받는다.
	sprintf(sbuf, "/p %s", fio);
	for (i = 0; i<usernum; i++) 
	{
		if (!list[i].admin) //admin이 아닌 모든 user에게 전송
		{
			send(list[i].usd, sbuf, strlen(sbuf), 0); // /f 문구를 먼저 전송
			sleep(1);
			send(list[i].usd, filesize, sizeof(filesize), 0); // 파일크기정보 전송
			sleep(1);
			write(list[i].usd, buf, size); //파일 정보 전송
			sleep(1);
			printf("%s 에게 파일전송 성공!\n", list[i].unick);
			
		}
	}
	memset(buf, 0x00, size);
	free(buf);

}

void Comnd(User user, char *rbuf)
{
	int i = 0, j = 2;
	char *whisp[12400];
	char *ptr;
	char command[1024] = "";
	char sbuf[MAX_MESSAGE_LEN];
	rbuf[strlen(rbuf) - 1] = '\0';
	ptr = strtok(rbuf, " "); 
	while (ptr != NULL)
	{
		whisp[i++] = ptr;
		ptr = strtok(NULL, " "); //받은 문자열들을 " "로 분리시킨다.
	}
	for (i = 0; i<usernum; i++)
	{
		if (!strcmp(whisp[1], list[i].unick))
		{
			while (whisp[j] != NULL)
			{
				sprintf(command, "%s %s", command, whisp[j++]); //보낼 명령어들만 합친다.
			}
			sprintf(sbuf, "command: %s %s", command, user.unick); // command: 명령어 유저이름 현태로 파일을 만든다.
			if (list[i].rsd == user.rsd)
				send(list[i].usd, sbuf, strlen(sbuf), 0); //파일전송
			break;
		}
	}

	if (i == usernum)
		send(user.usd, "사용자가 없습니다.\n", 30, 0);
}

void ClientExit(User user)
{
	int i, j;
	char sbuf[MAX_MESSAGE_LEN];

	sprintf(sbuf, "%s님이 퇴장하였습니다.\n", user.unick);

	for (i = 0; i<usernum; i++)
	{
		if (list[i].usd == user.usd)
		{
			for (j = 0; j<usernum; j++)
			{
				if (list[j].usd == user.usd)
					continue;
				if (list[j].rsd == user.rsd)
					send(list[j].usd, sbuf, sizeof(sbuf), 0);
			}
			break;
		}
	}

	pthread_mutex_lock(&usermutex);
	for (j = i; j<usernum - 1; j++)
	{
		list[j] = list[j + 1];
	}
	usernum--;
	pthread_mutex_unlock(&usermutex);
}

void Whisper(User user, char *rbuf)
{

	int i = 0, j = 2,c=0;
	char *whisp[12400];
	char *ptr;
	char sbuf[MAX_MESSAGE_LEN];

	ptr = strtok(rbuf, " ");
	while (ptr != NULL)
	{
		whisp[c++] = ptr;
		ptr = strtok(NULL, " "); //받은 문자열들을 " "로 분리시킨다.
	}
	for (i = 0; i<usernum; i++)
	{
		if (!strcmp(whisp[1], list[i].unick)) //유저이름이 있으면 찾아서 전송한다.
		{
			sprintf(sbuf, "%s (whisper) : ", user.unick); // 유저 (whisper) 문자열 형태로 만든다.
			while (j != c)
			{
				sprintf(sbuf, "%s %s", sbuf, whisp[j++]);
			}
			if (list[i].rsd == user.rsd)
				
				send(list[i].usd, sbuf, strlen(sbuf), 0); // 메세지 전송
			break;
		}
	}

	if (i == usernum)
		send(user.usd, "사용자가 없습니다.\n", 30, 0);
}

void *Notice(void *user)
{
	int i;
	User us = *(User *)user;
	char sbuf[MAX_MESSAGE_LEN];
	char inbuf[MAX_MESSAGE_LEN];

	sprintf(sbuf, "%s님이 접속하였습니다.\n", us.unick); 
	for (i = 0; i<usernum; i++) //모든 유저에게 접속을 알린다.
	{
		if (list[i].rsd == us.rsd)
			send(list[i].usd, sbuf, strlen(sbuf), 0);
	}
	memset(sbuf, 0, sizeof(sbuf));
	while (1) // 문자열을 입력하면 알림사항으로 모든 유저에게 알린다.
	{
		memset(sbuf, 0, sizeof(sbuf));
		memset(inbuf, 0, sizeof(inbuf));
		fgets(inbuf, sizeof(inbuf), stdin);
		sprintf(sbuf, "[알림사항] : %s\n", inbuf);
		for (i = 0; i<usernum; i++)
			send(list[i].usd, sbuf, strlen(sbuf), 0);
	}
}



void SigExit(int signo)
{
	printf("서버를 종료합니다.\n");
	close(rsd);
	close(sd);
	exit(0);
}

