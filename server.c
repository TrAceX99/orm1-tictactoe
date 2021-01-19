#include <arpa/inet.h>  //inet_addr
#include <fcntl.h>      //for open
#include <stdio.h>      //printf
#include <string.h>     //strlen
#include <sys/socket.h> //socket
#include <unistd.h>     //for close
#include "defines.h"

char victory(char *board)
{
    if( (board[0] == SYMBOL_X && board[3] == SYMBOL_X && board[6] == SYMBOL_X) || 
        (board[1] == SYMBOL_X && board[4] == SYMBOL_X && board[7] == SYMBOL_X) ||
        (board[2] == SYMBOL_X && board[5] == SYMBOL_X && board[8] == SYMBOL_X) ||
        (board[2] == SYMBOL_X && board[4] == SYMBOL_X && board[6] == SYMBOL_X) ||
        (board[0] == SYMBOL_X && board[4] == SYMBOL_X && board[8] == SYMBOL_X) ||
        (board[0] == SYMBOL_X && board[1] == SYMBOL_X && board[2] == SYMBOL_X) ||
        (board[3] == SYMBOL_X && board[4] == SYMBOL_X && board[5] == SYMBOL_X) ||
        (board[6] == SYMBOL_X && board[7] == SYMBOL_X && board[8] == SYMBOL_X)
    )
        return SYMBOL_X;

    else if( (board[0] == SYMBOL_O && board[3] == SYMBOL_O && board[6] == SYMBOL_O) || 
             (board[1] == SYMBOL_O && board[4] == SYMBOL_O && board[7] == SYMBOL_O) ||
             (board[2] == SYMBOL_O && board[5] == SYMBOL_O && board[8] == SYMBOL_O) ||
             (board[2] == SYMBOL_O && board[4] == SYMBOL_O && board[6] == SYMBOL_O) ||
             (board[0] == SYMBOL_O && board[4] == SYMBOL_O && board[8] == SYMBOL_O) ||
             (board[0] == SYMBOL_O && board[1] == SYMBOL_O && board[2] == SYMBOL_O) ||
             (board[3] == SYMBOL_O && board[4] == SYMBOL_O && board[5] == SYMBOL_O) ||
             (board[6] == SYMBOL_O && board[7] == SYMBOL_O && board[8] == SYMBOL_O)
           )
        return SYMBOL_O;
    else
        return 0;
}

char draw(char *board)
{  
	int i;
    int brPopunjenihPolja = 0;

    for(i = 0; i < 8; i++)
    {
        if(board[i] != ' ')
            brPopunjenihPolja++;
    }
    if(brPopunjenihPolja == 9)
        return 'T';
    else
        return 0;
}



int main(int argc, char *argv[])
{
    int socket_desc;
    int client1_sock;
    int client2_sock;
    int c;
    struct sockaddr_in server;
    struct sockaddr_in client1;
    struct sockaddr_in client2;
    char send_buffer1[DEFAULT_LEN];
    char send_buffer2[DEFAULT_LEN];
    char board[9] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
    unsigned char playerChoice;


    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    //Bind
    if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        //print the error message
        perror("Bind failed. Error");
        return 1;
    }
    puts("Bind done");

    //Listen
    listen(socket_desc, 2);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    // klijent1:

    //Accept connection from an incoming client
    client1_sock = accept(socket_desc, (struct sockaddr *)&client1, (socklen_t*)&c);
    if (client1_sock < 0)
    {
        perror("Accept failed");
        return 1;
    }
    puts("Connection accepted");

    send_buffer1[0] = 'S';
    send_buffer1[1] = 'X';


    if(send(socket_desc, send_buffer1, START_LEN, 0) < 0)
    {
        puts("Send failed");
        return 1;
    }


    if(recv(socket_desc, &playerChoice, 1, 0) != 1)
    {
        fprintf(stderr, "Invalid message length from client\n");
        close(socket_desc);
        return 1;
    }

    board[playerChoice] = SYMBOL_X;


    if(victory(board))
        send_buffer1[0] = victory(board);
    else if(draw(board))
        send_buffer1[0] = draw(board);
    else
        send_buffer1[0] = 'P';

    
    for (int i = 0; i < 9; i++) {
        send_buffer1[i + 1] = board[i];
	}
	
    if(send(socket_desc, send_buffer1, DEFAULT_LEN, 0) < 0)
    {
        puts("Send failed");
        return 1;
    }


    // klijent2:

    //Accept connection from an incoming client
    client2_sock = accept(socket_desc, (struct sockaddr *)&client2, (socklen_t*)&c);
    if (client2_sock < 0)
    {
        perror("Accept failed");
        return 1;
    }
    puts("Connection accepted"); 

    send_buffer2[0] = 'P';
    send_buffer2[1] = 'O';
   
    if(send(socket_desc, send_buffer2, START_LEN, 0) < 0)
    {
        puts("Send failed");
        return 1;
    }


    send_buffer2[0] = 'P';

    for (int i = 0; i < 9; i++) {
        send_buffer2[i + 1] = board[i];
	}
	
    if(send(socket_desc, send_buffer2, DEFAULT_LEN, 0) < 0)
    {
        puts("Send failed");
        return 1;
    }


    if(recv(socket_desc, &playerChoice, 1, 0) != 1)
    {
        fprintf(stderr, "Invalid message length from client\n");
        close(socket_desc);
        return 1;
    }

    board[playerChoice] = SYMBOL_O;


    if(victory(board))
        send_buffer1[0] = victory(board);
    else if(draw(board))
        send_buffer1[0] = draw(board);
    else
        send_buffer1[0] = 'P';

    
    for (int i = 0; i < 9; i++) {
        send_buffer2[i + 1] = board[i];
	}
	
    if(send(socket_desc, send_buffer2, DEFAULT_LEN, 0) < 0)
    {
        puts("Send failed");
        return 1;
    }


    close(socket_desc);
    return 0;

}

