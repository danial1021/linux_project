#include "client.h"

int main()
{
	int port;
	char pt[MAX_MESSAGE_LEN];
	//접속할 수 있는 강의 목록들 
	puts("------강의 목록------");
	puts("1. 유닉스프로그램즈");
	puts("2. 자료구조");
	puts("3. 컴퓨터구조");
	printf("들어가실 서버를 선택해주세요 :");
	scanf("%d", &port);
	getchar();
	signal(SIGINT, SigExit);
	port = port + SERV_PORT - 1;

	pthread_mutex_init(&usermutex, NULL);

	if ((sd = SockSetting(SERV_IP, port)) == -1) //해당 강의 서버로 접속한다.
	{
		perror("socket");
		return 0;
	}

	ChatStart(sd);

	return 0;
}

void ChatStart(int ssd)
{
	User user;
	pthread_t ptr[2];
	char nick[MAX_NICK_LEN] = "";
	char rbuf[MAX_MESSAGE_LEN] = "";
	char* root = "admin";
	recv(ssd, rbuf, 100, 0);
	fputs(rbuf, stdout);
	fgets(nick, sizeof(nick), stdin);
	send(ssd, nick, strlen(nick), 0); //유저 이름을 보낸다.
	sleep(1);
	send(ssd, root, strlen(root), 0);  //관리자 계정임을 서버에 알린다.
	user.usd = ssd;
	strcmp(user.unick, nick);

	pthread_create(&ptr[0], NULL, RecvM, &user); // RecvM() 실행
	pthread_detach(ptr[0]);
	pthread_create(&ptr[1], NULL, SendM, &user); // SendM() 실행
	pthread_detach(ptr[1]);

	while (1)
		pause();
}

void *RecvM(void *user)
{
	User us = *(User *)user;
	char rbuf[MAX_MESSAGE_LEN];
	
	char command[255] = "";
	char from[255] = "";
	char line[124] = "";
	char result[10240] = "";
	int count = 0;
	while (1)
	{
		
		while (recv(us.usd, rbuf, sizeof(rbuf), 0) > 0)
		{
			if (!strncmp(rbuf, "command: ", 9)) // command: 라는 문자열이 있으면 실행한다.
			{
				char * ptr,*last,*token;
				int i = 0,j;
				char *ptarr[10] = { NULL, };
				ptr = strtok(rbuf, " ");
				ptr = strtok(NULL, " ");
				while (ptr != NULL)
				{
					ptarr[i++] = ptr;
					
					ptr = strtok(NULL, " "); //문자열을 " "단위로 분리 
				}
				for(j=0 ; j<i-1; j++)
				sprintf(command, "%s %s", command, ptarr[j]); //명령어 부분만 합친다.
				sprintf(from, "%s", ptarr[i - 1]);
				sprintf(result, "/w %s \n", from); //귓속말 형태로 보낼 준비를 한다.
				FILE *p = popen(command, "r"); //명령어 결과를 파일포인터에 저장한다.
				if (p != NULL) {
					while (fgets(line, sizeof(line), p) != NULL)
						sprintf(result, "%s %s", result, line); //파일포인터에 저장된 결과를 출력한다.
				}
				else
				{
					sprintf(result, "%s %s", result, "fault result");
				}
				while (result[count++] != '\0');
				send(us.usd, result, count, 0); //명령어 전송 결과를 보낸다.
				memset(result, 0, sizeof(result));
				memset(command, 0, sizeof(command));
				memset(line, 0, sizeof(line));
				memset(rbuf, 0, sizeof(rbuf));
				pclose(p);
				continue;
			}
			else if(!strncmp(rbuf, "/f ", 2))// /f 라는 문자열이 있으면 실행한다.
			{
				char *fio,*buf;
				FILE *fp;
				char filesize[1024];
				int size;
				strtok(rbuf, " ");
				fio = strtok(NULL, " ");
				fp = fopen(fio,"wb"); //fio에있는 문자열의 이름의 파일을 생성한다.
				recv(us.usd, filesize, sizeof(filesize), 0); //파일 사이즈를 받는다
				size = atoi(filesize);
				buf = malloc(size + 1);
				memset(buf, 0x00, size);
				read(us.usd, buf, size); // 파일 바이너리를 받아온다.
				fwrite(buf, 1, size, fp); // 바이너리 파일을 쓴다.
				fclose(fp);
				free(buf);
				memset(rbuf, 0, sizeof(rbuf));
				memset(filesize, 0, sizeof(filesize));
				continue;
			}
			else
			{
				fputs(rbuf, stdout);
				memset(rbuf, 0, sizeof(rbuf));
			}
		}
	}
}

