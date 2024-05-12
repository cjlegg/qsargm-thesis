#include "config.h"

/* standard includes*/
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <omp.h>

#if ARGM_WITH_SNAPI == 1
/* from test.c of Snapi example */
#include <snapi_mq.h>
#include <macrologger.h>
#include <snapi_mq.h>
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* END from test.c of Snapi example */
#else
#include <mpi.h>
#include <unistd.h>
#include "waitfordebugger.h"
#endif
/* local includes */
#include "argmessage.h"
#include "argmessagesizes.h"
#include "packunpack.h"
#include "argmessageinternalhelpers.h"
#include "atomic.h"


#if ARGM_WITH_SNAPI == 1
/* from test.c of Snapi example */
static char *bf_dev_name = "mlx5_0";
snapi_mq_t snapi_server_mq;
struct snapi_mq_params snapi_server_params;
/* END from test.c of Snapi example */
#endif /* ARGM_WITH_SNAPI == 1 */


extern bool functionisvoid[];

#if defined QSARGM_OPTIM_01 && defined HAVE_OPENMP 
extern char * messagebuffers;
#endif


/* BEGIN: helper function section */




/* END: helper function section */


/* BEGIN: server engine section */

struct argmessage_serverengine* engine = NULL;
/* serverengine - singleton pattern 
 *              - so getobject returns the existing one or instantiates one
*/
struct argmessage_serverengine* argmessage_serverenginegetobject(int argc, char* argv[]){
    //argmessage_helpersinit();
    // printf("S: in server getobject\n");
    // fflush(stdout);

