#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "file_handling.h"

//LAST TX c1311ddfb105f44f952421669d5961a147a6bca5350c522f14d6198b1f341526

//for test 91971bf3545f1cdaa2554dc1c9a150cd11893e1fe3034975d9dfac93248bb8ea

unsigned long hashToIndex(char *hash){
    char string[5];
    for(int i=0; i<4; i++){
        string[i] = hash[i];
    }
    string[4] = '\0';
    char *end;
    return strtoul(string, &end, 16);
}

int main(){
    int xml_file;
    unsigned long long i,j;
    int k;
    const char *file_name = "file.xml";
    POSITION pos = 0;
    unsigned long index;
    tx_outputs *txo;
    enum tags t;

    unsigned long long ntxin;
    char hash[65];
    char in_hash[65];

    FILE *stream;
    stream = fopen("result_", "w");

    tx_outputs **table = malloc(0xffff * sizeof(tx_outputs*));
    for(i=0;i<0xffff;i++){
        table[i] = NULL;
    }

    if((xml_file = open(file_name, O_RDONLY)) == -1){
        perror(file_name);
        exit(1);
    }

    int cont = 1;

    do{
        t = readTag(xml_file, pos);
        if(t == TX){
            txo = malloc(sizeof(tx_outputs));
            (*txo).outs_length = nTxOuts(xml_file, pos);
            getTxHash(xml_file, pos, (*txo).hash);
            index = hashToIndex((*txo).hash);
            (*txo).outs = arrayOfTxOuts(xml_file, pos);
            table[index] = txo;

            int comp = strncmp((*txo).hash, "91971bf3545f1cdaa2554dc1c9a150cd11893e1fe3034975d9dfac93248bb8ea", 64);
            if(!comp){
                cont = 0;
            }
        }
        pos = nextTag(xml_file, pos);
    }while(cont);
    printf("Caricamento terminato\n");

    pos = 0;
    cont = 1;

    do{
        t = readTag(xml_file, pos);
        if(t == TX){
            getTxHash(xml_file, pos, hash);
            hash[64] = '\0';
            fprintf(stream, "%s:", hash);
            ntxin = nTxInputs(xml_file, pos);

            pos = nextTag(xml_file, pos); //posiziona sul tag <inputs>
            pos = nextTag(xml_file, pos); //posiziona sul primo tag <input>

            for(i=0; i<ntxin; i++){
                pos = nextTag(xml_file, pos); //posizione su tag <index>
                unsigned long out_index = getOutIndex(xml_file, pos);
                pos = nextTag(xml_file, pos); //posizione su tag </index>
                pos = nextTag(xml_file, pos); //posizione su tag <in_tx_hash>
                getInHash(xml_file, pos, in_hash);
                in_hash[64] = '\0';
                index = hashToIndex(in_hash);
                if(index == 0){
                    fprintf(stream, "_coinbase_");
                }
                else{
                    if (table[index] != NULL){
                        if(strncmp(in_hash, table[index]->hash, 64)){
                            //printf("collision!\n");
                            //printf("in_hash:\t\t %s\n", in_hash);
                            //printf("table[index]->hash:\t %s\n", table[index]->hash);
                            fprintf(stream, "NotFound");
                        }
                        else{
                            for(k=0; k<(table[index]->outs[out_index]).address_length; k++){
                                fprintf(stream, "%c", (table[index]->outs[out_index]).address[k]);
                            }
                        }
                    }
                    else{
                        fprintf(stream, "NotFound");
                    }
                }

                pos = nextTag(xml_file, pos); //posizione su tag </in_tx_hash>
                pos = nextTag(xml_file, pos); //posizione su tag </input>
                pos = nextTag(xml_file, pos); //posizione su tag <input>
                if(i < ntxin-1){
                    fprintf(stream, ",");
                }
            }

            fprintf(stream, "\n");
            int comp = strncmp(hash, "91971bf3545f1cdaa2554dc1c9a150cd11893e1fe3034975d9dfac93248bb8ea", 64);
            if(!comp){
                cont = 0;
            }
        }
        pos = nextTag(xml_file, pos);
    }while(cont);

}
