#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "file_handling.h"

int main(){
    int xml_file;
    int i,j;
    const char *file_name = "file.xml";
    POSITION pos = 0;
    char buffer[8];
    char hash[64];

    unsigned long long outs;

    tx_outputs txo;

    enum tags t;

    if((xml_file = open(file_name, O_RDONLY)) == -1){
        perror(file_name);
        exit(1);
    }

    for(i=0;i<1000;i++){
        t = readTag(xml_file, pos);
        if(t == TX){
            txo.outs_length = nTxOuts(xml_file, pos);
            //txo.outs = malloc(outs * sizeof(output));
            //printf("%c%c%c%c -> %llu\n", hash[0],hash[1],hash[2],hash[3],outs);
            getTxHash(xml_file, pos, txo.hash);
            txo.outs = arrayOfTxOuts(xml_file, pos);

            for(j=0;j<64;j++){
                printf("%c",txo.hash[j]);
            }
            printf("\n");
            for(outs=0;outs<txo.outs_length;outs++){
                for(j=0;j<4;j++){
                    printf(" ");
                }
                for(j=0;j<txo.outs[outs].address_length;j++){
                    printf("%c",txo.outs[outs].address[j]);
                }
                printf("\n");
            }

        }
        pos = nextTag(xml_file, pos);
    }
}
