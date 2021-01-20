#include <arpa/inet.h>  //inet_addr
#include <fcntl.h>      //for open
#include <stdio.h>      //printf
#include <string.h>     //strlen
#include <sys/socket.h> //socket
#include <unistd.h>     //for close
#include "defines.h"

char victory(char *board)
{
    if ((board[0] == SYMBOL_X && board[3] == SYMBOL_X && board[6] == SYMBOL_X) ||
        (board[1] == SYMBOL_X && board[4] == SYMBOL_X && board[7] == SYMBOL_X) ||
        (board[2] == SYMBOL_X && board[5] == SYMBOL_X && board[8] == SYMBOL_X) ||
        (board[2] == SYMBOL_X && board[4] == SYMBOL_X && board[6] == SYMBOL_X) ||
        (board[0] == SYMBOL_X && board[4] == SYMBOL_X && board[8] == SYMBOL_X) ||
        (board[0] == SYMBOL_X && board[1] == SYMBOL_X && board[2] == SYMBOL_X) ||
        (board[3] == SYMBOL_X && board[4] == SYMBOL_X && board[5] == SYMBOL_X) ||
        (board[6] == SYMBOL_X && board[7] == SYMBOL_X && board[8] == SYMBOL_X))
        return SYMBOL_X;

    else if ((board[0] == SYMBOL_O && board[3] == SYMBOL_O && board[6] == SYMBOL_O) ||
             (board[1] == SYMBOL_O && board[4] == SYMBOL_O && board[7] == SYMBOL_O) ||
             (board[2] == SYMBOL_O && board[5] == SYMBOL_O && board[8] == SYMBOL_O) ||
             (board[2] == SYMBOL_O && board[4] == SYMBOL_O && board[6] == SYMBOL_O) ||
             (board[0] == SYMBOL_O && board[4] == SYMBOL_O && board[8] == SYMBOL_O) ||
             (board[0] == SYMBOL_O && board[1] == SYMBOL_O && board[2] == SYMBOL_O) ||
             (board[3] == SYMBOL_O && board[4] == SYMBOL_O && board[5] == SYMBOL_O) ||
             (board[6] == SYMBOL_O && board[7] == SYMBOL_O && board[8] == SYMBOL_O))
        return SYMBOL_O;
    else
        return 0;
}

