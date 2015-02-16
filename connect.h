#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h>

#define CMD_LOGIN 1
#define CMD_REGISTER 2
#define CMD_EXIT 3
#define CMD_HELP 4
#define CMD_LIST_PEOPLE 5
#define CMD_CLIENT_EXIT 6
#define CMD_SEND_ALL 7
#define CMD_SEND_TO 8
#define CMD_SEND_FROM 11
#define CMD_MESSAGE

#define LOGIN_SUCCESS 9
#define LOGIN_FAIL 10

#define SUCCESS 12
#define FAILED 13

#define OPEN_PORT  7764



typedef struct {
	int cmd;
	char user_name[50];
	char password[50];
	char mess[200];
	char to_user[50];
	int nOnline;
	char people[50][50];
} message;

typedef struct{
	int status;
	char mess[50];
} response;


typedef struct{
	char client_name[50];
	int sock_id;
	int isSet;
	char pass[50];
	int isOnline;
} Client;