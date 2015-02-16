#include "connect.h"

int isLogin = 0;
int sock;
char username[50];
pthread_t receive_thread;

void* receive_handle(void*);
void login();
void list_people();
void chat_with_people();
void terminate();
void process();


void login(){
	printf("Login function");

	message msg;
	response rep;
	bzero(&rep, sizeof(rep));
	while(isLogin == 0){
		sleep(1);
		bzero(&username,sizeof(username));
		printf("Please choose:\n");
		printf("1.Login\n");
		printf("2.Register\n");
		printf("3.Exit\n");
		int choice = 0;
		
		int isValid = 0;
		
		while(isValid == 0){
			printf("Enter your choose: ");
			choice = 0;
			fflush(stdin);
			if(scanf("%d", &choice) != 1 ){
				printf("Unknow command!\n");
			}else{
				isValid = 1;
			}
		}
		printf("You choose %d \n",choice);
		switch(choice)
		{
			case 1:
				msg.cmd = CMD_LOGIN;
				break;
			case 2:
				msg.cmd = CMD_REGISTER;
				break;
			case 3:
			default:
				printf("Unknow command!\n");
				continue;
		}
		printf("Enter your username: ");
		fflush(stdin);
		scanf("%s",msg.user_name);
		strcpy(username,msg.user_name);
		printf("Enter your password: ");
		fflush(stdin);
		scanf("%s",msg.password);
		//Send some data
        if( send(sock , &msg , sizeof(msg) , 0) < 0)
        {
            puts("Send failed");
        }
        int isWaiting = 1;
        switch(msg.cmd){
        	case CMD_LOGIN:
        		printf("Try to login as %s \n", msg.user_name);
        		break;
        	case CMD_REGISTER:
        	   	printf("Try to register as %s \n", msg.user_name);
        		break;
        }
     
        puts("Waiting server response....");
        while(isWaiting == 1){
	        if( recv(sock , &rep , sizeof(rep) , 0) < 0)
	        {
	            puts("recv failed");
	            break;
	        }else{
	        	switch(rep.status){
	        		case LOGIN_SUCCESS:
	        			puts("login success\n");
	        			isWaiting = 0;
	        			isLogin = 1;
	        			break;
	        		case LOGIN_FAIL:
	        			puts(rep.mess);
	        			isWaiting = 0;
	        			break;
	        		case SUCCESS:
	        			puts(rep.mess);
	        			isWaiting = 0;
	        			break;
	        		case FAILED:
	        			puts(rep.mess);
	        			isWaiting = 0;
	        			break;
	        		default:
	        			isWaiting = 1;
	        	}
	        }
    	}
     
	}
	return;
}

void list_people(){
	message msg;
	bzero(&msg, sizeof(msg));
	msg.cmd = CMD_LIST_PEOPLE;
	if( send(sock , &msg , sizeof(msg) , 0) < 0)
    {
            puts("Request fail");
    }
}

void chat_with_people(){
	char send_message[100];
	printf("Enter message in form: [name]|[message]\n");
	puts("Enter your message: ");
	fflush(stdin);
	fgets(send_message,100,stdin);

	//build send message
	message msg;
	char*tok;
    tok = strtok(send_message,"|");
    strcpy(msg.to_user, tok);
    tok = strtok(NULL,"|");
    strcpy(msg.mess, tok);
    strcpy(msg.user_name,username);
	msg.cmd = CMD_SEND_TO;

	//send message
	printf("Sending to %s..., message = %s",msg.to_user,msg.mess);
	if( send(sock , &msg , sizeof(msg) , 0) < 0)
    {
        puts("Send failed");
    }

}

void terminate(){
	pthread_cancel(receive_thread);
	message msg;
	msg.cmd = CMD_EXIT;
	strcpy(msg.user_name,username);
	if( send(sock , &msg , sizeof(msg) , 0) < 0)
    {
        puts("Send failed");
    }
}
void process(){
	while(1 == 1){
		puts("Please choose: ");
		puts("1. List online people");
		puts("2. Send message to all");
		puts("3. Send message to a person");
		puts("4. Exit");
		
		int choice = 0;
		printf("Enter your choose: ");
		fflush(stdin);
		scanf("%d",&choice);
		int c;
		do{
    		 c = getchar();
		}while(c != EOF && c != '\n');
		switch(choice){
			case 1:
				list_people();
				break;
			case 2:
				printf("Not implement this function\n");
				break;
			case 3:
				chat_with_people();
				break;
			case 4:
				terminate();
				return;
			default:
				printf("Unknow command!\n");
				break;
		}

	}
}
 
int main(int argc , char *argv[])
{
    struct sockaddr_in server;
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
  //  server.sin_addr.s_addr = inet_addr("127.0.0.1");
   // server.sin_family = AF_INET;
   // server.sin_port = htons( OPEN_PORT );
 	
 	int port_client;
 	char host_name[20];
 	int isConnected = 0;
 	while(isConnected == 0){
 		printf("Enter host (ex: 127.0.0.1) :");
 		fflush(stdin);
 		scanf("%s",host_name);
 		printf("Enter port: ");
 		fflush(stdin);
 		scanf("%d",&port_client);
 		server.sin_addr.s_addr = inet_addr(host_name);
    	server.sin_family = AF_INET;
    	server.sin_port = htons( port_client );
    	    //Connect to remote server
	    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        perror("connect failed. Error");

	    }else{
	        isConnected = 1;
	   		 puts("Connected\n");
	    }
 	}


    //login
    login();

    //after lofin, create a thread to receive message from server
    int* new_sock = malloc(1);
    *new_sock = sock;
    if(pthread_create(&receive_thread,NULL,receive_handle,(void*)new_sock)){
    	perror("Cannot receive message from server\n");
    	return 1;
    }
   
    process();
       
    close(sock);
    return 0;
}

void*receive_handle(void*socket_desc){

    int sock = *(int*)socket_desc;
    int read_size;
    message msg;
     
    //Receive a message from server
    while( (read_size = recv(sock , &msg , sizeof(msg) , 0)) > 0 )
    {
    	//printf("Receiving message\n");
        switch(msg.cmd){
        	case CMD_SEND_FROM:
        		printf("\nMessage FROM: %s. CONTENT: %s \n",msg.user_name,msg.mess);
        		bzero(&msg,sizeof(msg));
        		break;
            case CMD_EXIT:
            	break;
            case CMD_LIST_PEOPLE:
            //	puts("List online people function");
            	printf("There are %d online people. They are: \n",msg.nOnline);
            	for(int i = 0;i< msg.nOnline;i++){
            		printf("%s\n",msg.people[i]);
            	}
            	bzero(&msg,sizeof(msg));
                break;
        }
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    //Free the socket pointer
    free(socket_desc);
     
    return 0;
}