    if (engine == NULL){
        // printf("S: creating server object struct\n");
        // fflush(stdout);
        engine = (struct argmessage_serverengine*) malloc(sizeof(struct argmessage_serverengine));
        // printf("S: initialising MPI\n");
        // fflush(stdout);
        //MPI_Init(&argc, &argv);
        char hostname[256];
        gethostname(hostname, 256);
        printf("This is the argmessage server on host: %s\n", hostname);
#if ARGM_WITH_SNAPI == 1
// this is the server of argmessage and will also be the Snapi server
// from test.c of Snapi example basic_server_test
    LOG_INFO("Snapi server: starting init");
    LOG_DEBUG("bf_dev_name is %s", bf_dev_name);

    snapi_server_params.message_length = ARGMESSAGE_RECEIVE_BUFFER_BYTES;
    snapi_server_params.queue_size = 32;
    snapi_mq_status_t status;
    LOG_DEBUG("Snapi server: calling snapi_mq_create");
    status = snapi_mq_create(bf_dev_name, &snapi_server_params, &snapi_server_mq);
    if (status != SNAPI_MQ_OK) {
        LOG_ERROR("snapi_mq_create failed with err %s",
                  snapi_mq_status_str(status));
    }
    // listen until connected
    LOG_INFO("Snapi server: Listening for client connections");
    do {
        status = snapi_mq_listen(&snapi_server_mq);
    } while (status == SNAPI_MQ_NO_CONNECTIONS);
    if (status == SNAPI_MQ_CONNECTION_ERROR) {
        LOG_ERROR("Got error: %s", snapi_mq_status_str(status));
        snapi_mq_close(&snapi_server_mq);
    }
    LOG_INFO("Snapi server: successfully connected to client");
// END from test.c of Snapi example basic_server_test

//TODO  allow for what engine->myrank and engine->ranks will be
// let's try for Snapi server - rank 0, client - rank 1
// will need to agree with server determination in argmessage helpers
    engine->myrank=0;
    engine->ranks=2;
#else
        int mpi_thread_provided = 0;
        MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpi_thread_provided);
        if (mpi_thread_provided != MPI_THREAD_MULTIPLE) {
            printf("MPI_THREAD_MULTIPLE not provided !!!\n");
        }
        // MPI_Init(&argc, &argv);
        wait_for_debugger();
        printf("ARGM SERVER: mpi initialised\n"); fflush(stdout);
        MPI_Comm_rank(MPI_COMM_WORLD, &(engine->myrank));
        MPI_Comm_size(MPI_COMM_WORLD, &(engine->ranks));
#endif /* #if ARGM_WITH_SNAPI == 1 */
        engine->killifnoadaptersisrequested = false;
        engine->waitingforfirstadapter = true;
        engine->liveadapters = 0;
        engine->activethreads = 0;
        engine->gomultithreaded = false;
        memset(&(engine->requestshandledbyproxy), 0, ARGMESSAGE_MAX_RANKS * sizeof(long));
        for (int adapter_idx=0; adapter_idx<ARGMESSAGE_MAX_ADAPTERS_PER_ENGINE; adapter_idx++){
            engine->adapters[adapter_idx].live = false;
            engine->adapters[adapter_idx].myrank = engine->myrank;
            engine->adapters[adapter_idx].proxyrank = adapter_idx / ARGMESSAGE_ADAPTERS_PER_PROXY; // intentional integer division  
            engine->adapters[adapter_idx].requestshandled=0;
            for (int func_idx=0; func_idx<ARGMESSAGE_MAX_FUNCTION_NAMES; func_idx++){
                engine->adapters[adapter_idx].targetfunctions[func_idx] = &argmessage_serverenginenullfunclog;
                engine->adapters[adapter_idx].functioncallers[func_idx] = &packunpack_callbytearray;
                engine->adapters[adapter_idx].funcisvoid[func_idx] = true;
            }
            /* register (possibly overwriting defaults above) internal functions callable by proxy */
            /* all these functions do not send a response to the proxy
             * any that do would need engine->adapters[adapter_idx].functionisvoid[func_idx] = false;
             */ 
            engine->adapters[adapter_idx].targetfunctions[nooperation] = &argmessage_serverenginenoop;
            engine->adapters[adapter_idx].functioncallers[nooperation] = &packunpack_callnoargs;
            engine->adapters[adapter_idx].targetfunctions[enlivenadapter] = &argmessage_serverengineenlivenadapter;
            engine->adapters[adapter_idx].functioncallers[enlivenadapter] = &packunpack_callnoargs;
            engine->adapters[adapter_idx].targetfunctions[killadapter] = (void*) &argmessage_serverenginekilladapter;
            engine->adapters[adapter_idx].functioncallers[killadapter] = &packunpack_callnoargs;
            engine->adapters[adapter_idx].funcisvoid[killadapter] = false;
            engine->adapters[adapter_idx].targetfunctions[killserverrequest] = (void*) &argmessage_serverenginekillserverrequest;
            engine->adapters[adapter_idx].functioncallers[killserverrequest] = &packunpack_callnoargs;
            engine->adapters[adapter_idx].funcisvoid[killserverrequest] = false;
            engine->adapters[adapter_idx].targetfunctions[stdoutmessage] = &argmessage_serverenginestdoutmessage;
            engine->adapters[adapter_idx].functioncallers[stdoutmessage] = &packunpack_callonestring;
            engine->adapters[adapter_idx].targetfunctions[echoenginestatus] = &argmessage_serverengineechoenginestatus;
            engine->adapters[adapter_idx].functioncallers[echoenginestatus] = &packunpack_callnoargs;
            engine->adapters[adapter_idx].targetfunctions[howmanyadapters] = (void*) &argmessage_serversendhowmanyadapters;
            engine->adapters[adapter_idx].functioncallers[howmanyadapters] = &packunpack_callnoargs;
            engine->adapters[adapter_idx].funcisvoid[howmanyadapters] = false;
            engine->adapters[adapter_idx].targetfunctions[restoreserversinglethreaded] = (void*) &argmessage_restoreserversinglethreaded;
            engine->adapters[adapter_idx].functioncallers[restoreserversinglethreaded] = &packunpack_callnoargs;
            engine->adapters[adapter_idx].funcisvoid[restoreserversinglethreaded] = false;
              /* template for some more of these
            engine->adapters[adapter_idx].targetfunctions[] = &;
            engine->adapters[adapter_idx].functioncallers[] = &;
            engine->adapters[adapter_idx].functionisvoid[] = false;
            */
        }
        // printf("S: calling serverrank...\n");
        // fflush(stdout);

        /* participate in collective identification of server rank - this calls MPI*/


#if ARGM_WITH_SNAPI == 1
        int servercount=1; /* for an an out parameter */
        int foundserverrank = 0; // fixed for Snapi argmessage_serverrank(true , &servercount);
#else
        int servercount=0; /* for an an out parameter */
        int foundserverrank = argmessage_serverrank(true , &servercount);
#endif /* ARGM_WITH_SNAPI == 1 */         
        if (!foundserverrank){
            printf("ARGMESSAGE SERVER (rank %d):  ERROR: No serverrank found - rank %d\n", 
               engine->myrank, engine->myrank);
            fflush(stdout);
        }
        if (servercount != 1){
            printf("ARGMESSAGE SERVER (rank %d):  ERROR: Server count != 1 - rank %d\n", 
               engine->myrank, engine->myrank);
            fflush(stdout);
        }
#ifdef ARGM_SHOW_TRACE        
        printf("ARGMESSAGE SERVER: returned from serverrank: foundserverrank=%d, servercount=%d\n", 
               foundserverrank, servercount);
        fflush(stdout);
        if (servercount==0){
            printf("ARGMESSAGE SERVER (rank %d):  ERROR: No serverrank found but proceeding with me - rank %d\n", 
                engine->myrank, engine->myrank);
            fflush(stdout);
        }
        else{
            if (servercount>1){
                printf("ARGMESSAGE SERVER (rank %d): Multiple servers found - proceeding with rank %d\n", 
                    engine->myrank, engine->myrank);
                fflush(stdout);
            } 
            else{
                printf("ARGMESSAGE SERVER (rank %d): Server found at rank %d\n", 
                    engine->myrank, foundserverrank);
                if (foundserverrank==engine->myrank){
                    printf("*** which is nice because the found server rank equals mine!\n");
                }
                else{
                    printf("ARGMESSAGE SERVER (rank %d):  ERROR: found server rank is not mine!!\n",
                        engine->myrank);
                }
                fflush(stdout);
            }            
        }
#endif /* ARGM_SHOW_TRACE */
    }
    //printf("end of engine creation if\n"); fflush(stdout);
    engine->terminate = false;
    return engine;
}

