#include "strutture.h"

typedef unsigned long long POSITION;
enum tags {BLOCK, TX, INPUTS, INPUT, OUTPUTS, OUTPUT, CLOSING, INTXHASH, ADDRESS, INDEX, ERROR};

POSITION nextTag(int fd, POSITION pos);
enum tags readTag(int fd, POSITION pos);
void getTxHash(int fd, POSITION pos, char* hash);
unsigned long long nTxOuts(int fd, POSITION pos);
output* arrayOfTxOuts(int fd, POSITION pos);
unsigned long long nTxInputs(int fd, POSITION pos);
unsigned long getOutIndex(int fd, POSITION pos);
void getInHash(int fd, POSITION pos, char *hash);
