/* 
 * A demo client program using the argmessage library of quicksched
 * 
 * this demo uses some of the quicksced functions
 * 
 * when it is debugged the official examples of the quicksched library can be similarly coded. 
 *  
 * This demo sets up all the various strategies, i.e. the different splits, of implementing the qsched functions. 
 * 
 */


/* standard inlcudes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* local includes */
#include "argmessagesizes.h"
#include "packunpack.h"
#include "argmessage.h"
#include "argmessageproxy.h"
// no ! this should just include the modified quicksced library, which is proxy and strategy aware
#include "qsargm_common0.h"
#include "qsargm_proxy0.h"
#include "waitfordebugger.h"

extern struct argmessage_proxy* proxy;

int main(int argc, char* argv[]){
    // TODO: interpret arguments

    /* intialise the proxy */
    /* returns a reference to the singleton proxy object */
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM CLIENT APP: starting initialisation of proxy...\n\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    /* can do this if you need to refer to the proxy struct
     * but if you use that and then do no so refere gcc complains it is unused:
     * struct argmessage_proxy* proxy = argmessage_proxygetobject(argc, argv); 
     */
    argmessage_proxygetobject(argc, argv); 

    //printf("Rank: %d\n", engine->myrank);
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM CLIENT APP: echoing initial status of proxy ...\n\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    argmessage_proxyechoproxystatusonproxy(); 



    /* set proxy running */
    int maxrequests = 100;
#ifdef ARGM_SHOW_TRACE
    printf("ARGMESSAGE CLIENT APP: starting engine with %d requests limit ...\n\n", maxrequests);
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    argmessage_proxyrun(100);    

    /* example calls to quicksched */
    /* task graph generation phase */


    /* run phase */


    /* clean up phase */



    char examplemessage[] = "Example quicksched calcualtion is now done.";
    fflush(stdout);
    argmessage_proxysendstdoutmessage(examplemessage);

  
    /* sign off with server */
    argmessage_proxykillserverrequest();
    // TODO: amend to handle each strategy 
    argmessage_proxykilladapter(proxy->myrank * ARGMESSAGE_ADAPTERS_PER_PROXY);

    /* server should terminated, so tidy up this proxy as well*/
    /* this frees the proxy object */
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM CLIENT APP: proxy done - cleaning up ...\n\n");
    printf("QSARGM CLIENT APP: but first echoing final status of proxy ...\n\n");
#endif /* ARGM_SHOW_TRACE */
    argmessage_proxyechoproxystatusonproxy();

    argmessage_proxyfree();

    /* BYE */
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM CLIENT APP: client app finished.\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    return 0;
}