void argmessage_serverengineregisterfunction(int adapter_idx, int func_idx, void (*targetfunc)(), void (*callerfunc)(), bool funcisvoid){
            engine->adapters[adapter_idx].targetfunctions[func_idx] = targetfunc;
            engine->adapters[adapter_idx].functioncallers[func_idx] = callerfunc;
            engine->adapters[adapter_idx].funcisvoid[func_idx] = funcisvoid;
}

void argmessage_gomultithreaded(){
    engine->gomultithreaded = true;
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM SERVER: argmessage_gomultithreaded: engine->gomultithreaded is now %d\n", engine->gomultithreaded);
    fflush(stdout);
    #endif
}


/* not for use in snapi version - TODO user proof exclusionof that */
void argmessage_checkandconsumemessage(int tag){
    #ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE SERVER: probing for message from proxy (with tag %d)...\n", tag);
        fflush(stdout);
    #endif /* ARGM_SHOW_TRACE */
    
    /* probe for message */
    MPI_Status probe_status;
    int flag; // will be set to true if message is available
    MPI_Message waiting_message;
    // int MPI_Improbe(int source, int tag, MPI_Comm comm, int *flag, MPI_Message *message, MPI_Status *status)
    MPI_Improbe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &flag, &waiting_message, &probe_status);
    if (flag){ // if not _checkandconsumemessage takes no action 
        /* receive a request */
        char* buffer = malloc(ARGMESSAGE_RECEIVE_BUFFER_BYTES);
        MPI_Status status;
        #ifdef ARGM_SHOW_TRACE
        printf( "Server - Receving probed message, with tag %d, on thread %d \n", tag, omp_get_thread_num());
        fflush(stdout);
        #endif
        // int MPI_Mrecv(void *buf, int count, MPI_Datatype type, MPI_Message *message, MPI_Status *status)
        MPI_Mrecv(buffer, ARGMESSAGE_RECEIVE_BUFFER_BYTES, MPI_BYTE, &waiting_message, &status);
        // MPI_Recv(buffer, ARGMESSAGE_RECEIVE_BUFFER_BYTES, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        #ifdef ARGM_SHOW_TRACE
        printf( "Server - MPI recv status: cancelled %d, error %d, thread %d\n", status._cancelled, status.MPI_ERROR, omp_get_thread_num());
        fflush(stdout);
        #endif

        // TODO factor out this section of acting on message and sending any reply to new fn and use that it plain _consume message also
        //int fromrank = status.MPI_SOURCE;
        /* dispatch request to relevant function */
        /* message is obtained by recasting the MPI buffer */
        struct argmessage_message* message = (struct argmessage_message*) buffer;  
#ifdef ARGM_SHOW_TRACE        
        printf("Server - RECEVIED message proxyid=%d, functionid=%d, from thread id=%d, on thread %d\n", message->proxyid, message->functionid, message->threadnum, omp_get_thread_num());
        fflush(stdout);
        printf("ARGMESSAGE SERVER: raw message received:\n");
        argmessage_printbuffer(message);
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        //message->argsbufferstart = ((char*) message) + message->messagesize - message->argsbuffersize; 
        
        // /* FORNOW: debugging aid - write from end of message to end of buffer with '~' */
        // for (char* address = ((char*) message) + message->messagesize; address < ((char*) message) + ARGMESSAGE_RECEIVE_BUFFER_BYTES; address++) *address = '~'; 
        
        // printf("ARGMESSAGE SERVER; adjusted message received:\n");
        // argmessage_printbuffer(message);
        // fflush(stdout);
        int func_idx = message->functionid;//status.MPI_TAG;
        int sent_adapter_idx = message->adapterid;
        int using_adapter_idx;
        if (sent_adapter_idx == -1) using_adapter_idx = message->proxyid * ARGMESSAGE_ADAPTERS_PER_PROXY; 
        else using_adapter_idx = sent_adapter_idx;
 
#ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE SERVER: message received from %d, for adapter %d with tag %d\n", message->proxyid, sent_adapter_idx, func_idx);
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        message->adapterid = using_adapter_idx;
        /* check whether this function type returns values or not and call function in the relevant manner */
        if (engine->adapters[using_adapter_idx].funcisvoid[func_idx]){
            // FORLATER: prehaps check here that the adpater is live - if not live do what?
            engine->adapters[using_adapter_idx].functioncallers[func_idx](using_adapter_idx, engine->adapters[using_adapter_idx].targetfunctions[func_idx], buffer);
        }
        else{
#ifdef ARGM_SHOW_TRACE
            printf("ARGMESSAGE SERVER: getting a reference to the user function to call\n");
            fflush(stdout);
#endif
            struct argmessage_message* (*callerwithresult)() = (struct argmessage_message* (*)()) engine->adapters[using_adapter_idx].functioncallers[func_idx];
#ifdef ARGM_SHOW_TRACE
            printf("ARGMESSAGE SERVER: calling the user function\n");
            fflush(stdout);
#endif
            struct argmessage_message* reply = callerwithresult(using_adapter_idx, engine->adapters[using_adapter_idx].targetfunctions[func_idx], buffer);
#ifdef ARGM_SHOW_TRACE
            printf("ARGMESSAGE SERVER: adding some accounting info to the reply message\n");
            fflush(stdout);
#endif
            reply->proxyid = message->proxyid;
            reply->functionid = message->functionid;
            reply->adapterid = message->adapterid;
            reply->threadnum = message->threadnum;
            #ifdef ARGM_SHOW_TRACE
            printf("ARGM SERVER: replying with funcid %d for thread %d\n", message->functionid, message->threadnum);
            fflush(stdout);
            #endif
            reply->proxysequencenumber = message->proxysequencenumber;
#ifdef ARGM_SHOW_TRACE
            printf("ARGMESSAGE SERVER: sending result back ...\n");
            argmessage_printbuffer(reply);
            fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
            #ifdef ARGM_SHOW_TRACE
            printf( "Server - Sending message with proxyid = %d, functionid = %d, to thread=%d, on thread %d \n", reply->proxyid, reply->functionid, message->threadnum, omp_get_thread_num());
            fflush(stdout);
            #endif
            MPI_Send((void*) reply, reply->messagesize, MPI_BYTE, message->proxyid, ARGM_THREAD_TAG_BASE + message->threadnum, MPI_COMM_WORLD);
            #ifdef ARGM_SHOW_TRACE
            printf( "Server - SENT    message with proxyid = %d, functionid = %d, to thread=%d, on thread %d\n", reply->proxyid, reply->functionid, message->threadnum, omp_get_thread_num());
            fflush(stdout);
            #endif

            free(reply); /* TODO how soon can this be done with Snapi? */
        }
        /* end of loop increment and loop termination logic */
        // THREAD SAFETY:  these need to be atomic increments 
        atomic_inc(&engine->requestshandled);
        atomic_inc(&engine->requestshandledbyproxy[message->proxyid]);

#ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE SERVER: count of requests now handled=%ld\n",engine->requestshandled);
        fflush(stdout);
        printf("ARGMESSAGE SERVER: checking message sequence number ... ");
        if (engine->requestshandledbyproxy[message->proxyid] == message->proxysequencenumber){
            printf("sequence number OK!\n");
        }
        else {
            printf("ERROR: sequence number unmatching!! received=%d, local=%d\n", message->proxysequencenumber, engine->adapters[using_adapter_idx].requestshandled);
        }
        printf("ARGMESSAGE SERVER: ***** end of main loop ******\n\n");
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        free(buffer); // this message has now been consumed (inc any reply sent)
        // and loop for next message


        // printf("maxrequests=%ld, requestshandled=%ld, killengine=%d, liveadapters=%d, waitingforfirst=%d\n",
        //     maxrequests, engine->requestshandled, (int)engine->killifnoadaptersisrequested,
        //     engine->liveadapters, (int)engine->waitingforfirstadapter);
        // printf("((maxrequests>0) && (engine->requestshandled>=maxrequests)=%d\n",(int)((maxrequests>0) && (engine->requestshandled>=maxrequests)));    
        // printf("(engine->killifnoadaptersisrequested && (engine->liveadapters==0))=%d\n",(int)(engine->killifnoadaptersisrequested && (engine->liveadapters==0)));
        // printf("engine->waitingforfirstadapter=%d\n",(int)engine->waitingforfirstadapter);
        int maxrequests = engine->maxrequests;
        int terminate = ((maxrequests>0) && (engine->requestshandled>=maxrequests));
        terminate |= (engine->killifnoadaptersisrequested && (engine->liveadapters==0)); 
        terminate &= !engine->waitingforfirstadapter;
        // printf("terminate=%d\n",(int)terminate);
        // fflush(stdout);
        // THREAD SAFETY: action of all threads would be to set this from false to true - no lock needed - but must avoid keep rewriting to false
        if (terminate) engine->terminate = terminate;
        //fprintf(stderr, "engine->terminate=%d\n", terminate);
    }
}

