#include <errno.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

void base94_encode(const unsigned char *plain, unsigned char *code);

void base94_decode(const unsigned char *code, unsigned char *plain);