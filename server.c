#include "connect.h"


Client client[50];
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int port_server;
 
void *connection_handler(void *);
void login(int sock, message msg);
void client_register(int sock,message msg);
void client_exit(int sock,char client_name[]);
void who_is_online(int sock);
void send_to(message inmess);
void init();


void client_exit(int sock,char client_name[]){
    pthread_mutex_lock( &mutex1 );
    for(int i = 0;i < 50;i++){
       
        if(!strcmp(client_name,client[i].client_name) && (sock == client[i].sock_id)){
             printf("Delete client %s from array \n",client_name);
            client[i].isOnline = 0;
            client[i].sock_id = 0;
        }else{
            
        }
    }
    pthread_mutex_unlock( &mutex1 );
}

void client_register(int sock,message msg){
    pthread_mutex_lock( &mutex1 );
    response res;
    bzero(&res, sizeof(res));
    printf("Client register with username = %s, password = %s \n",msg.user_name,msg.password);
    for(int i = 0;i<50;i++){
        if(!strcmp(msg.user_name,client[i].client_name)){
            res.status = FAILED;
            strcpy(res.mess,"Client name existed");
            write(sock, &res, sizeof(res));
            return;
        }
    }

    FILE* file = fopen("user.db", "a+");
    if(file != NULL)
    {
        fprintf(file, "%s|%s\n", msg.user_name,msg.password);
        res.status = SUCCESS;
        strcpy(res.mess,"Register success");
        for(int i = 0;i<50;i++){
            if(client[i].isSet == 0){
                client[i].isSet = 1;
                client[i].sock_id = sock;
                strcpy(client[i].client_name, msg.user_name);
                strcpy(client[i].pass, msg.password);
                break;
            }
        }
    }
    else 
    {   
        res.status = FAILED;
        strcpy(res.mess,"Database error");
           
    }
    write(sock, &res, sizeof(res));
    fclose(file);
    pthread_mutex_unlock( &mutex1 );
}
void login(int sock, message msg){
  
    response res;
    bzero(&res, sizeof(res));
    printf("Client try to log in with username:%s  and password: %s \n", msg.user_name, msg.password);
    
    int isExits = 0;
    for(int i = 0;i < 50;i++){
       // printf("clien")
        if(!strcmp(msg.user_name,client[i].client_name) && !strcmp(msg.password,client[i].pass)){
            isExits = 1;
            if(client[i].isOnline == 0){
                client[i].isOnline = 1;
                client[i].sock_id = sock;
                res.status = LOGIN_SUCCESS;
                break;
            }else{
                res.status = LOGIN_FAIL;
                strcpy(res.mess,"Client is online on other devices");
            }
        }else{
            //printf("Compare %s %s %d %d",msg.user_name,client[i].client_name,strlen(msg.user_name),strlen(client[i].client_name));
            //printf(" and compare %s %s %d %d\n",msg.password,client[i].pass,strlen(msg.password),strlen(client[i].pass));
        }
    }

    if(isExits == 0){
        res.status = LOGIN_FAIL;
        strcpy(res.mess,"Wrong username and password");
    }
    write(sock, &res, sizeof(res));
    pthread_mutex_unlock( &mutex1 );
}
 
void who_is_online(int sock){
    printf("Who is online function\n");
    message msg;
    bzero(&msg, sizeof(msg));
    msg.cmd = CMD_LIST_PEOPLE;
    msg.nOnline = 0;
    for(int i = 0;i < 50; i++){
        if(client[i].isSet ==1 && client[i].isOnline == 1){
            strcpy(msg.people[msg.nOnline],client[i].client_name);
            msg.nOnline++;
        }
    }

    write(sock,&msg,sizeof(msg));
}
void send_to(message inmess){
    printf("send from server to %s, message = %s",inmess.to_user,inmess.mess);
    message msg;
    msg.cmd = CMD_SEND_FROM;
    strcpy(msg.mess,inmess.mess);
    strcpy(msg.user_name,inmess.user_name);
    for(int i = 0;i<50;i++){
        if(client[i].isSet == 1 && client[i].isOnline == 1 && !strcmp(client[i].client_name,inmess.to_user)){
            puts("Send message...");
            printf("Send to port...%d",client[i].sock_id);
            write(client[i].sock_id, &msg, sizeof(msg));
            break;
        }else{
            printf("Not send to %s \n",client[i].client_name);
        }
    }
}

void init(){
    for(int i = 0;i<50;i++){
        client[i].isSet = 0;
      //  client[i].client_name = "";
        client[i].sock_id = 0;
        client[i].isOnline = 0;
        //client[i].pass = "";
    }

    FILE*file = fopen("user.db","a+");
     if (file == NULL) {
         printf("I couldn't open db for\n");
         exit(0);
      }else{
         printf("Open file success\n");
      }
    char buffer[100];
    
    fseek(file, 0, SEEK_SET);
    while(fgets(buffer,100,file) != NULL){
            char client_name[50];
            char client_pass[50];
            char*tok;
            tok = strtok(buffer,"|\n");
           // strncpy(client_name, tok);
            strcpy(client_name, tok);
           // printf("tok 1 %s \n",tok);
            tok = strtok(NULL,"|\n");
               strcpy(client_pass, tok);
            //strncpy(client_pass, tok);
            for(int i = 0;i<50;i++){
                if(client[i].isSet == 0){
                    client[i].isSet = 1;
                    strcpy(client[i].pass,client_pass);
                    strcpy(client[i].client_name,client_name);
                    break;
                }
            }
        
    }
    fclose(file);
    printf("Load db sucess\n");

    // for(int i = 0;i< 50;i++){
    //     printf("Client %d name %s pass %s \n",i, client[i].client_name, client[i].pass);
    // }

}


int main(int argc , char *argv[])
{
    
    init();
    int socket_desc , new_socket , c , *new_sock;
    struct sockaddr_in server , client;
    char *mess;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
   // server.sin_port = htons( OPEN_PORT);
     
    int isOpen = 0;
    while(isOpen == 0){
        printf("Enter port to open: ");
        fflush(stdin);
        scanf("%d",&port_server);
        server.sin_port = htons( port_server);
        if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
        {
            puts("bind failed");
        }else{
            puts("bind done");
            isOpen = 1;
        }
    }
    //Bind

     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        //Reply to the client
        mess = "Hello Client , I have received your connection. And now I will assign a handler for you\n";
        write(new_socket , mess, strlen(mess));
         
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
     
    if (new_socket<0)
    {
        perror("accept failed");
        return 1;
    }
    free(&socket_desc);
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    int read_size;
    char *mess;
    message msg;
     
    //Send some messages to the client
    mess = "Greetings! I am your connection handler\n";
    write(sock , mess , strlen(mess));
     
    mess = "Now type something and i shall repeat what you type \n";
    write(sock , mess , strlen(mess));
     
    //Receive a message from client
    while( (read_size = recv(sock , &msg , sizeof(msg) , 0)) > 0 )
    {
        //Send the message back to client
        switch(msg.cmd){
            case CMD_LOGIN:
                login(sock,msg);
                bzero(&msg,sizeof(msg));
                break;
            case CMD_REGISTER:
                client_register(sock,msg);
                break;
            case CMD_SEND_TO:
                send_to(msg);
                break;
            case CMD_LIST_PEOPLE:
                who_is_online(sock);
                break;
            case CMD_EXIT:
                client_exit(sock,msg.user_name);
                //puts("Client disconnected");
                //fflush(stdout);
                //free(socket_desc);
                break;
        }
        //write(sock , client_message , strlen(client_message));
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