void argmessage_consumemessage(){
    #ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE SERVER: waiting for message from proxy ...\n");
        fflush(stdout);
    #endif /* ARGM_SHOW_TRACE */
 
        /* receive a request */
        char* buffer = malloc(ARGMESSAGE_RECEIVE_BUFFER_BYTES);
#if ARGM_WITH_SNAPI == 1
        LOG_DEBUG("Snapi server: calling snapi_mq_recv");
        size_t snapi_recv_len;
        while (snapi_mq_recv(&snapi_server_mq, buffer, &snapi_recv_len) != SNAPI_MQ_OK)
            ;
        LOG_DEBUG("Snapi server: received message of size %ld", snapi_recv_len);
#else
        MPI_Status status;
        #ifdef ARGM_SHOW_TRACE
        printf( "Server - Receving message on thread %d \n", omp_get_thread_num());
        fflush(stdout);
        #endif
        MPI_Recv(buffer, ARGMESSAGE_RECEIVE_BUFFER_BYTES, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        #ifdef ARGM_SHOW_TRACE
        printf( "Server - MPI recv status: cancelled %d, error %d, thread %d\n", status._cancelled, status.MPI_ERROR, omp_get_thread_num());
        fflush(stdout);
        #endif
        //int fromrank = status.MPI_SOURCE;
        /* dispatch request to relevant function */
        /* message is obtained by recasting the MPI buffer */
#endif /* ARGM_WITH_SNAPI == 1 */
        struct argmessage_message* message = (struct argmessage_message*) buffer;  
        #ifdef ARGM_SHOW_TRACE
        printf("Server - RECEVIED message proxyid=%d, functionid=%d, from thread id=%d, on thread %d\n", message->proxyid, message->functionid, message->threadnum, omp_get_thread_num());
        fflush(stdout);
        #endif
#ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE SERVER: raw message received:\n");
        argmessage_printbuffer(message);
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        //message->argsbufferstart = ((char*) message) + message->messagesize - message->argsbuffersize; 
        
        // /* FORNOW: debugging aid - write from end of message to end of buffer with '~' */
        // for (char* address = ((char*) message) + message->messagesize; address < ((char*) message) + ARGMESSAGE_RECEIVE_BUFFER_BYTES; address++) *address = '~'; 
        
        // printf("ARGMESSAGE SERVER; adjusted message received:\n");
        // argmessage_printbuffer(message);
        // fflush(stdout);
        int func_idx = message->functionid;//status.MPI_TAG;
        int sent_adapter_idx = message->adapterid;
        int using_adapter_idx;
        if (sent_adapter_idx == -1) using_adapter_idx = message->proxyid * ARGMESSAGE_ADAPTERS_PER_PROXY; 
        else using_adapter_idx = sent_adapter_idx;
 
#ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE SERVER: message received from %d, for adapter %d with tag %d\n", message->proxyid, sent_adapter_idx, func_idx);
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        message->adapterid = using_adapter_idx;
        /* check whether this function type returns values or not and call function in the relevant manner */
        if (engine->adapters[using_adapter_idx].funcisvoid[func_idx]){
            // FORLATER: prehaps check here that the adpater is live - if not live do what?
            engine->adapters[using_adapter_idx].functioncallers[func_idx](using_adapter_idx, engine->adapters[using_adapter_idx].targetfunctions[func_idx], buffer);
        }
        else{
#ifdef ARGM_SHOW_TRACE
            printf("ARGMESSAGE SERVER: getting a reference to the user function to call\n");
            fflush(stdout);
#endif
            struct argmessage_message* (*callerwithresult)() = (struct argmessage_message* (*)()) engine->adapters[using_adapter_idx].functioncallers[func_idx];
#ifdef ARGM_SHOW_TRACE
            printf("ARGMESSAGE SERVER: calling the user function\n");
            fflush(stdout);
#endif
            struct argmessage_message* reply = callerwithresult(using_adapter_idx, engine->adapters[using_adapter_idx].targetfunctions[func_idx], buffer);
#ifdef ARGM_SHOW_TRACE
            printf("ARGMESSAGE SERVER: adding some accounting info to the reply message\n");
            fflush(stdout);
#endif
            reply->proxyid = message->proxyid;
            reply->functionid = message->functionid;
            reply->adapterid = message->adapterid;
            reply->threadnum = message->threadnum;
            #ifdef ARGM_SHOW_TRACE
            printf("ARGM SERVER: replying with funcid %d for thread %d\n", message->functionid, message->threadnum);
            fflush(stdout);
            #endif
            reply->proxysequencenumber = message->proxysequencenumber;
#ifdef ARGM_SHOW_TRACE
            printf("ARGMESSAGE SERVER: sending result back ...\n");
            argmessage_printbuffer(reply);
            fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
#if ARGM_WITH_SNAPI == 1
        LOG_DEBUG("Snapi server: calling snapi_mq_send");
        snapi_mq_status_t status;
        status = snapi_mq_send(&snapi_server_mq, reply, reply->messagesize);
        if (status != SNAPI_MQ_OK) {
            LOG_ERROR("Snapi_mq_send failed with status %s",
                    snapi_mq_status_str(status));
        }


#else
            #ifdef ARGM_SHOW_TRACE
            printf( "Server - Sending message with proxyid = %d, functionid = %d, to thread=%d, on thread %d \n", reply->proxyid, reply->functionid, message->threadnum, omp_get_thread_num());
            fflush(stdout);
            #endif
            MPI_Send((void*) reply, reply->messagesize, MPI_BYTE, message->proxyid, ARGM_THREAD_TAG_BASE + message->threadnum, MPI_COMM_WORLD);
            #ifdef ARGM_SHOW_TRACE
            printf( "Server - SENT    message with proxyid = %d, functionid = %d, to thread=%d, on thread %d\n", reply->proxyid, reply->functionid, message->threadnum, omp_get_thread_num());
            fflush(stdout);
            #endif

#endif /* #if ARGM_WITH_SNAPI == 1 */     
            free(reply); /* TODO how soon can this be done with Snapi? */
        }
        /* end of loop increment and loop termination logic */
        // THREAD SAFETY:  these need to be atomic increments 
        atomic_inc(&engine->requestshandled);
        atomic_inc(&engine->requestshandledbyproxy[message->proxyid]);

#ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE SERVER: count of requests now handled=%ld\n",engine->requestshandled);
        fflush(stdout);
        printf("ARGMESSAGE SERVER: checking message sequence number ... ");
        if (engine->requestshandledbyproxy[message->proxyid] == message->proxysequencenumber){
            printf("sequence number OK!\n");
        }
        else {
            printf("ERROR: sequence number unmatching!! received=%d, local=%d\n", message->proxysequencenumber, engine->adapters[using_adapter_idx].requestshandled);
        }
        printf("ARGMESSAGE SERVER: ***** end of main loop ******\n\n");
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        free(buffer); // this message has now been consumed (inc any reply sent)
        // and loop for next message


        // printf("maxrequests=%ld, requestshandled=%ld, killengine=%d, liveadapters=%d, waitingforfirst=%d\n",
        //     maxrequests, engine->requestshandled, (int)engine->killifnoadaptersisrequested,
        //     engine->liveadapters, (int)engine->waitingforfirstadapter);
        // printf("((maxrequests>0) && (engine->requestshandled>=maxrequests)=%d\n",(int)((maxrequests>0) && (engine->requestshandled>=maxrequests)));    
        // printf("(engine->killifnoadaptersisrequested && (engine->liveadapters==0))=%d\n",(int)(engine->killifnoadaptersisrequested && (engine->liveadapters==0)));
        // printf("engine->waitingforfirstadapter=%d\n",(int)engine->waitingforfirstadapter);
        int maxrequests = engine->maxrequests;
        int terminate = ((maxrequests>0) && (engine->requestshandled>=maxrequests));
        terminate |= (engine->killifnoadaptersisrequested && (engine->liveadapters==0)); 
        terminate &= !engine->waitingforfirstadapter;
        // printf("terminate=%d\n",(int)terminate);
        // fflush(stdout);
        // THREAD SAFETY: action of all threads would be to set this from false to true - no lock needed - but must avoid keep rewriting to false
        if (terminate) engine->terminate = terminate;
        //fprintf(stderr, "engine->terminate=%d\n", terminate);
}


// LATER: add max duration in seconds
// LATER: background so foreground can kill, threads for differenc adapter instances
int argmessage_serverenginerun(long maxrequests){
    if (engine==NULL){
        /* this is an error - client should get a refernce to the client first */
        return 1;
    }
    //LATER: this buffer could be freshly allocated for each receive - so moved below
    // which is related to out of order execution
    // wrong! buffer is an output parameter of MPI_Recv - hence comment our of malloc
    // TODO: or perhaps not!!
    //char* buffer = NULL; // = malloc(ARGMESSAGE_RECEIVE_BUFFER_BYTES);

    // engine->maxrequests = maxrequests;
    //bool terminate = false;
    engine->gomultithreaded = false;
    /* start with single thead and then do multithreaded when signalled to do so */
    while (!engine->terminate) {  // repeat single then multihtreaded message consumption

    #ifdef ARGM_SHOW_TRACE
    printf("ARGM SERVER: BEGIN: consuming messages on a single thread (thread %d), engine->gomultithreaded is %d\n ", omp_get_thread_num(), engine->gomultithreaded);
    fflush(stdout);
    #endif


    while (!engine->gomultithreaded && !engine->terminate){
        #ifdef ARGM_SHOW_TRACE
        printf("ARGM SERVER: single threaded loop BEFORE consume message - engine->gomultithreaded is %d\n", engine->gomultithreaded);
        fflush(stdout);
        #endif
        argmessage_consumemessage();
        #ifdef ARGM_SHOW_TRACE
        printf("ARGM SERVER: single threaded loop AFTER  consume message - engine->gomultithreaded is %d\n", engine->gomultithreaded);
        fflush(stdout);
        #endif
    }
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM SERVER:   END: consuming messages on a single thread (thread %d)\n", omp_get_thread_num());
    fflush(stdout);
    #endif

    #pragma omp parallel
    {
    atomic_inc(&engine->activethreads);
    #ifdef ARGM_SHOW_TRACE
    int thread_num = omp_get_thread_num();  // this var is private to the thread
    if (thread_num == 0) printf("ARGM SERVER: thread 0 reports: number of threads is %d\n", omp_get_num_threads());
    printf("ARGM SERVER: BEGIN: consuming messages in parallel on omp thread: %d\n", thread_num);
    fflush(stdout);    
    #endif
    while (engine->gomultithreaded && !engine->terminate){
        #ifdef ARGM_SHOW_TRACE
        printf("ARGM SERVER: multi threaded loop BEFORE consume message, on thread %d - engine->gomultithreaded is %d\n", thread_num, engine->gomultithreaded);
        fflush(stdout);
        #endif
        argmessage_consumemessage();
        #ifdef ARGM_SHOW_TRACE
        printf("ARGM SERVER: multi threaded loop AFTER  consume message, on thread %d - engine->gomultithreaded is %d\n", thread_num, engine->gomultithreaded);
        fflush(stdout);
        #endif
    }
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM SERVER:   END: consuming messages in parallel on omp thread: %d\n", thread_num);
    fflush(stdout);
    #endif
    atomic_dec(&engine->activethreads);
    }

    } // END repeat single then multihtreaded message consumption
    // LATER: if there is a return channel notify proxies
    
    /* leave tidy up to client otherwise */
    // see corrected mistake above - buffer is an out paramter of MPI_Recv
    // so now above each message is freed once it is consumed 
    // and buffer is a duplicate pointer to the last message and 
    // so must not be freed (i.e. the memory would be freed for a second time)
    //free(buffer);
    return 0;
}

int argmessage_serverenginefree(){
        /* FORNOW: close down MPI here - anywhere else better?*/
#if ARGM_WITH_SNAPI == 1
    LOG_DEBUG("Snapi server: calling snapi_mq_close");
    snapi_mq_close(&snapi_server_mq);
#else
        
        printf("ARGM SERVER - finalizing MPI - will wait at barrier first\n");
        fflush(stdout);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();   
        printf("ARGM SERVER - MPI Finalized\n");
        fflush(stdout);

#endif /* ARGM_WITH_SNAPI == 1 */
    if (engine==NULL){
        /* this is an error - client should get a refernce to the serverengine first */
        return 1;
    }
    /* free any dependent resources first */

    /* and then free the engine itself */
    free(engine); //client's pointer to this is now out of date!
    engine = NULL; // this is the static pointer to the singleton serverengine
    return 0;
}


/* BEGIN: internal functions callable from proxy 
 * beginning with defaults for those
*/

void argmessage_serverenginenullfuncerror(int adapter_idx){
//TODO how to raise an error in C !! - FORNOW: a printstatement
#ifdef ARGM_SHOW_TRACE
    printf("Call made to function index for which no function is registered! (from adapter: %d)", adapter_idx);
#endif /* ARGM_SHOW_TRACE */
    return;
}

void argmessage_serverenginenullfunclog(int adapter_idx){
    //TODO - intialise the function table, all entries, with pointers to this func 
#ifdef ARGM_SHOW_TRACE
    printf("Call made to function index for which no function is registered! (from ADAPTER/MPIRANK: %d)", adapter_idx);
#endif /* ARGM_SHOW_TRACE */
    return;
}

// THREAD SAEFTY: nothing being done here - no changes needed
void argmessage_serverenginenoop(int adapter_idx){
     /* nothing here because it is a no operation
     - useful for testing
     */ 
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM SERVER: doing a noop\n");
    fflush(stdout);
    #endif
    return;
}

// THREAD SAEFTY: this does update common values - but should be no competition to update 1sta nd 3rd items - for 2nd item : only using 1 adapter so far 
void argmessage_serverengineenlivenadapter(int adapter_idx){
    if (!engine->adapters[adapter_idx].live){
#ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE SERVER: enlivening adpater %d\n", adapter_idx);
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        engine->adapters[adapter_idx].live = true;
        engine->liveadapters++;
        engine->waitingforfirstadapter = false;
    }
    return;
}

// THREAD SAFETY see next function above for comments 
struct argmessage_message* argmessage_serverenginekilladapter(int adapter_idx){
    if (engine->adapters[adapter_idx].live){
#ifdef ARGM_SHOW_TRACE
        printf("ARGMESSAGE SERVER: killing adpater %d on thread %d\n", adapter_idx, omp_get_thread_num());
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        engine->adapters[adapter_idx].live = false;
        engine->liveadapters--;
    }
    // send back the number of noops to send to allow listening threads to terminate - if there are no more adapters to service 
    if (engine->liveadapters == 0){
        #ifdef ARGM_SHOW_TRACE
        printf("ARGM SERVER: kill adapter: returning number of active threads: %d (would be active in server run)\n", engine->activethreads);
        fflush(stdout);
        #endif
        return packunpack_packoneint(engine->activethreads);  //  needs (?) an adjustment e.g. because this thread is going to terminate anyway - do at client end
    }
    else
    {
        #ifdef ARGM_SHOW_TRACE
        printf("ARGM SERVER: kill adapter: returning number of active threads: as 0 because there are still live adapters (would be active in server run)\n");
        fflush(stdout);
        #endif
        return packunpack_packoneint(0);
    } 
}

struct argmessage_message* argmessage_restoreserversinglethreaded(int adapter_idx){
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM SERVER: argmessage_restoreserversinglethreaded: returning number of active threads, on thread %d: %d (would be active in server run)\n", omp_get_thread_num(), engine->activethreads);
    fflush(stdout);
    printf("ARGM SERVER: argmessage_restoreserversinglethreaded: setting engine->gomultithreaded = false, on thread %d: %d (would be active in server run)\n", omp_get_thread_num(), engine->activethreads);
    fflush(stdout);
    #endif
    engine->gomultithreaded = false;
    #ifdef ARGM_SHOW_TRACE
    printf("ARGM SERVER: argmessage_restoreserversinglethreaded: engine->gomultithreaded = %d\n", engine->gomultithreaded);
    fflush(stdout);
    #endif
    return packunpack_packoneint(engine->activethreads);  //  needs (?) an adjustment e.g. because this thread is going to terminate anyway - do at client end
}


// THREAD SAFETY - same comments as preceding two functions
struct argmessage_message* argmessage_serverenginekillserverrequest(int adapter_idx){
#ifdef ARGM_SHOW_TRACE
    printf("ARGMESSAGE SERVER: server kill requst from %d\n", adapter_idx);
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    engine->killifnoadaptersisrequested = true;
    return packunpack_packoneint(0);
}

// THREAD SAFETY: two threads could attempt to print to stdout at the same time - what would happen?
void argmessage_serverenginestdoutmessage(int adapter_idx, int string0len, char* string0){

    printf("AGRMESSAGE SERVER: ADAPTER/MPIRANK %d says: %s\n", adapter_idx, string0);
    fflush(stdout);

    return;
}

// THREAD SAFETY: two threads could attempt to print to stdout at the same time - what would happen?
void argmessage_serverengineechoenginestatus(int adapter_idx){

#ifdef ARGM_SHOW_TRACE
    printf("------------------------------------------\n");
    printf("         ARGMESSAGE SERVER STATUS\n");
    printf("Request to print this came from adapter #%d\n", adapter_idx);
    if (adapter_idx / ARGMESSAGE_ADAPTERS_PER_PROXY == engine->myrank){
        printf("*** which in this case is the server!\n");
    }
    printf("Values from the engine struct:\n");
    printf("MPI Rank               : %d\n", engine->myrank);
    printf("MPI ranks count        : %d\n", engine->ranks);
    printf("Count of live adpaters : %d\n", engine->liveadapters);
    printf("Requests limit         : %ld\n", engine->maxrequests);
    printf("Requests so far        : %ld\n", engine->requestshandled);
    printf("------------------------------------------\n\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */

    //return;
}

//THREAD SAFETY - this information could become out of date even before it arrives at the cleint - but only 1 adapter being used at present.
struct argmessage_message* argmessage_serversendhowmanyadapters(int adapter_idx){
    return packunpack_packoneint(engine->liveadapters);
}

/* TODO:
    verbosityfilteredstdoutmessage,
    echofunctionnames  --> the names are not yet anywhere in the program
*/

/* END: internal functions callable from proxy */

/* END: server engine section */

/* BEGIN: adapter section */

/*
void argmessage_adapter_registertargetfunc(){

}
*/


/* TODO:
    nooperation = ARGMESSAGE_ADAPTER_FUNCTIONS_BASE,
    setunknownfunctionnumberbehaviour,
    startlog,
    logmessage,
    closelog
*/


/* END: Adapter section */