#include "config.h"

/* standard includes */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef ARGM_WITH_SNAPI
/* from test.c of Snapi example */
#include <snapi_mq.h>
#include <macrologger.h>
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* from test.c of Snapi example */
#else
#include <mpi.h>
#include <unistd.h>
#include "waitfordebugger.h"
#endif

// #if defined QSARGM_OPTIM_01 && defined HAVE_OPENMP 
// #include <omp.h>
// #endif

#include <omp.h>


/* local includes */
#include "argmessageproxy.h"
#include "argmessage.h"
#include "packunpack.h" 
#include "argmessageinternalhelpers.h"

#ifdef ARGM_WITH_SNAPI
/* from test.c of Snapi example */
static char *compute_dev_name = "mlx5_2";
/* from test.c of Snapi example */
snapi_mq_t snapi_client_mq;
struct snapi_mq_params snapi_client_params;

#endif

/* pick up this flag set from helpers.c */
extern bool functionisvoid[];

#if defined QSARGM_OPTIM_01 && defined HAVE_OPENMP 
extern char * messagebuffers;
#endif

/* object for the singleton pattern */
struct argmessage_proxy* proxy = NULL;
/* values for a dummy buffer - will be frequently made use of */

// removed for refactor to new message struct
// each send will need a new message object
//int dummybytescount = 0;
//char* dummybuffer = NULL;

struct argmessage_proxy* argmessage_proxygetobject(int argc, char* argv[]){
    // printf("P: in proxyinit\n");
    // fflush(stdout);
    if (proxy == NULL){
        //argmessage_helpersinit();
        // printf("P: doing struct initialisation\n");
        // fflush(stdout);
        //dummybuffer = packunpack_dummypayload(&dummybytescount);
        proxy = (struct argmessage_proxy*) malloc(sizeof(struct argmessage_proxy));
        proxy->maxrequests = 0;
        proxy->requestshandled = 0;
        //memset(proxy->commonreceivebuffer, 0, ARGMESSAGE_RECEIVE_BUFFER_BYTES);
        #if defined QSARGM_OPTIM_01 && defined HAVE_OPENMP
        int nr_threads = omp_get_max_threads();
        proxy->sendrecvbuffers = malloc(nr_threads * 2 * ARGMESSAGE_RECEIVE_BUFFER_BYTES);
        messagebuffers = proxy->sendrecvbuffers
        #endif

        // printf("P: doing MPI Init ...\n");
        // fflush(stdout);
        char hostname[256];
        gethostname(hostname, 256);
        printf("This is the argmessage client on host: %s\n", hostname);

#if ARGM_WITH_SNAPI == 1
/* from test.c of Snapi example basic_test_client() */
    LOG_INFO("Client: Starting test");
    // open..
    LOG_DEBUG("compute_dev_name is %s", compute_dev_name);
    snapi_client_params.message_length = ARGMESSAGE_RECEIVE_BUFFER_BYTES;  //TODO consider this value
    snapi_client_params.queue_size = 32;
    snapi_mq_status_t status;
    LOG_DEBUG("Client: calling snapi_mq_create");
    status = snapi_mq_create(compute_dev_name, &snapi_client_params, &snapi_client_mq);
    if (status != SNAPI_MQ_OK) {
        LOG_ERROR("snapi_mq_create failed with status %s",
                  snapi_mq_status_str(status));
    }
    // listen until connected
    LOG_DEBUG("Client: calling snapi_mq_connect");
    status = snapi_mq_connect(&snapi_client_mq);
    if (status != SNAPI_MQ_OK) {
        LOG_ERROR("Snapi_mq_send failed with status %s",
                  snapi_mq_status_str(status));
    }
    LOG_INFO("Client: successfully connected to server");
/* END from test.c of Snapi example basic_test_client() */
//TODO  allow for what engine->myrank and engine->ranks will be
// let's try for Snapi server - rank 0, client - rank 1
// will need to agree with server determination in argmessage helpers
    proxy->myrank=1;
    proxy->ranks=2;
    proxy->serverrank = 0; // fixed for snapi argmessage_serverrank(false, &servercount);
#else 
        int mpi_thread_provided = 0;
        MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpi_thread_provided);
        if (mpi_thread_provided != MPI_THREAD_MULTIPLE) {
            printf("MPI_THREAD_MULTIPLE not provided !!!\n");
        }
        wait_for_debugger();
#ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE PROXY OBJECT: mpi initialised\n"); fflush(stdout);
#endif
        MPI_Comm_rank(MPI_COMM_WORLD, &(proxy->myrank));
        MPI_Comm_size(MPI_COMM_WORLD, &(proxy->ranks));
#ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE PROXY OBJECT: rank=%i, worldsize=%i", proxy->myrank, proxy->ranks );
        printf("ARGMESSAGE PROXY OBJECT: calling serverrank ...\n");
        fflush(stdout);
#endif 
        /* praticipate in collective identification of the server's rank */
        int servercount=0; /* for an an out parameter - used a bit later below in conditional compiled sections */
        proxy->serverrank = argmessage_serverrank(false, &servercount);

#ifdef ARGM_SHOW_TRACE
        if (servercount>1){
            printf("ARGMESSAGE PROXY OBJECT(rank %d): Multiple servers found - proceeding with rank %d\n", 
                proxy->myrank, proxy->serverrank);
            fflush(stdout);
        } 
        else{
            printf("ARGMESSAGE PROXY OBJECT (rank %d): Server found at rank %d\n", 
                proxy->myrank, proxy->serverrank);
            fflush(stdout);
        }
#endif /* ARGM_SHOW_TRACE */
#endif /* ARGM_WITH_SNAPI */        

