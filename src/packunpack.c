#include "config.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "argmessageinternalhelpers.h"


struct argmessage_message* packunpack_dummypayload(){
    int argsbuffersize = sizeof(int);
    struct argmessage_message* message = argmessage_messagecreate(argsbuffersize);
    /* using message->argbuffer directly results in the error:
     * "dereferencing type-punned pointer will break strict-aliasing rules"
     */
    char* bytearray = (char*) message->argsbuffer; 
    *((int*)bytearray) = 0;
    return message;
}

struct argmessage_message* packunpack_packonestring(char* string0){
    int string0len = strlen(string0) + 1;
    int argsbuffersize = sizeof(int) + string0len ;
    struct argmessage_message* message = argmessage_messagecreate(argsbuffersize);
    char* bytearray = (char*) message->argsbuffer;
    *((int*)bytearray) = string0len;
    memcpy(bytearray + sizeof(int), string0, string0len);
    return message;
}


struct argmessage_message* packunpack_packoneint(int int0){
    int argsbuffersize = sizeof(int);
    struct argmessage_message* message = argmessage_messagecreate(argsbuffersize);
    char* bytearray = (char*) message->argsbuffer;
    *((int*)bytearray) = int0;
    return message;
}

struct argmessage_message* packunpack_packtwoint(int int0, int int1){
    int argsbuffersize = 2 * sizeof(int);
    struct argmessage_message* message = argmessage_messagecreate(argsbuffersize);
    int* intarray = (int*)message->argsbuffer;
    intarray[0] = int0;
    intarray[1] = int1;
    return message;
}

struct argmessage_message* packunpack_packthreeint(int int0, int int1, int int2){
    int argsbuffersize  = 3 * sizeof(int);
    struct argmessage_message* message = argmessage_messagecreate(argsbuffersize);
    int* intarray = (int*)message->argsbuffer;
    intarray[0] = int0;
    intarray[1] = int1;
    intarray[2] = int2;
    return message;
}

struct argmessage_message* packunpack_packfourint(int int0, int int1, int int2, int int3){
    int argsbuffersize = 4 * sizeof(int);
    struct argmessage_message* message = argmessage_messagecreate(argsbuffersize);
    int* intarray = (int*)message->argsbuffer;
    intarray[0] = int0;
    intarray[1] = int1;
    intarray[2] = int2;
    intarray[3] = int3;
    return message;
}

struct argmessage_message* packunpack_packfiveint(int int0, int int1, int int2, int int3, int int4){
    int argsbuffersize = 5 * sizeof(int);
    struct argmessage_message* message = argmessage_messagecreate(argsbuffersize);
    int* intarray = (int*)message->argsbuffer;
    intarray[0] = int0;
    intarray[1] = int1;
    intarray[2] = int2;
    intarray[3] = int3;
    intarray[4] = int4;
    return message;
}

struct argmessage_message* packunpack_packsixint(int int0, int int1, int int2, int int3, int int4, int int5){
    int argsbuffersize = 6 * sizeof(int);
    struct argmessage_message* message = argmessage_messagecreate(argsbuffersize);
    int* intarray = (int*)message->argsbuffer;
    intarray[0] = int0;
    intarray[1] = int1;
    intarray[2] = int2;
    intarray[3] = int3;
    intarray[4] = int4;
    intarray[5] = int5;
    return message;
}

void packunpack_callnoargs(int adapter_idx, void (*func)(), struct argmessage_message* message){
    /* i.e. dropping message */
    func(adapter_idx);
}

struct argmessage_message* packunpack_callnoargswithreturn(int adapter_idx, struct argmessage_message* (*func)(), struct argmessage_message* message){
    /* i.e. dropping message */
    return func(adapter_idx);
}


void packunpack_callonestring(int adapter_idx, void (*func)(), struct argmessage_message* message){
    char* bytearray = (char*) message->argsbuffer;
    int string0len = *((int*) bytearray);
    func(adapter_idx, string0len, bytearray + sizeof(int));
}


void packunpack_callbytearray(int adapter_idx, void (*func)(), struct argmessage_message* message){
    func(adapter_idx, message->argsbuffer);
    return;
}

void packunpack_calloneint(int adapter_idx, void (*func)(), struct argmessage_message* message){
    int* intarray = (int*) message->argsbuffer;
    func(adapter_idx, intarray[0]);
    return;
}

void packunpack_calltwoint(int adapter_idx, void (*func)(), struct argmessage_message* message){
    //printf("DEBUG: in calltwoint ...\n");
    //fflush(stdout);
    int* intarray = (int*) message->argsbuffer;
    //printf("DEBUG: intarray[0]=%d, intarray[1]=%d\n", intarray[0], intarray[1]);
    func(adapter_idx, intarray[0], intarray[1]);
    return;
}

void packunpack_callthreeint(int adapter_idx, void (*func)(), struct argmessage_message* message){
    int* intarray = (int*) message->argsbuffer;
    func(adapter_idx, intarray[0], intarray[1], intarray[2]);
    return;
}

void packunpack_callfourint(int adapter_idx, void (*func)(), struct argmessage_message* message){
    int* intarray = (int*) message->argsbuffer;
    func(adapter_idx, intarray[0], intarray[1], intarray[2], intarray[3]);
    return;
}

void packunpack_callfiveint(int adapter_idx, void (*func)(), struct argmessage_message* message){
    int* intarray = (int*) message->argsbuffer;
    func(adapter_idx, intarray[0], intarray[1], intarray[2], intarray[3], intarray[4]);
    return;
}

void packunpack_callsixint(int adapter_idx, void (*func)(), struct argmessage_message* message){
    int* intarray = (int*) message->argsbuffer;
    func(adapter_idx, intarray[0], intarray[1], intarray[2], intarray[3], intarray[4], intarray[5]);
    return;
}

