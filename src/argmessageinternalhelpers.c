#include "config.h"

/* standard includes*/
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef ARGM_WITH_SNAPI
#include <snapi_mq.h>
#else
#include <mpi.h>
#endif

#if defined QSARGM_OPTIM_01 && defined HAVE_OPENMP 
#include <omp.h>
#endif


#include "argmessageinternalhelpers.h"
#include "argmessage.h"
#include "argmessageproxy.h"

/* DELETE: ?? do we need tehse two - if not registering on client */
// bool functionisvoid[ARGMESSAGE_MAX_FUNCTION_NAMES];

// void argmessage_helpersinit(){
//     for (int idx=0; idx<ARGMESSAGE_MAX_FUNCTION_NAMES; idx++) functionisvoid[idx] = true;
//     /* initialise for default functions of server 
//     * elements are those of enum argmessage_server_functions 
//     * a user progam based on this library should initialise its elements of this array
//     */
//     functionisvoid[nooperation] = true;
//     functionisvoid[enlivenadapter] = true;
//     functionisvoid[killadapter] = true;
//     functionisvoid[killserverrequest] = true;
//     functionisvoid[stdoutmessage] = true;
//     functionisvoid[echoenginestatus] = true;
//     functionisvoid[howmanyadapters] = false; /* this one provides a reply */
//     functionisvoid[verbosityfilteredstdoutmessage] = true;
//     functionisvoid[echofunctionnames] = true;
//     functionisvoid[setunknownfunctionnumberbehaviour] = true;
//     functionisvoid[startlog] = true;
//     functionisvoid[logmessage] = true;
//     functionisvoid[closelog] = true;
//     return;
// }

// extern struct argmessage_proxy* proxy;
#if defined QSARGM_OPTIM_01 && defined HAVE_OPENMP 
    // this is to be set by the proxy to proxy->sendrecvbuffers 
    // and for the server to what? - needs supplying there
    char * messagebuffers;
#endif