        /* enliven the adapter at the server corresponding to this proxy
         * this is a formal procedure before the adapter will accept other requests
         */
        // printf("P: doing enlivenadapter\n");
        // fflush(stdout);
        
        /* enliven just the zeroth for this proxy */
        argmessage_proxyenlivenadapter(proxy->myrank * ARGMESSAGE_ADAPTERS_PER_PROXY);
    }
    // printf("leaving proxyinit");
    // fflush(stdout);
    return proxy;
} 

int argmessage_proxyfree(){

#if ARGM_WITH_SNAPI == 1

    LOG_DEBUG("Snapi client: calling snapi_mq_close");
    snapi_mq_close(&snapi_client_mq);

#else

        /* FORNOW: close down MPI here - anywhere else better?*/
        printf("attempting to Finalize MPI - will wait at Barrier first\n");
        fflush(stdout);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();   
        printf("  MPI finalized\n");
        fflush(stdout);

#endif /* ARGM_WITH_SNAPI == 1 */

    if (proxy==NULL){
        /* this is an error - client should get a refernce to the proxy first */
        return 1;
    }
    /* free dependent resources first */
    //free(dummybuffer);
    //free(proxy->commonreceivebuffer);
    /* and then free the engine itself */
    printf("attempting to free proxy\n");
    fflush(stdout);
    #if defined QSARGM_OPTIM_01 && defined HAVE_OPENMP
    free(proxy->sendrecvbuffers);
    #endif
    free(proxy); //client's pointer to this is now out of date!, so:
    printf(" proxy freed\n");
    fflush(stdout);

    proxy = NULL; // this is the static pointer to the singleton serverengine
    return 0;
}

void argmessage_proxyrun(int maxrequests){
    if (proxy==NULL){
        proxy->maxrequests = maxrequests;
    }
}


void argmessage_proxyechoproxystatusonproxy(){
    printf("------------------------------------------\n");
    printf("         ARGMESSAGE PROXY STATUS\n");
    printf("Request to print was local on rank #%d\n", proxy->myrank);
    printf("Values from the proxy struct:\n");
    printf("MPI Rank               : %d\n", proxy->myrank);
    printf("MPI ranks count        : %d\n", proxy->ranks);
    printf("Server rank            : %d\n", proxy->serverrank);
    printf("Requests limit         : %ld\n", proxy->maxrequests);
    printf("Requests so far        : %ld\n", proxy->requestshandled);
    printf("------------------------------------------\n\n");
    return;
}



/* functionname can be obtained with the __func__ GCC magic constant */


// BEGIN: wrappers for internal funtions that can be called on the server


/* TODO: a list of remote functions to wrap - some not yet implemented at server end
enum argmessage_server_functions {
    DONE: nooperation = ARGMESSAGE_SERVER_FUNCTIONS_BASE,
    DONE: enlivenadapter, 
    DONE: killadapter,
    DONE: killserverrequest,
    DONE: stdoutmessage,
    echoenginestatus,
    verbosityfilteredstdoutmessage,
    echofunctionnames,
    setunknownfunctionnumberbehaviour,
    startlog,
    logmessage,
    closelog
};
*/
void argmessage_proxysendnoop(){
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = nooperation;
    argmessage_proxysendfunction(message);
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif
    return;
}

void argmessage_proxyenlivenadapter(int adapter_idx){
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = enlivenadapter;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif
    return;
}

