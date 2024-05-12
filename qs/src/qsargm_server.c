/* 
 * A server program using the argmessage library
 * in quicksched
 * this server should support all strategies - i.e. the various splits of the qshced functions
 * between client and server. 
 * 
 */


/* standard inlcudes */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <mpi.h>

/* local includes */
#include "argmessagesizes.h"
#include "packunpack.h"
#include "argmessage.h"
#include "qsargm_adapter0.h"
#include "waitfordebugger.h"

int main(int argc, char* argv[]){
    // TODO: interpret arguments

    /* intialise the server engine */
    /* returns a reference to the singleton engine object */
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM SERVER APP: starting initialisation of engine ...\n\n");
#endif /* ARGM_SHOW_TRACE */
    struct argmessage_serverengine* engine = argmessage_serverenginegetobject(argc, argv); 

    //char dummy[] = "DUMMY\n";
    //printf("%s", dummy);
    //printf("Rank: %d\n", engine->myrank);
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM SERVER APP: echoing initial status of server engine ...\n\n");
#endif /* ARGM_SHOW_TRACE */
    argmessage_serverengineechoenginestatus(engine->myrank);
    
    /* register the qsched application's functions with the engine, for strategy 0  */
    // TODO: sort out in the for statment which adapter does for strategy 0 per client 

    for (int adapter0_idx=0; adapter0_idx<ARGMESSAGE_MAX_ADAPTERS_PER_ENGINE; adapter0_idx += ARGMESSAGE_ADAPTERS_PER_PROXY){
        qsargmadapter0_serverregisterfunctions(adapter0_idx);
    }


    /* intialise the application servers' states */ 
    qsargm_serversinit0(engine);

    /* set the engine running */
    int maxrequests = 1000;
#ifdef ARGM_SHOW_TRACE
    printf("QSARGMSERVER APP: starting engine with %d requests limit ...\n\n", maxrequests);
#endif /* ARGM_SHOW_TRACE */
    argmessage_serverenginerun(maxrequests);    

    /* server engine has terminated, so tidy up */
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM SERVER APP: engine done - cleaning up ...\n\n");
    printf("QSARGM SERVER APP: but first echoing final status of engine ...\n\n");
#endif /* ARGM_SHOW_TRACE */
    argmessage_serverengineechoenginestatus(engine->myrank);
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM SERVER APP: cleaning up server states ...\n\n");
#endif /* ARGM_SHOW_TRACE */
    // TO DO: write the implementation of the is functions - is it per strategy/adapter?
    //qsargm_serversfree();
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM SERVER APP: cleaning up engine ...\n\n");
#endif /* ARGM_SHOW_TRACE */
    /* this frees the engine object */
    argmessage_serverenginefree();

    /* BYE */
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM SERVER APP: server app finished.\n");
#endif /* ARGM_SHOW_TRACE */
    return 0;
}
