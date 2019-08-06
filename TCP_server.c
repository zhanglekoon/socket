#include<stdio.h>  
#include<stdlib.h>  
#include<netinet/in.h>  
#include<sys/socket.h>  
#include<arpa/inet.h>  
#include<string.h>  
#include<unistd.h>  
#define BACKLOG 5     //完成三次握手但没有accept的队列的长度  
#define CONCURRENT_MAX 8   //the ue limitation
#define SERVER_PORT 11332  
#define BUFFER_SIZE 1024  
int client_fds[CONCURRENT_MAX]; 
struct timeval tv;  
struct sockaddr_in server_addr; 
fd_set server_fd_set;  
int max_fd = -1;  
/*
fuction: init_server socket
return  file description
*/
int  init_server_socket(char* ipaddr){
   	server_addr.sin_family = AF_INET;  
    server_addr.sin_port = htons(SERVER_PORT);  
    server_addr.sin_addr.s_addr = inet_addr(ipaddr);  
    bzero(&(server_addr.sin_zero), 8);  
    int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);  
    if(server_sock_fd == -1)  
    {  
        perror("socket error");  
        return -1;  
    }  
    int bind_result = bind(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));  
    if(bind_result == -1)  
    {  
        perror("bind error");  
        return -1;  
    }   
    if(listen(server_sock_fd, BACKLOG) == -1)  
    {  
        perror("listen error");  
        return -1;  
    }  
    return server_sock_fd;
}
void set_fd(int s_fd){
	    FD_ZERO(&server_fd_set);  
        FD_SET(STDIN_FILENO, &server_fd_set);   
        if(max_fd <STDIN_FILENO)  
        {  
            max_fd = STDIN_FILENO;  
        }  
        FD_SET(s_fd, &server_fd_set);  
        if(max_fd < s_fd)  
        {  
            max_fd = s_fd;  
        }  
        //client conn
      int i = 0; 
        for(i =0; i < CONCURRENT_MAX; i++)  
        {   
            if(client_fds[i] != 0)  
            {  
                FD_SET(client_fds[i], &server_fd_set);  
                if(max_fd < client_fds[i])  
                {  
                    max_fd = client_fds[i];  
                }  
            }  
        } 
}
/* 
function:send message to fd 
@param: send message str 
ok return 1
fail  return 0 
*/ 
int send_to_client()
            { 
                 char input_msg[BUFFER_SIZE]; 
                printf("send message：\n");  
                bzero(input_msg, BUFFER_SIZE);  
                fgets(input_msg, BUFFER_SIZE, stdin);
				int i =0;  
                for(i = 0; i < CONCURRENT_MAX; i++)  
                {  
                    if(client_fds[i] != 0)  
                    {  
                        printf("client_fds[%d]=%d\n", i, client_fds[i]);  
                       int len = send(client_fds[i], input_msg, BUFFER_SIZE, 0); 
						 if(len<=0)
						 {
						 	printf("send to UE(%d) failed",i);
						 	return 0;
						 }
						  
                    }  
                }
			return 1; } 
/*
function:add new ue and record the fd
ok return 1
else return 0 
*/
int new_ue_add(int s_fd)
{
                struct sockaddr_in client_address;  
                socklen_t address_len;  
                int client_sock_fd = accept(s_fd, (struct sockaddr *)&client_address, &address_len);  
                printf("new connection client_sock_fd = %d\n", client_sock_fd);  
                if(client_sock_fd > 0)  
                {  
                    int index = -1; 
					int i = 0; 
                    for(i = 0; i < CONCURRENT_MAX; i++)  
                    {  
                        if(client_fds[i] == 0)  
                        {  
                            index = i;  
                            client_fds[i] = client_sock_fd;  
                            break;  
                        }  
                    }  
                    if(index >= 0)  
                    {  
                        printf("new UE(%d)joined successfully %s:%d\n", index, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));  
                        return 1;
					}  
                    else  
                    {   char input_msg[BUFFER_SIZE]; 
                        bzero(input_msg, BUFFER_SIZE);  
                        strcpy(input_msg, "the UE numbers are max!\n");  
                        send(client_sock_fd, input_msg, BUFFER_SIZE, 0);  
                        printf("UE max,jion failed %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));  
                        return 0;
                    }  
                } 
} 
/*
function:if ue send msg to server, store and print it
we can distinguish the ue from fd  

*/
void recv_from_ue(){
			int i =0;
		   char recv_msg[BUFFER_SIZE];  
            for(i =0; i < CONCURRENT_MAX; i++)  
            {  
                if(client_fds[i] !=0)  
                {  
                    if(FD_ISSET(client_fds[i], &server_fd_set))  
                    {   
                        bzero(recv_msg, BUFFER_SIZE);  
                        long byte_num = recv(client_fds[i], recv_msg, BUFFER_SIZE, 0);  
                        if (byte_num > 0)  
                        {  
                            if(byte_num > BUFFER_SIZE)  
                            {  
                                byte_num = BUFFER_SIZE;  
                            }  
                            recv_msg[byte_num] = '\0';  
                            printf("recv from UE(%d):%s\n", i, recv_msg);  
                        }  
                        else if(byte_num < 0)  
                        {  
                            printf("recv UE(%d) message wrong.\n", i);  
                        }  
                        else  
                        {  
                            FD_CLR(client_fds[i], &server_fd_set);  
                            client_fds[i] = 0;  
                            printf("the UE(%d) out\n", i);  
                        }  
                    }  
                }  
            } 
}
int main()  
{    //tv.tv_sec = 20;  
     //tv.tv_usec = 0; 
     char input_msg[BUFFER_SIZE];  
     char recv_msg[BUFFER_SIZE];  
     char *ipadd = "127.0.0.1";
     int server_sock_fd = init_server_socket(ipadd); 
    while(1)  
    {   int i = 0; 
        set_fd(server_sock_fd);
        //pause and wait signal   only respond with signal  
        int ret = select(max_fd + 1, &server_fd_set, NULL, NULL, NULL);  
       //deal with the select return value 
	    if(ret < 0)   
        {  
            perror("select wrong\n");  
            continue;  
        }  
        else  
        {  
            if(FD_ISSET(STDIN_FILENO, &server_fd_set))  
            {  
              send_to_client();
            }  
            if(FD_ISSET(server_sock_fd, &server_fd_set))  
            {  
             new_ue_add(server_sock_fd);
            } 
            recv_from_ue();
        }  
    }  
    return 0;  
}