void argmessage_proxyrestoreserversinglethreaded(int adapter_idx){
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM CLIENT: restoreserversinglethreaded\n");
    fflush(stdout);
    #endif
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = restoreserversinglethreaded;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM CLIENT: restoreserversinglethreaded - receiving reply\n");
    fflush(stdout);
    #endif
    struct argmessage_message* reply = argmessage_proxyreceivefunctionresult(message);
    int server_active_threads = *((int*)reply->argsbuffer);
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM CLIENT: Number of active threads on sever received as %d\n", server_active_threads);
    fflush(stdout);
    #endif
    if (server_active_threads > 1){
        int noops_to_send = server_active_threads - 1;  // had been -1 for the thread that continues but -2 because freesched or something else seems to kill another one, but then this did not seem to be enough so back to 1 
        if (noops_to_send < 0){
            noops_to_send = 0;
        }
        printf("ARGM CLEINT: sending noops for %d server threads\n", noops_to_send );
        fflush(stdout);
        for (int idx=1; idx<=noops_to_send; idx++){
            printf("ARGM CLIENT: sending a noops index %d \n", idx);
            fflush(stdout);
            argmessage_proxysendnoop();
            printf("ARGM CLIENT: noop index %d sent\n", idx);
            fflush(stdout);
        }
    }
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif
    free(reply);
    return;
}


void argmessage_proxykilladapter(int adapter_idx){
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM CLIENT: entering kill adapter\n");
    fflush(stdout);
    #endif
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = killadapter;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM CLIENT: kill adapter message sent - receiving reply\n");
    fflush(stdout);
    #endif
    struct argmessage_message* reply = argmessage_proxyreceivefunctionresult(message);  // reply is a dummy - needed for timing?
#ifdef ARGM_SHOW_TRACE
    printf("leaving proxykilladapter\n");
    fflush(stdout); 
#endif /* ARGM_SHOW_TRACE */

    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif
    free(reply);
    return;
}

void argmessage_proxykillserverrequest(){
    /* this request is per proxy so message->adapterid not changed from default =-1 */
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = killserverrequest;
    argmessage_proxysendfunction(message);
    struct argmessage_message* reply = argmessage_proxyreceivefunctionresult(message);  // reply is a dummy - to sensure this has been processed before killing adapter
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif
    free(reply);
    return;
}

void argmessage_proxysendstdoutmessage(char* string0){
    /* this request is from the proxy to the server (not to an adapter) */
    struct argmessage_message* message = packunpack_packonestring(string0);
    message->functionid = stdoutmessage;
    argmessage_proxysendfunction(message);
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif
    return;
}

