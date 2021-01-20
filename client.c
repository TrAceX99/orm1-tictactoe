#include <arpa/inet.h>  //inet_addr
#include <fcntl.h>      //for open
#include <stdio.h>      //printf
#include <string.h>     //strlen
#include <sys/socket.h> //socket
#include <unistd.h>     //for close
#include <signal.h>
#include <stdlib.h>
#include "defines.h"

// Client socket
int sock;

void print_board(char* board, char player) {
    // Clear screen
    printf("\e[1;1H\e[2J");

    printf("== You are: %c ==\n\n", player);
    printf("\t%c|%c|%c\n", board[6], board[7], board[8]);
    printf("\t-+-+-\n");
    printf("\t%c|%c|%c\n", board[3], board[4], board[5]);
    printf("\t-+-+-\n");
    printf("\t%c|%c|%c\n", board[0], board[1], board[2]);
}

void interruptHandler(int a) {
    printf(" Exiting...\n");
    close(sock);
    exit(0);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server;
    char recv_buffer[DEFAULT_LEN];
    char playerSymbol;
    unsigned char playerChoice;
    char board[9] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };

    // Catch SIGINT
    signal(SIGINT, interruptHandler);

    //Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        fprintf(stderr, "Could not create socket\n");
        return 1;
    }

    // Get IP address from user
    char addrBuffer[ADDR_STRING_MAX_LEN];
    printf("Enter the server IP address: ");
    fgets(addrBuffer, ADDR_STRING_MAX_LEN, stdin);
    addrBuffer[strlen(addrBuffer) - 1] = 0;

    if (inet_aton(addrBuffer, (struct in_addr *)&server.sin_addr.s_addr) == 0) {
        printf("Invalid IP address! Defaulting to 127.0.0.1\n");
        server.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);

    //Connect to remote server
    printf("Connecting to remote server...\n");
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed. Error");
        close(sock);
        return 1;
    }
    printf("Connected\n");

    printf("Waiting for the other player...\n");

    // Receive the start signal
    if (recv(sock, recv_buffer, START_LEN, 0) != START_LEN || recv_buffer[0] != 'S' || 
    (recv_buffer[1] != 'X' && recv_buffer[1] != 'O')) {
        fprintf(stderr, "Invalid start signal from server\n");
        close(sock);
        return 1;
    }
    playerSymbol = recv_buffer[1];

    if (playerSymbol == SYMBOL_X) {
        recv_buffer[0] = MESSAGE_PLAY;
    } else {
        print_board(board, playerSymbol);
        printf("Waiting for player X ...\n");

        if (recv(sock, recv_buffer, DEFAULT_LEN, 0) != DEFAULT_LEN) {
            fprintf(stderr, "Invalid message length from server\n");
            close(sock);
            return 1;
        }
        for (int i = 0; i < 9; i++) {
            board[i] = recv_buffer[i + 1];
        }
    }

    // Game loop
    while (recv_buffer[0] != 'W' && recv_buffer[0] != 'L' && recv_buffer[0] != 'T') {
        if (recv_buffer[0] != 'P') {
            fprintf(stderr, "Invalid message header from server\n");
            close(sock);
            return 1;
        }

        print_board(board, playerSymbol);
        printf("Your turn: \n");

        // Get player choice
        while (playerChoice == 0 || playerChoice > 9) {
            scanf("%hhu\n", &playerChoice);
        }
        if (send(sock, &playerChoice, 1, 0) < 0) {
            fprintf(stderr, "Send failed\n");
            close(sock);
            return 1;
        }

        printf("Waiting for player %c ...\n", (playerSymbol == SYMBOL_X) ? SYMBOL_O : SYMBOL_X);

        if (recv(sock, recv_buffer, DEFAULT_LEN, 0) != DEFAULT_LEN) {
            fprintf(stderr, "Invalid message length from server\n");
            close(sock);
            return 1;
        }
        for (int i = 0; i < 9; i++) {
            board[i] = recv_buffer[i + 1];
        }
    }

    print_board(board, playerSymbol);
    switch (recv_buffer[0]) {
        case MESSAGE_WIN:
            printf("You won!\n");
            break;
        case MESSAGE_LOSE:
            printf("You lost!\n");
            break;
        case MESSAGE_TIE:
            printf("It's a tie!\n");
            break;
        default:
            break;
    }

    close(sock);
    return 0;
}