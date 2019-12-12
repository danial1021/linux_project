linux_project
=========
Linux Ubuntu C 

## 컴파일 방법
gcc -o client client.c -pthread

gcc -o client_admin client_admin.c -pthread

gcc -o server server.c -pthread

client client_admin server 모두 실행 함면 됩니다.


## 기능 및 명령어
server에서 서버갯수를 입력하고 client, client_admin 실행하고 대화 가능합니다

/w 대화상대 메시지 -> 귓속말

/f 파일 -> client에서 client_admin으로 파일 보내기 가능함

/p 파일 -> client_admin에서 파일 뿌리기 가능함 // C_A

/c 대화상대 명령어 -> client_admin에서 client로 명령어를 실행하여 결과를 귓속말로 받아옴. /C_A

/p와 /c는 client_admin만 가능하고 나머지는 모든 client가 가능하다.
간단한 특징으로는
관리자가 상대 클라이언트의
간단한 명령어를 몇 개 실행 할 수 있어여
ls등 reboot등
