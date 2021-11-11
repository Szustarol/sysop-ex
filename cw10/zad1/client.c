//
// Created by karolszustakowski on 24.05.2021.
//
#include "defines.h"

struct sockaddr_un localSocket;
struct sockaddr_in webSocket;
int conFd;

char recvbuf[BUF_SIZE];

int playing = 0;

typedef enum{
    CIRCLE_WINS,
    CROSS_WINS,
    NONE_WINS,
    KEEP_PLAYING
}victoryType;

typedef enum{
    WEB,
    LOCAL
}connectionType;

void sigintHandler(int signo){
    exit(0);
}

void sendGameOver(void){
    if(playing){
        write(conFd, "DC", strlen("DC")+1);
    }
}

void drawTable(const char * table){
    char tableContent[500];
    sprintf(tableContent, "(1)%c|(2)%c|(3)%c\n--------------\n(4)%c|(5)%c|(6)%c\n--------------\n(7)%c|(8)%c|(9)%c\n",
            table[0], table[1], table[2], table[3], table[4], table[5], table[6], table[7], table[8]);
    printf("%s", tableContent);
}

victoryType checkVictory(const char * table){
    victoryType  result = KEEP_PLAYING;
    for(int i = 0; i < 3; i++){
        if (table[i*3] != ' ' && table[i*3] == table[i*3+1] && table[i*3+1] == table[i*3+2]){
            if(table[i*3] == 'O')
                result = CIRCLE_WINS;
            else
                result = CROSS_WINS;
            break;
        }
        if(table[i] != ' ' && table[i] == table[i+3] && table[i+3] == table[i+6]){
            if(table[i] == 'O')
                result = CIRCLE_WINS;
            else
                result = CROSS_WINS;
            break;
        }
    }

    if(table[0] == table[4] && table[4] == table[8]){
        if(table[0] == 'O')
            result = CIRCLE_WINS;
        else if (table[0] == 'X')
            result = CROSS_WINS;
    }
    if(table[2] == table[4] && table[4] == table[6]){
        if(table[4] == 'O')
            result = CIRCLE_WINS;
        else if (table[0] == 'X')
            result = CROSS_WINS;
    }

    if(result == KEEP_PLAYING){
        result = NONE_WINS;
        for(int i = 0; i < 9; i++){
            if(table[i] == ' '){
                result = KEEP_PLAYING;
                break;
            }
        }
    }

    return result;
}

void pickPlace(char * table, char myChar){
    while(1) {
        int position;
        puts("Pick position to put your mark in: (1-9)");
        scanf("%d", &position);
        if (position < 1 || position > 9) {
            puts("Invalid position selected");
            continue;
        }
        if (table[position - 1] != ' ') {
            puts("Position already taken");
            continue;
        }
        table[position - 1] = myChar;
        break;
    }
}

int parseVictory(char * table){
    victoryType vt = checkVictory(table);
    if(vt == KEEP_PLAYING){
        return 0;
    }
    else if(vt == CIRCLE_WINS){
        puts("Circle wins!");
    }
    else if(vt == CROSS_WINS){
        puts("Circle wins!");
    }
    else if(vt == NONE_WINS){
        puts("A draw!");
    }
    return 1;
}

void runClient(char * myName){//conFd is set up at this time
    write(conFd, myName, strlen(myName)+1);
    int nread = read(conFd, recvbuf, BUF_SIZE);
    recvbuf[nread] = 0;

    if(strcmp(recvbuf, "TAKEN") == 0){
        puts("Name already taken");
        return;
    }
    puts("Connected to the server.");

    while(1){
        nread = read(conFd, recvbuf, BUF_SIZE);
        if(nread < 1){
            puts("Server closed the connection");
            return;
        }
        recvbuf[nread] = 0;
        if(strcmp(recvbuf, "PING") == 0){
            write(conFd, "PONG", strlen("PONG")+1);
        }
        else{
            playing = 1;
            char table[9] = "         ";
            char myChar = recvbuf[0];
            int skipFirst = myChar == 'O' ? 0 : 1;
            puts(recvbuf);
            while(1){
                //get move from this player
                if(!skipFirst){
                    puts("\n\nThe other player made a move:\n");
                    drawTable(table);
                    int victorious = parseVictory(table);
                    if(victorious){
                        write(conFd, "GAMEOVER", strlen("GAMEOVER")+1);
                        playing = 0;
                        break;
                    }
                    pickPlace(table, myChar);
                    puts("Sending the move to another player...");
                    write(conFd, table, 9);
                    drawTable(table);
                    victorious = parseVictory(table);
                    if(victorious){
                        write(conFd, "GAMEOVER", strlen("GAMEOVER")+1);
                        playing = 0;
                        break;
                    }
                }
                else{
                    skipFirst = 0;
                    puts("The other player starts. Wait for him to make a move");
                }
                puts("Waiting for the other player's response.");
                nread = read(conFd, recvbuf, BUF_SIZE);
                if(nread < 1){
                    puts("Server closed the connection");
                    return;
                }
                if(strcmp(recvbuf, "DC") == 0){
                    puts("The other player quit the game...");
                    playing = 0;
                    break;
                }
                strncpy(table, recvbuf, 9);
            }
        }
    }
}

int main(int argc, char ** argv){
    signal(SIGPIPE, SIG_IGN);
    if(argc < 4){
        puts("Invalid number of arguments");
    }
    char * myName = argv[1];
    if(strlen(myName) > 30){
        puts("Your name is too long");
        return 0;
    }

    connectionType ctype;

    if(strcmp(argv[2], "WEB") == 0){
        ctype = WEB;
    }
    else if(strcmp(argv[2], "LOCAL") == 0){
        ctype = LOCAL;
    }
    else{
        puts("Invalid connection type selected (WEB|LOCAL)");
        return 0;
    }

    char * addr = argv[3];
    char * port = NULL;

    if(ctype == WEB){
        for(int i = 0; i < strlen(argv[3]); i++){
            if(addr[i] == ':'){
                addr[i] = 0x0;
                port = addr+i+1;
            }
        }
        if(port == NULL){
            puts("Port number not specified");
            return 0;
        }
    }



    if(ctype == WEB){
        conFd = socket(AF_INET, SOCK_STREAM, 0);
        if(conFd == -1) {
            puts("Unable to create web socket");
            return 0;
        }
        struct in_addr inAddr;
        if(inet_pton(AF_INET, addr, &inAddr) == 0){
            puts("Invalid network address");
            return 0;
        }
        short portnum;
        if((portnum = atoi(port)) == 0){
            puts("Invalid port number");
            return 0;
        }

        webSocket.sin_port = htons(portnum);
        webSocket.sin_addr = inAddr;
        webSocket.sin_family = AF_INET;

        if(connect(conFd, (struct sockaddr*)&webSocket, sizeof(webSocket)) == -1){
            puts("Unable to connect via WEB");
            return 0;
        }
    }
    else{
        conFd = socket(AF_LOCAL, SOCK_STREAM, 0);
        if(conFd == -1){
            puts("Unable to create local socket");
            return 0;
        }

        localSocket.sun_family = AF_LOCAL;
        strcpy(localSocket.sun_path, addr);
        if(connect(conFd, (struct sockaddr*)&localSocket, sizeof(localSocket)) == -1){
            puts("Unable to connect via LOCAL");
            return 0;
        }
    }
    atexit(sendGameOver);
    signal(SIGINT, sigintHandler);

    runClient(myName);


    return 0;
}