int argmessage_proxyhowmanyliveadapters(){
    /* this request is from the proxy to the server (not to an adapter) */
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = howmanyadapters;
#ifdef ARGM_SHOW_TRACE
    printf("in proxyhowmanyliveadapters - sending \n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    argmessage_proxysendfunction(message);
#ifdef ARGM_SHOW_TRACE
    printf("in proxyhowmanyliveadapters - receiving\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    struct argmessage_message* reply = argmessage_proxyreceivefunctionresult(message);
    int int0 = *((int*)reply->argsbuffer); 
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    free(reply);
    #endif
    return int0;
}

// END: wrappers for internal funtions that can be called on the server

/* a function for client codes to call their own functions at the server 
 * (that have been registered there)
 * also serves as remote caller for internal fucntions - e.g. in wrappers above
 */
void argmessage_proxysendfunction(struct argmessage_message* message){
    // functiontag is both MPI tag and id for the function to be called
    // application programmer is invited to set up an enum of funcition names to avoid mapping errors
    // QUESTION: are tags particular to the MPI Communicator? 
    // (this is a library so should not disrupt application's tags)
    if ((proxy->maxrequests > 0) && (proxy->requestshandled >= proxy->maxrequests)){
        printf("PROXY OBJECT (rank %d):  ERROR: max requests exceeded\n", proxy->myrank);
        fflush(stdout);
    }
    // TODO - this is not thread safe - but may ditch this check for multithreaded. (as it is this is only used for a check when trace is in use)
    proxy->requestshandled++;
    message->proxysequencenumber = proxy->requestshandled; 
    message->proxyid = proxy->myrank;
    message->threadnum = omp_get_thread_num();
    //fprintf(stderr, "Client - sending message with proxyid = %d, functionid = %d, thread id=%d, payload 1st int=%d\n", proxy->myrank, message->functionid, message->threadnum, *((int*)message->argsbuffer));;
#ifdef ARGM_SHOW_TRACE
    printf("PROXY OBJECT: sending this message:\n");
    argmessage_printbuffer(message);
    printf("\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */

#if ARGM_WITH_SNAPI == 1
// this is the argmessage client and will be the snapi client as well
// so start snapi as the client. 
    snapi_mq_status_t status;
    LOG_DEBUG("Snapi server: calling snapi_mq_send");
    status = snapi_mq_send(&snapi_client_mq, message, message->messagesize);
    if (status != SNAPI_MQ_OK) {
        LOG_ERROR("Snapi_mq_send failed with status %s",
                  snapi_mq_status_str(status));
    }
#else
// penultimate arg is the MPI tag TODO - currently using function id - need tag to identify thread?? (incorporate function and thread in tag - then in receive match the tag used.
// Qustion should the function id be incorporated into the tag or should it go in the message body allowing 
// Will it be correct anyway - yes if only gettask has a return type - the return value is the task id so any thread could process it - but then the scheduler is trying to be cache efficient - which is then lost.  
// BUT functionid is already in the message!
    MPI_Send((void*) message, message->messagesize, MPI_BYTE, proxy->serverrank, message->functionid, MPI_COMM_WORLD);  // functionid tag not yet used at receiver - different threads there could process these
    //fprintf(stderr, "Client - SENT    message with proxyid = %d, functionid = %d, thread id=%d, payload 1st int=%d\n", proxy->myrank, message->functionid, message->threadnum, *((int*)message->argsbuffer));
#endif /* ARGM_WITH_SNAPI == 1 */
    return;
}

struct argmessage_message* argmessage_proxyreceivefunctionresult(struct argmessage_message* sentmessage){
    /* FORNOW: Wait for server to reply 
     * check sequence number anyway 
     * LATER: may want to use sequence number to find the return value from amongst those received 
     */

#ifdef ARGM_SHOW_TRACE
    printf("ARGMESSAGE PROXY; waiting for function result\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */

    #if defined QSARGM_OPTIM_01 && defined HAVE_OPENMP
    char* buffer = proxy->sendrecvbuffers + (omp_get_thread_num() * 2 + 1) * ARGMESSAGE_RECEIVE_BUFFER_BYTES;
    #else
    char* buffer = malloc(ARGMESSAGE_RECEIVE_BUFFER_BYTES);
    #endif

    //int thread_num = omp_get_thread_num();
    //fprintf(stderr, "Client - receiving message on thread id=%d, sent thread=%d, functionid%d\n", thread_num, sentmessage->threadnum, sentmessage->functionid);

#if ARGM_WITH_SNAPI == 1
    LOG_DEBUG("Snapi client: calling snapi_mq_recv");
    size_t snapi_recv_len;
    while (snapi_mq_recv(&snapi_client_mq, buffer, &snapi_recv_len) != SNAPI_MQ_OK)
        ;
    LOG_DEBUG("Snapi client: received message of size %ld", snapi_recv_len);
#else
    MPI_Status status;
    MPI_Recv(buffer, ARGMESSAGE_RECEIVE_BUFFER_BYTES, MPI_BYTE, proxy->serverrank, ARGM_THREAD_TAG_BASE + sentmessage->threadnum, MPI_COMM_WORLD, &status);
    //int latestthreadnum = omp_get_thread_num();
    //fprintf(stderr, "Client - RECEIVED  message on thread id=%d, latestthreadnum=%d, sent thread=%d, MPI status tag=%d\n", thread_num, latestthreadnum, sentmessage->threadnum, status.MPI_TAG);
    /* message is obtained by recasting the MPI buffer 
     * note for now the buffer is common to all receives - 
     * consumer of message with need to copy out its payload values 
     * which it will do in normal unpacking methods
     * but beware if we switch to implementation with plural buffers existing concurrently
     */
#endif /* #if ARGM_WITH_SNAPI == 1 */

    struct argmessage_message* reply = (struct argmessage_message*) buffer;  
    
#ifdef ARGM_SHOW_TRACE
    printf("ARGMESSAGE PROXY: raw message received:\n");
    argmessage_printbuffer(reply);
    printf("receivefunctionresult: retruned from printbuffer\n");
    fflush(stdout);
    /* check that sequence number of reply message obtained matches the incoming one */
    if (reply->proxysequencenumber != sentmessage->proxysequencenumber){
        printf("ARGMESSAGE PROXY:  ERROR: reply from server has incorrect sequence number!!\n");
        fflush(stdout);
    }
    printf("receivefunctionresult: returning reply\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    return reply;
}