char draw(char *board)
{
    int i;
    int brPopunjenihPolja = 0;

    for (i = 0; i < 8; i++)
    {
        if (board[i] != EMPTY)
            brPopunjenihPolja++;
    }
    if (brPopunjenihPolja == 9)
        return MESSAGE_TIE;
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
    char send_buffer[DEFAULT_LEN];
    char board[9] = {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY};
    unsigned char playerChoice;

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    // Set SOREUSEADDR option
    int option = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    //Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        //print the error message
        perror("Bind failed. Error");
        close(socket_desc);
        return 1;
    }
    puts("Bind done");

    //Listen
    listen(socket_desc, 2);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //Accept connection from an incoming client
    client1_sock = accept(socket_desc, (struct sockaddr *)&client1, (socklen_t *)&c);
    if (client1_sock < 0)
    {
        perror("Accept failed");
        close(socket_desc);
        return 1;
    }
    puts("Connection accepted");

    //Accept connection from an incoming client
    client2_sock = accept(socket_desc, (struct sockaddr *)&client2, (socklen_t *)&c);
    if (client2_sock < 0)
    {
        perror("Accept failed");
        close(socket_desc);
        return 1;
    }
    puts("Connection accepted");

    // Send start signals

    send_buffer[0] = MESSAGE_START;
    send_buffer[1] = SYMBOL_X;

    if (send(client1_sock, send_buffer, START_LEN, 0) < 0)
    {
        puts("Send failed");
        close(socket_desc);
        return 1;
    }

    send_buffer[0] = MESSAGE_START;
    send_buffer[1] = SYMBOL_O;

    if (send(client2_sock, send_buffer, START_LEN, 0) < 0)
    {
        puts("Send failed");
        close(socket_desc);
        return 1;
    }

    // Game loop

    while (1)
    {

        if (recv(client1_sock, &playerChoice, 1, 0) != 1)
        {
            fprintf(stderr, "Invalid message length from client\n");
            close(socket_desc);
            return 1;
        }

        board[playerChoice - 1] = SYMBOL_X;

        if (victory(board))
        {
            char playerXState;
            char playerOState;
            for (int i = 0; i < 9; i++)
            {
                send_buffer[i + 1] = board[i];
            }
            if (victory(board) == SYMBOL_X)
            {
                playerXState = MESSAGE_WIN;
                playerOState = MESSAGE_LOSE;
            }
            else
            {
                playerXState = MESSAGE_LOSE;
                playerOState = MESSAGE_WIN;
            }
            send_buffer[0] = playerXState;
            if (send(client1_sock, send_buffer, DEFAULT_LEN, 0) < 0)
            {
                puts("Send failed");
                close(socket_desc);
                return 1;
            }
            send_buffer[0] = playerOState;
            if (send(client2_sock, send_buffer, DEFAULT_LEN, 0) < 0)
            {
                puts("Send failed");
                close(socket_desc);
                return 1;
            }
            break;
        }
        else if (draw(board))
        {
            for (int i = 0; i < 9; i++)
            {
                send_buffer[i + 1] = board[i];
            }

            send_buffer[0] = MESSAGE_TIE;

            if (send(client1_sock, send_buffer, DEFAULT_LEN, 0) < 0)
            {
                puts("Send failed");
                close(socket_desc);
                return 1;
            }
            if (send(client2_sock, send_buffer, DEFAULT_LEN, 0) < 0)
            {
                puts("Send failed");
                close(socket_desc);
                return 1;
            }
            break;
        }
        else
            send_buffer[0] = MESSAGE_PLAY;

        for (int i = 0; i < 9; i++)
        {
            send_buffer[i + 1] = board[i];
        }

        if (send(client2_sock, send_buffer, DEFAULT_LEN, 0) < 0)
        {
            puts("Send failed");
            close(socket_desc);
            return 1;
        }

        if (recv(client2_sock, &playerChoice, 1, 0) != 1)
        {
            fprintf(stderr, "Invalid message length from client\n");
            close(socket_desc);
            return 1;
        }

        board[playerChoice - 1] = SYMBOL_O;

        if (victory(board))
        {
            char playerXState;
            char playerOState;
            for (int i = 0; i < 9; i++)
            {
                send_buffer[i + 1] = board[i];
            }
            if (victory(board) == SYMBOL_O)
            {
                playerOState = MESSAGE_WIN;
                playerXState = MESSAGE_LOSE;
            }
            else
            {
                playerOState = MESSAGE_LOSE;
                playerXState = MESSAGE_WIN;
            }
            send_buffer[0] = playerXState;
            if (send(client1_sock, send_buffer, DEFAULT_LEN, 0) < 0)
            {
                puts("Send failed");
                close(socket_desc);
                return 1;
            }
            send_buffer[0] = playerOState;
            if (send(client2_sock, send_buffer, DEFAULT_LEN, 0) < 0)
            {
                puts("Send failed");
                close(socket_desc);
                return 1;
            }
            break;
        }
        else if (draw(board))
        {
            for (int i = 0; i < 9; i++)
            {
                send_buffer[i + 1] = board[i];
            }

            send_buffer[0] = MESSAGE_TIE;

            if (send(client1_sock, send_buffer, DEFAULT_LEN, 0) < 0)
            {
                puts("Send failed");
                close(socket_desc);
                return 1;
            }
            if (send(client2_sock, send_buffer, DEFAULT_LEN, 0) < 0)
            {
                puts("Send failed");
                close(socket_desc);
                return 1;
            }
            break;
        }
        else
            send_buffer[0] = MESSAGE_PLAY;

        for (int i = 0; i < 9; i++)
        {
            send_buffer[i + 1] = board[i];
        }

        if (send(client1_sock, send_buffer, DEFAULT_LEN, 0) < 0)
        {
            puts("Send failed");
            close(socket_desc);
            return 1;
        }
    }

    close(socket_desc);
    return 0;
}