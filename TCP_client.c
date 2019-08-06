
#include<stdio.h>  
#include<stdlib.h>  
#include<netinet/in.h>  
#include<sys/socket.h>  
#include<arpa/inet.h>  
#include<string.h>  
#include<unistd.h>  
#define BUFFER_SIZE 1024  
struct sockaddr_in server_addr;  
fd_set client_fd_set;   
/*
func:init_client_socket()
ok return socket_fd
else return 0 
*/ 
int init_client_socket(char *ip){
	server_addr.sin_family = AF_INET;  
    server_addr.sin_port = htons(11332);  
    server_addr.sin_addr.s_addr = inet_addr(ip);  
    bzero(&(server_addr.sin_zero), 8); 
    int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);  
    if(server_sock_fd == -1)  
    {  
    perror("socket error");  
    return 0;  
    } 
	return server_sock_fd; 
}
void fd_init(int c_fd){
        FD_ZERO(&client_fd_set);  
        FD_SET(STDIN_FILENO, &client_fd_set);  
        FD_SET(c_fd, &client_fd_set);  	
}
int send_to_server(int fd){
	        char input_msg[BUFFER_SIZE]; 
	        bzero(input_msg, BUFFER_SIZE);  
            fgets(input_msg, BUFFER_SIZE, stdin);  
            if(send(fd, input_msg, BUFFER_SIZE, 0) == -1)  
            {  
                perror("send msg wrong!\n");  
            } 
			return 1; 
}
int recv_msg_from_server(int fd){
	        char recv_msg[BUFFER_SIZE]; 
			bzero(recv_msg, BUFFER_SIZE);  
            long byte_num = recv(fd, recv_msg, BUFFER_SIZE, 0);  
            if(byte_num > 0)  
            {  
            if(byte_num > BUFFER_SIZE)  
            {  
                byte_num = BUFFER_SIZE;  
            }  
            recv_msg[byte_num] = '\0';  
            printf("server msg:%s\n", recv_msg);  
            }  
            else if(byte_num < 0)  
            {  
            printf("recv msg wrong!\n");  
            }  
            else  
            {  
            printf("server is out!\n");  
            exit(0);  
} }
int main()  
{  
    char * ipaddr = "127.0.0.1";
    int server_sock_fd = init_client_socket(ipaddr);
    char recv_msg[BUFFER_SIZE];  
    char input_msg[BUFFER_SIZE];  
    if(connect(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == 0)  
    {  
    while(1)  
    {   
	   fd_init(server_sock_fd);
       select(server_sock_fd + 1, &client_fd_set, NULL, NULL,NULL);  
       if(FD_ISSET(STDIN_FILENO, &client_fd_set))  
        {  
         send_to_server(server_sock_fd); 
        }  
        if(FD_ISSET(server_sock_fd, &client_fd_set))  
        {  
              recv_msg_from_server(server_sock_fd); 
        }  
    }  
   }
    return 0;  
}
