#ifndef ARGMESSAGEINTERNALHELPERS_H
#define ARGMESSAGEINTERNALHELPERS_H
#include "config.h"
#include <stdbool.h>

struct argmessage_message {
    /* this makes use of the C99 flexible array feature */
    int messagesize;
    int proxyid;
    int proxysequencenumber;
    int adapterid;
    int functionid;
    int threadnum;
    /* FORLATER: ?? add here some attibutes to do with out of order execution, if needed 
    but for graph building execution of function could be preceded by testing that refererred to graph items exist!
    */
    int argsbuffersize;
    //char* argsbufferstart;
    char argsbuffer[];
};

#define ARGM_THREAD_TAG_BASE 1024

/* function prototypes */
void argmessage_helpersinit();
struct argmessage_message* argmessage_messagecreate(int argsbuffersize);
void argmessage_printbuffer(struct argmessage_message* message);
#if ARGM_WITH_SNAPI != 1 /* this funtion is not used with Snapi */
int argmessage_serverrank(bool iamserver, int *servercount);
#endif /* ARGM_WITH_SNAPI != 1 */

#endif /* ARGMESSAGEINTERNALHELPERS_H */