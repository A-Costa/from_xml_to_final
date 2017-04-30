#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "file_handling.h"


POSITION nextTag(int fd, POSITION pos){
    int lfd = dup(fd);
    char c = '\0';
    if(lseek(lfd, pos, SEEK_SET) == -1){
        perror("lseek_jumpspaces");
        exit(1);
    }
    if(read(lfd, &c, 1) == -1){
        perror("read_nextTag");
        exit(1);
    }
    if(c != '<'){
        printf("nextTag: not on a tag!\n");
        return -1;
    }
    do{
        pos +=1;
        if(read(lfd, &c, 1) == -1){
            perror("read_nextTag");
            exit(1);
        }
    }while(c != '<');
    close(lfd);
    return pos;
}

enum tags readTag(int fd, POSITION pos){
    char c;
    int lfd = dup(fd);
    if(lseek(lfd, pos, SEEK_SET) == -1){
        perror("lseek_readtag");
        exit(1);
    }
    if(read(lfd, &c, 1) == -1){
        perror("read_readtag");
        exit(1);
    }
    if(c != '<'){
        printf("readTag: not on a tag!\n");
        return ERROR;
    }
    if(read(lfd, &c, 1) == -1){
        perror("read_readtag");
        exit(1);
    }
    switch(c){
        case 'a':
            close(lfd);
            return ADDRESS;
        case 'b':
            close(lfd);
            return BLOCK;
        case 't':
            close(lfd);
            return TX;
        case 'i':
            if(lseek(lfd, 4, SEEK_CUR) == -1){
                perror("lseek_readtag");
                exit(1);
            }
            if(read(lfd, &c, 1) == -1){
                perror("read_readtag");
                exit(1);
            }
            switch(c){
                case '>':
                    if(lseek(lfd, -2, SEEK_CUR) == -1){
                        perror("lseek_readtag");
                        exit(1);
                    }
                    if(read(lfd, &c, 1) == -1){
                        perror("read_readtag");
                        exit(1);
                    }
                    switch(c){
                        case 't':
                            close(lfd);
                            return INPUT;
                        case 'x':
                            close(lfd);
                            return INDEX;
                        }
                case 's':
                    close(lfd);
                    return INPUTS;
                case '_':
                    close(lfd);
                    return INTXHASH;
                default:
                    close(lfd);
                    return ERROR;
            }
        case 'o':
            if(lseek(lfd, 5, SEEK_CUR) == -1){
                perror("lseek_readtag");
                exit(1);
            }
            if(read(lfd, &c, 1) == -1){
                perror("read_readtag");
                exit(1);
            }
            switch(c){
                case '>':
                    close(lfd);
                    return OUTPUT;
                case 's':
                    close(lfd);
                    return OUTPUTS;
                default:
                    close(lfd);
                    return ERROR;
            }
        case '/':
            close(lfd);
            return CLOSING;
    }
    close(lfd);
    return ERROR;
}

void getTxHash(int fd, POSITION pos, char* hash){
    int i;
    int lfd = dup(fd);
    if(lseek(lfd, pos+10, SEEK_SET) == -1){
        perror("lseek_printtxhash");
        exit(1);
    }
    if(read(lfd, hash, 64) == -1){
        perror("read_printtxhash");
        exit(1);
    }
    close(lfd);
}

unsigned long long nTxOuts(int fd, POSITION pos){
    unsigned long long result = 0;
    enum tags t;

    int lfd = dup(fd);
    pos = nextTag(lfd, pos);
    t = readTag(lfd, pos);

    do{
        if(t == OUTPUT){
            result +=1;
        }
        pos = nextTag(lfd, pos);
        t = readTag(lfd, pos);
    }while(t != TX); // chiaramente questo causa un problema: si puo lavorare fino alla penultima tx

    close(lfd);
    return result;
}

unsigned long long nTxInputs(int fd, POSITION pos){
    unsigned long long result = 0;
    enum tags t;

    int lfd = dup(fd);
    pos = nextTag(lfd, pos);
    t = readTag(lfd, pos);

    do{
        if(t == INPUT){
            result +=1;
        }
        pos = nextTag(lfd, pos);
        t = readTag(lfd, pos);
    }while(t != TX); // chiaramente questo causa un problema: si puo lavorare fino alla penultima tx

    close(lfd);
    return result;
}

output* arrayOfTxOuts(int fd, POSITION pos){
    unsigned long long n = nTxOuts(fd, pos);
    unsigned long long i = 0;
    unsigned long j = 0;
    enum tags t;

    output *array;
    array = malloc(n * sizeof(output));

    int lfd = dup(fd);
    pos = nextTag(lfd, pos);
    t = readTag(lfd, pos);

    char c;

    do{
        if(t == OUTPUT){
            pos = nextTag(lfd, pos);
            pos = nextTag(lfd, pos);
            pos = nextTag(lfd, pos);
            if(lseek(lfd, pos+9, SEEK_SET) == -1){
                perror("lseek_arrayoftxouts");
                exit(1);
            }
            read(lfd, &c, 1);
            while(c != '<'){
                (array[i]).address[j] = c;
                j++;
                read(lfd, &c, 1);
            }
            array[i].address_length = j;
            i++;
            j=0;
        }
        pos = nextTag(lfd, pos);
        t = readTag(lfd, pos);
    }while(t != TX);

    close(lfd);
    return array;
}

unsigned long getOutIndex(int fd, POSITION pos){
    int lfd = dup(fd);
    char buffer[11];
    char c;
    int i;
    if(lseek(lfd, pos+7, SEEK_SET) == -1){
        perror("lseek_getoutindex");
        exit(1);
    }
    i=0;
    while(i<10){
        read(lfd, &c, 1);
        if(c!='<'){
            buffer[i] = c;
            i++;
        }
        else{
            buffer[i] = '\0';
            break;
        }
    }

    close(lfd);
    char *end;
    return strtoul(buffer, &end, 10);
}


void getInHash(int fd, POSITION pos, char *hash){
    int lfd = dup(fd);
    if(lseek(lfd, pos+12, SEEK_SET) == -1){
        perror("lseek_gethash");
        exit(1);
    }
    if(read(lfd, hash, 64) == -1){
        perror("read_gethash");
        exit(1);
    }
    close(lfd);
}