void *SendM(void *user)
{
	User us = *(User *)user;
	char sbuf[MAX_MESSAGE_LEN];
	char rbuf[MAX_MESSAGE_LEN];
	char *buf;
	char filesize[1024];
	FILE *sourse_fd;
	char*fio;
	int size;
	while (1)
	{
		fgets(sbuf, sizeof(sbuf), stdin);
		strcpy(rbuf, sbuf);

		if (!strncmp(sbuf, "/f", 2)) { // /f 라는 문자열이 있으면 실행한다.
			strtok(sbuf, " ");
			fio = strtok(NULL, " ");
			fio[strlen(fio) - 1] = '\0';
			sourse_fd = fopen(fio, "rb"); //fio 문자열 이름의 파일을 바이너리 형태로 읽는다.
			if (sourse_fd == NULL) {
				memset(sbuf, 0, sizeof(sbuf));
				memset(rbuf, 0, sizeof(rbuf));
				continue;
			}
			send(us.usd, rbuf, sizeof(rbuf), 0);  //파일 이름은 먼저 보낸다.
			fseek(sourse_fd, 0, SEEK_END);    
			size = ftell(sourse_fd);          // 파일 크기를 구한다.
			sleep(1);
			sprintf(filesize, "%d", size);
			send(us.usd, filesize, strlen(filesize), 0); //파일크기보냄
			sleep(1);
			buf = malloc(size + 1); //파일크기만큼 동적할당한다. 
			memset(buf, 0, size);
			fseek(sourse_fd, 0, SEEK_SET);
			fread(buf, size, 1, sourse_fd); //buf에  파일을 읽고 저장한다.

			write(us.usd, buf, size);  //파일을 전송한다.
			memset(buf, 0, size);
			fclose(sourse_fd);
			free(buf);
		}
		else if (!strncmp(sbuf, "/p", 2)) // /p 라는 문자열이 있으면 실행한다.
		{
			strtok(sbuf, " ");
			fio = strtok(NULL, " ");
			fio[strlen(fio) - 1] = '\0';
			sourse_fd = fopen(fio, "rb"); //fio 문자열 이름의 파일을 바이너리 형태로 읽는다.
			if (sourse_fd == NULL) {
				memset(sbuf, 0, sizeof(sbuf));
				memset(rbuf, 0, sizeof(rbuf));
				continue;
			}
			send(us.usd, rbuf, sizeof(rbuf), 0); //파일 이름은 먼저 보낸다.
			fseek(sourse_fd, 0, SEEK_END);    
			size = ftell(sourse_fd);             // 파일 크기를 구한다.
			sleep(1);
			sprintf(filesize, "%d", size);
			send(us.usd, filesize, strlen(filesize), 0); //파일크기보냄
			sleep(1);
			buf = malloc(size + 1); //파일크기만큼 동적할당한다. 
			memset(buf, 0, size);
			fseek(sourse_fd, 0, SEEK_SET);
			fread(buf, size, 1, sourse_fd); //buf에  파일을 읽고 저장한다.

			write(us.usd, buf, size);  //파일을 전송한다.
			memset(buf, 0, size);
			fclose(sourse_fd);
			free(buf);
		}
		else
		{
			send(us.usd, sbuf, sizeof(sbuf), 0); //일반 메세지를 보낸다.
		}


		memset(sbuf, 0, sizeof(sbuf));
		memset(rbuf, 0, sizeof(rbuf));
	}
}

int SockSetting(char *ip, int port) //소켓을 연결하는 함수이다.
{
	int ssd;

	if ((ssd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("sock error");
		return -1;
	}

	struct sockaddr_in servaddr = { 0 };
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	if (connect(ssd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		perror("connect");
		return -1;
	}

	return ssd;
}

void SigExit(int signo)
{
	printf("클라이언트를 종료합니다.\n");
	close(sd);
	exit(0);
}