char *argmessage_server_function_id_to_string(uint idx){
    static char *argmessage_server_function_names[closelog - ARGMESSAGE_SERVER_FUNCTIONS_BASE + 1];
    argmessage_server_function_names[nooperation - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "nooperation";
    argmessage_server_function_names[enlivenadapter - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "enlivenadapter";
    argmessage_server_function_names[killadapter - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "killadapter";
    argmessage_server_function_names[killserverrequest - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "killserverrequest";
    argmessage_server_function_names[stdoutmessage - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "stdoutmessage";
    argmessage_server_function_names[echoenginestatus - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "echoenginestatus";
    argmessage_server_function_names[howmanyadapters - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "howmanyadapters";
    argmessage_server_function_names[verbosityfilteredstdoutmessage - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "verbosityfilteredstdoutmessage";
    argmessage_server_function_names[echofunctionnames - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "echofunctionnames";
    argmessage_server_function_names[setunknownfunctionnumberbehaviour - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "setunknownfunctionnumberbehaviour";
    argmessage_server_function_names[startlog - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "startlog";
    argmessage_server_function_names[logmessage - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "logmessage";
    argmessage_server_function_names[closelog - ARGMESSAGE_SERVER_FUNCTIONS_BASE] = "closelog";
    if (idx >= 0 && idx <= closelog - ARGMESSAGE_SERVER_FUNCTIONS_BASE) {
        return argmessage_server_function_names[idx];
    } 
    return "";
};


struct argmessage_message* argmessage_messagecreate(int argsbuffersize){
    int headersize = offsetof(struct argmessage_message, argsbuffer); // offset of is a macro in stddef.h //6*sizeof(int);
    // int headersize = sizeof(struct argmessage); // not allowed "incomplete type"
    //printf("oldheadersize=%d, newheadersize=%d", oldheadersize, headersize);

    // TODO - note that loc_qsched_run() has a runtime determination for using pthreads instead of openmp
    // which might make a nonsense of the openmp branch here: 
    #if defined QSARGM_OPTIM_01 && defined HAVE_OPENMP 
    // + 0 offset for send buffer, +1 for corresponding recv  - message create is used for sending 
    // omp_get_thread_num(): "The routine returns 0 if it is called from the sequential part of a program.", which is fine for this program
    struct argmessage_message* newitem = (struct argmessage_message*) (proxy->sendrecvbuffers + (omp_get_thread_num() * 2 + 0) * ARGMESSAGE_RECEIVE_BUFFER_BYTES);
    #else
    struct argmessage_message* newitem = (struct argmessage_message*)
                                      malloc(headersize + argsbuffersize);
    #endif
    newitem->messagesize = headersize+argsbuffersize;
    /* OPTIMISATION: setting next three items to -1 can be omitted, -1 means not yet set */
    newitem->proxyid = -1;
    newitem->proxysequencenumber = -1;
    newitem->adapterid = -1;
    newitem->functionid = -1;
    newitem->threadnum = -1;
    newitem->argsbuffersize = argsbuffersize;
    //newitem->argsbufferstart = newitem->argsbuffer; // its already a pointer
    return newitem;
}

void argmessage_printbuffer(struct argmessage_message* message){
    printf("Mess size:    Proxy ID:     Prox seq no:  Adapter ID:   Func ID:      Thread ID     Argbuff size: \n");
    char* bytearray = (char*) message;
    for (int offset=0; offset<84; offset++){
        char c = (unsigned char) *(bytearray+offset);
        if (offset<message->messagesize){
            // if (c<=-1) printf("-- "); else printf("%02X ", c );            
            printf("%02X ", c & 0xff);            
        }
        else printf(" ~ ");
        if ((offset % 4) == 3) printf("| ");
        if ((offset % 28) == 27) printf("\n");
    }
    printf("Text Equivalent :-\n");
    for (int offset=0; offset<84; offset++){
        char c = *(bytearray+offset);
        if (offset<message->messagesize){
            if (c>=31 && c<=126) printf(" %c ", c); else printf("   ");
        }
        else printf(" ~ ");        
        if ((offset % 4) == 3) printf("| ");
        if ((offset % 28) == 27) printf("\n");
    }
    //printf("\n");
    // printf("struct members:\n  messagesize=%d, proxyid=%d\n  proxysequencenumber=%d\n adapterid= %d, functionid=%d\n  argsbuffersize=%d, argsbuffer%p\n messagestartaddress=%p\n",
    //      message->messagesize, message->proxyid, 
    //      message->proxysequencenumber,message->adapterid, message->functionid, 
    //      message->argsbuffersize, message->argsbuffer, (char*)message);
    printf("selected struct members:\n  adapterid=%d, args buffer address=%p, message start address=%p, function name=%s\n",
        message->adapterid, message->argsbuffer, (char*)message, argmessage_server_function_id_to_string(message->functionid));
    printf("print message buffer: returing from printbuffer\n");
}

#if ARGM_WITH_SNAPI != 1
/* to be run on all MPI ranks at the same time */
int argmessage_serverrank(bool iamserver, int *servercount){
    printf("in serverrank...\n");
    printf(" - iamserver=%d, passed servercount=%d\n", iamserver,*servercount);
    fflush(stdout);
    int ranks, myrank;

    printf(" - calling MPI Comm rank...\n");  
    fflush(stdout);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    printf(" - calling MPI Comm size...\n");
    fflush(stdout);
    MPI_Comm_size(MPI_COMM_WORLD, &ranks);
    printf("ranks=%d, rank=%d\n", ranks, myrank);
    printf(" - allocating responses...\n");
    fflush(stdout);
    bool* responses = malloc(sizeof(bool) * ranks);
    for (int j=0; j<ranks; j++) {
        responses[j]=false;
        printf("j=%d, responses[j]=%d\n", j, (int)responses[j]);
    }

    fflush(stdout);
    printf(" - calling MPI Allgather...\n");
    fflush(stdout);
    /*
    int MPI_Allgather(const void *sendbuf, int  sendcount,
     MPI_Datatype sendtype, void *recvbuf, int recvcount,
     MPI_Datatype recvtype, MPI_Comm comm)
     */
    MPI_Allgather(&iamserver, 1, MPI_C_BOOL, responses, 1, MPI_C_BOOL, MPI_COMM_WORLD);
    printf(" - ... Allgather complete\n");
    fflush(stdout);

    // print (for up to 10 ranks)
    char rep[] = "0-1-2-3-4-5-6-7-8-9-";
    printf(" - rep initialised: %s\n", rep);
    fflush(stdout);

    for(int i=0; i<ranks; i++){
            printf("i=%d, responses[%d]=%d\n", i, i, (int)responses[i]);
            fflush(stdout);
        if (responses[i]==true){
            rep[i*2+1]='S';
        }
        else {
            rep[i*2+1]='P';
        }
    } 
    printf("\n\nrank=%d, iamserver=%d, server gather result %s\n\n", myrank, iamserver, rep);
    fflush(stdout);

    int serveridx = -1; /* -1 not yet found */
    (*servercount) = 0;
    for (int rankidx=0; rankidx<ranks; rankidx++){
        if (responses[rankidx]==true) {
            if (serveridx == -1) {
                serveridx = rankidx;
            }
            (*servercount)++;
        }
    }
    free(responses);
    printf("leaving serverrank: serveridx=%d, servercount=%d\n\n", serveridx,*servercount);
    fflush(stdout);
    return serveridx; /* -1 not found, otherwise lowest rank found */
}
#endif /* #if ARGM_WITH_SNAPI == 1 */ 


