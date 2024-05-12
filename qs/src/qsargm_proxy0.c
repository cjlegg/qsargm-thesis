/* Config parameters. */
#include "config.h"

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* OpenMP headers, only if available. */
//#ifdef HAVE_OPENMP  // TODO ? make or is defined TIMERS
    #include <omp.h>
//#endif

// /* Pthread headers, only if available. */
#ifdef HAVE_PTHREAD
    #include <pthread.h>
#endif



#include <string.h>

#include "quicksched_local.h"
#include "argmessagesizes.h"
#include "argmessageinternalhelpers.h"
#include "argmessage.h"
#include "argmessageproxy.h"
#include "packunpack.h"
#include "qsargm_common0.h"
#include "qsargm_proxy0.h"
#include "waitfordebugger.h"

struct qsched* proxy0sched;
extern struct argmessage_proxy* proxy;
/* Strategy 0 entry points for qsched external functions. */
void qsargmproxy0_init( int adapter_idx, int nr_queues , int flags ){
    
    proxy0sched = (struct qsched*)malloc(sizeof(*proxy0sched));

    /* Set the flags to begin with. */
    proxy0sched->flags = flags;
    
    /* Allocate and clear the queues (will init when sched is
       finalized. */
    // queues are going to be on server
    //if ( ( s->queues = (struct queue *)malloc( sizeof(struct queue) * nr_queues ) ) == NULL )
    //    error( "Failed to allocate memory for queues." );
    //bzero( s->queues , sizeof(struct queue) * nr_queues );
    
    // this not commented out - still need to know the number of queues on the client side
    // used in _run_openmp // s renamed proxy0sched
    proxy0sched->nr_queues = nr_queues;
    
    /* Allocate the task list. */
    // this is going to be on the server
    //s->size = qsched_size_init;
    //if ( ( s->tasks = (struct task *)malloc( sizeof(struct task) * s->size ) ) == NULL )
    //    error( "Failed to allocate memory for tasks." ); 
    //s->count = 0;
    
    /* Allocate the initial deps. */
    // this is going to be on the server
    //s->size_deps = qsched_init_depspertask * s->size;
    //if ( ( s->deps = (int *)malloc( sizeof(int) * s->size_deps ) ) == NULL ||
    //     ( s->deps_key = (int *)malloc( sizeof(int) * s->size_deps ) ) == NULL )
    //    error( "Failed to allocate memory for deps." );
    //s->count_deps = 0;

    /* Allocate the initial locks. */
    // this is going to be on the server
    //s->size_locks = qsched_init_lockspertask * s->size;
    //if ( ( s->locks = (int *)malloc( sizeof(int) * s->size_locks ) ) == NULL ||
    //     ( s->locks_key = (int *)malloc( sizeof(int) * s->size_locks ) ) == NULL )
    //    error( "Failed to allocate memory for locks." );
    //s->count_locks = 0;
    
    /* Allocate the initial res. */
    // this is going to be on the server
    //s->size_res = qsched_init_respertask * s->size;
    //if ( ( s->res = (struct res *)malloc( sizeof(struct res) * s->size_res ) ) == NULL )
    //    error( "Failed to allocate memory for res." );
    //s->count_res = 0;
    
    /* Allocate the initial uses. */
    // this is going to be on the server
    //s->size_uses = qsched_init_usespertask * s->size;
    //if ( ( s->uses = (int *)malloc( sizeof(int) * s->size_uses ) ) == NULL ||
    //     ( s->uses_key = (int *)malloc( sizeof(int) * s->size_uses ) ) == NULL )
    //    error( "Failed to allocate memory for uses." );
    //s->count_uses = 0;
    
    /* Allocate the initial data. */
    // this is going to be on the server
    //s->size_data = qsched_init_datapertask * s->size;
    //if ( ( s->data = malloc( s->size_data ) ) == NULL )
    //    error( "Failed to allocate memory for data." );
    //s->count_data = 0;
    
    /* Init the pthread stuff. */
    // threads remain local
    // QUESTION: which of the s properties are also used on the server - hopefully none

    // all the above is server side
    // so send a message to execute it
    struct argmessage_message* message = packunpack_packtwoint(nr_queues, flags);
    message->functionid = schedinit; 
    message->adapterid = adapter_idx; 
    argmessage_proxysendfunction(message);

    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif

    #ifdef HAVE_PTHREAD
        printf("in HAVE_PTHREAD OF qsargmproxy0_init\n");
        fflush(stdout);
        if ( pthread_cond_init( &proxy0sched->cond , NULL ) != 0 ||
             pthread_mutex_init( &proxy0sched->mutex , NULL ) != 0 )
            error( "Error initializing yield cond/mutex pair." );
        if ( pthread_cond_init( &proxy0sched->barrier_cond , NULL ) != 0 ||
             pthread_mutex_init( &proxy0sched->barrier_mutex , NULL ) != 0 )
            error( "Error initializing barrier cond/mutex pair." );
        proxy0sched->runners_count = 0;
        proxy0sched->runners_size = qsched_init_runners;
        if ( ( proxy0sched->runners = malloc( sizeof(struct qsched_pthread_runner) * proxy0sched->runners_size ) ) == NULL )
          error( "Failed to allocate runners." );
        proxy0sched->barrier_running = 0;
        proxy0sched->barrier_count = 0;
        proxy0sched->barrier_launchcount = 0;
        if ( pthread_mutex_lock( &proxy0sched->barrier_mutex ) != 0 )
            error( "Failed to lock barrier mutex." );
    #endif
    
    // perhaps on client and server
    /* Clear the timers. */
    #ifdef TIMERS
        bzero( proxy0sched->timers , sizeof(ticks) * qsched_timer_count );
    #endif
    
    // does the client program use members of the struct sched
    // 
    // this is needed on the server
    // but leave it on client for now as well
    /* Init the sched lock. */
    lock_init( &proxy0sched->lock );

    }

qsched_res_t qsargmproxy0_addres( int adapter_idx , int owner , qsched_res_t parent ){
    // all of this function is server side
    struct argmessage_message* message = packunpack_packtwoint(owner, (int) parent);
    message->functionid = addres;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
    // receive reply and return res id
    struct argmessage_message* reply = argmessage_proxyreceivefunctionresult(message);
#ifdef ARGM_SHOW_TRACE
    printf("back in qsargmproxy0_addres\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    qsched_res_t* resindexptr = (qsched_res_t*)(reply->argsbuffer); // new pointer assigned to avoid anti-aliasing error
#ifdef ARGM_SHOW_TRACE
    printf("caluclated resindexptr \n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    qsched_res_t resindex = * resindexptr;
#ifdef ARGM_SHOW_TRACE
    printf("caluclated resenidex=%d \n", resindex);
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    free(reply);
    #endif
#ifdef ARGM_SHOW_TRACE
    printf("freed message \n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    // free(reply); // suspected of crash
    // printf("freed reply \n");
    // fflush(stdout);
    return resindex;
}

void qsargmproxy0_addlock( int adapter_idx, qsched_task_t t , qsched_res_t res ){
    // all of this function is server side
    struct argmessage_message* message = packunpack_packtwoint((int) t , (int) res);
    message->functionid = addlock;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif
}

void qsargmproxy0_addunlock( int adapter_idx , qsched_task_t ta , qsched_task_t tb ){
    // all of this function is server side
    struct argmessage_message* message = packunpack_packtwoint((int) ta , (int) tb);
    message->functionid = addunlock;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif
}

struct argmessage_message* qsargmproxy0_addtask_pack(int type , unsigned int flags , void *data , int data_size , int cost ){
    int argsbuffersize = 4*sizeof(int) + data_size;
    struct argmessage_message* message = argmessage_messagecreate(argsbuffersize);
    char* bytearray = (char*) message->argsbuffer;
    *((int*)bytearray) = type;
    bytearray += sizeof(int);
    *((int*)bytearray) = flags;
    bytearray += sizeof(int);
    *((int*)bytearray) = data_size;
    bytearray += sizeof(int);
    memcpy(bytearray, data, data_size);
    bytearray += data_size;
    *((int*)bytearray) = cost;
    return message;
}

qsched_task_t qsargmproxy0_addtask( int adapter_idx, int type , unsigned int flags , void *data , int data_size , int cost ){
    // all of this function is server side
    struct argmessage_message* message = qsargmproxy0_addtask_pack(type , flags , data , data_size , cost );
    message->functionid = addtask;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
    // receive reply and return task id
    struct argmessage_message* reply = argmessage_proxyreceivefunctionresult(message);
    qsched_task_t* taskidptr = (qsched_task_t*) (reply->argsbuffer); // cast to new pointer to avoid anti-aliasing error
    int taskid = *taskidptr;
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    free(reply);
    #endif
    return taskid;
}

void qsargmproxy0_adduse( int adapter_idx , qsched_task_t t , qsched_res_t res ){
    // all of this function is server side
    struct argmessage_message* message = packunpack_packtwoint((int) t, (int) res);
    message->functionid = adduse;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif
}

void qsargmproxy0_free( int adapter_idx ){
    // call this function on the server side
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = schedfree;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
    // this should clear whatever of sched object exists on the proxy side
    //int k;
#ifdef ARGM_SHOW_TRACE
    printf("qsargmproxy0_free: position 0\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    /* Clear all the buffers if allocated. */
    //TODO: comment out the ones not actually initialised on client side
    //if ( proxy0sched->tasks != NULL ) { free( proxy0sched->tasks ); proxy0sched->tasks = NULL; }
//      if ( proxy0sched->locks != NULL ) { free( proxy0sched->locks ); proxy0sched->locks = NULL; }
// #ifdef ARGM_SHOW_TRACE
//     printf("qsargmproxy0_free: position 4\n");
//     fflush(stdout);
// #endif /* ARGM_SHOW_TRACE */
//     if ( proxy0sched->locks_key != NULL ) { free( proxy0sched->locks_key ); proxy0sched->locks_key = NULL; }
// #ifdef ARGM_SHOW_TRACE
//     printf("qsargmproxy0_free: position 5\n");
//     fflush(stdout);
// #endif /* ARGM_SHOW_TRACE */
//     if ( proxy0sched->uses != NULL ) { free( proxy0sched->uses ); proxy0sched->uses = NULL; }
// #ifdef ARGM_SHOW_TRACE
//     printf("qsargmproxy0_free: position 6\n");
//     fflush(stdout);
// #endif /* ARGM_SHOW_TRACE */
//     if ( proxy0sched->uses_key != NULL ) { free( proxy0sched->uses_key ); proxy0sched->uses_key = NULL; }
// #ifdef ARGM_SHOW_TRACE
//     printf("qsargmproxy0_free: position 7\n");
//     fflush(stdout);
// #endif /* ARGM_SHOW_TRACE */
//     if ( proxy0sched->res != NULL ) { free( (void *)proxy0sched->res ); proxy0sched->res = NULL; }
//     printf("qsargmproxy0_free: position 8\n");
//     fflush(stdout);
//     if ( proxy0sched->data != NULL ) { free( proxy0sched->data ); proxy0sched->data = NULL; }
// #ifdef ARGM_SHOW_TRACE
//     printf("qsargmproxy0_free: position 9\n");
//     fflush(stdout);
// #endif /* ARGM_SHOW_TRACE */
    
    /* Loop over the queues and free them too. */
    //for ( k = 0 ; k < proxy0sched->nr_queues ; k++ )
    //    queue_free( &proxy0sched->queues[k] );
#ifdef ARGM_SHOW_TRACE
    printf("qsargmproxy0_free: position 10\n");
    fflush(stdout);
 #endif /* ARGM_SHOW_TRACE */
//     free( proxy0sched->queues );
// #ifdef ARGM_SHOW_TRACE
//     printf("qsargmproxy0_free: position 11\n");
//     fflush(stdout);
// #endif /* ARGM_SHOW_TRACE */
    proxy0sched->queues = NULL;
    
    /* Destroy the mutex and condition. */
    #ifdef HAVE_PTHREAD
        #ifdef ARGM_SHOW_TRACE
        printf("in HAVE_PTHREAD OF qsargmproxy0_free\n");
        fflush(stdout);
        #endif

        if ( pthread_cond_destroy( &proxy0sched->cond ) != 0 ||
             pthread_mutex_destroy( &proxy0sched->mutex ) != 0 )
            error( "Error destroying pthread cond/mutex pair." );
    #endif
        
    /* Clear the flags. */
    proxy0sched->flags = qsched_flag_none;
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(message);
    #endif
    
    free(proxy0sched);
    }

void qsargmproxy0_run(  int adapter_idx, int nr_threads , qsched_funtype fun ){
#ifdef ARGM_SHOW_TRACE
    printf("entered qsargmproxy0_run \n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */

    /* Force use of pthreads? */
    if ( !HAVE_OPENMP || proxy0sched->flags & ( qsched_flag_yield | qsched_flag_pthread ) ){
#ifdef ARGM_SHOW_TRACE
        printf("calling qsargmproxy0_run_pthread\n");
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        qsargmproxy0_run_pthread( adapter_idx, nr_threads , fun );
    }
    /* Otherwise, default to OpenMP. */
    else if ( HAVE_OPENMP ) {
#ifdef ARGM_SHOW_TRACE
        printf("calling qsargmproxy0_run_openmp\n");
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        qsargmproxy0_run_openmp(  adapter_idx, nr_threads , fun );
    }    
    else
        error( "QuickSched was not compiled with OpenMP or pthreads." );
        
    }

void qsargmproxy0_run_pthread( int adapter_idx, int nr_threads , qsched_funtype fun ) {
#ifdef ARGM_SHOW_TRACE
        printf("in function: qsargmproxy0_run_pthread\n");
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
#if defined( HAVE_PTHREAD )

    /* Prepare the scheduler. */
    //  next has been coverted to remote command
    //qsched_prepare( . );
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = prepare;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);

    /* Make sure we have enough threads. */
    if ( nr_threads > proxy0sched->runners_count ) {
    
      /* Reallocate the threads array? */
      if ( nr_threads > proxy0sched->runners_size ) {
        int runners_size_new = proxy0sched->runners_size;
        while ( runners_size_new < nr_threads )
          runners_size_new *= qsched_stretch;
        struct qsched_pthread_runner *runners_new;
        if ( ( runners_new = malloc( sizeof(struct qsched_pthread_runner) * runners_size_new ) ) == NULL )
          error( "Failed to allocate new thread array." );
        memcpy( runners_new , proxy0sched->runners , sizeof(pthread_t) * proxy0sched->runners_count );
        free( proxy0sched->runners );
        proxy0sched->runners = runners_new;
        proxy0sched->runners_size = runners_size_new;
      }
      
      /* Launch the missing threads. */
      // tid appears here to be "thread index" - so this stays on proxy side (i.e. != "task id")
      // NO - are initial tasks sorted to top - however that can be there and this below here
      // NO AGAIN - definiton of struc runner has tid = thread id 
      for ( int tid = proxy0sched->runners_count ; tid < nr_threads ; tid++ ) {
        proxy0sched->barrier_running += 1;
        proxy0sched->runners[tid].tid = tid;
        proxy0sched->runners[tid].s = proxy0sched;
    // pthread_create is not on the call graph! - because its a standard OS call 
        if ( pthread_create( &proxy0sched->runners[tid].thread , NULL , qsargmproxy0_pthread_run , (void *)&proxy0sched->runners[tid] ) != 0 )
            error( "Failed to create pthread." );
      }
    }
    
    /* Set the runner function. */
    proxy0sched->fun = fun;

    /* Launch the threads. */
    qsched_launch_threads( proxy0sched , nr_threads );
            
#else
    error( "QuickSched was not compiled with pthread support." );
#endif

}

void *qsargmproxy0_pthread_run( void *in ) {
#ifdef ARGM_SHOW_TRACE
  printf("entered qsargmproxy0_pthread_run \n");
  fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
  struct qsched_pthread_runner *r = (struct qsched_pthread_runner *)in;
  struct qsched *s = r->s;
  int tid = r->tid;  // thread id
  // struct task *t; // not used in this implementation

  /* Main loop. */
  while ( 1 ) {
  
    /* Wait at the barrier. */
    qsched_barrier_wait( s , tid );
  
    /* Loop as long as there are tasks. */
    //  _gettask but remotely
    struct argmessage_message* message = packunpack_packoneint(tid); //so using thread id as queue id 
    message->adapterid = proxy->myrank * ARGMESSAGE_ADAPTERS_PER_PROXY; // in this example we are using the zeroth adapter of this proxy (for strategy0)
    message->functionid = gettask;
    argmessage_proxysendfunction(message);
    // now wait for the reply
    // buffer of incoming message is [task type, task data size and data]
#ifdef ARGM_SHOW_TRACE
    printf("proxy0: pthreadrun - waiting for gettask reply\n");
#endif /* ARGM_SHOW_TRACE */

    struct argmessage_message* gettaskreply = argmessage_proxyreceivefunctionresult(message);
#ifdef ARGM_SHOW_TRACE
    printf("proxy0: pthreadrun - reply message to gettask received\n");
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
    //sleep(10);
    // unpack the message
    char* bytearray = gettaskreply->argsbuffer;
    int task_idx = *((int*)bytearray);
    int task_type = *((int*)(bytearray + sizeof(int)));
#ifdef ARGM_SHOW_TRACE
    printf("proxy0: task_type unpacked as %d\n", task_type);
    fflush(stdout);
#endif /* ARGM_SHOW_TRACE */

    //int task_data_size =0; // variable not used in this implementation
    void* data;
    if (task_type != -1){
        //task_data_size = *((int*)(bytearray+2*sizeof(int)));
        data = bytearray + 3*sizeof(int);
    }
    while ( task_type != -1 ) {

        /* Call the user-supplied function on the task with its data. */
#ifdef ARGM_SHOW_TRACE
        printf("proxy0: calling kernel with taskype=%d\n", task_type);
        fflush(stdout);
        printf("in qsargmproxy0_pthread_run - got task #%i, from queue #-1, at time #%.12f\n", task_idx , omp_get_wtick());
        fflush(stdout);
        int* dat = (int*)data;
        printf("task details: {\"task_idx\": %i, \"qid\": %i, \"type\": %i, \"data\": (%i, %i, %i)}\n", \
                task_idx, tid, task_type, dat[0], dat[1], dat[2]);
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */            
        s->fun( task_type , data );
#ifdef ARGM_SHOW_TRACE
        printf("proxy0: returned from kernel \n");
        printf("in qsargmproxy0_pthread_run - done task #%i, from queue #-1, at time #%.12f\n", task_idx , omp_get_wtick());
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        
        /* Mark that task as done. */
        // qsched_done( s , t ) but done remotely
        message = packunpack_packoneint(task_idx);
        message->adapterid = proxy->myrank * ARGMESSAGE_ADAPTERS_PER_PROXY; // in this example we are using the zeroth adapter of this proxy (for strategy0)
        message->functionid = taskdone; 
        argmessage_proxysendfunction(message);
   
        //free(reply);
        } /* loop as long as there are tasks. */

        // Repeat of the section before the while loop
        message = packunpack_packoneint(tid);  //using queue id = thread id
        message->adapterid = proxy->myrank * ARGMESSAGE_ADAPTERS_PER_PROXY; // in this example we are using the zeroth adapter of this proxy (for strategy0)
        message->functionid = gettask;
        argmessage_proxysendfunction(message);
        // and wait for the reply
        struct argmessage_message* gettaskreply2 = argmessage_proxyreceivefunctionresult(message);
        // unpack the message
        bytearray = gettaskreply2->argsbuffer;
        task_idx = *((int*)bytearray);
        task_type = *((int*)(bytearray + sizeof(int)));
        if (task_type != -1){
            //task_data_size = *((int*)(bytearray+sizeof(int))); // not used in this implementation
            data = bytearray + 3*sizeof(int);
        }
        free(gettaskreply);
        free(gettaskreply2);
    } /* main loop. */
    //free(reply);
  }


void qsargmproxy0_run_openmp( int adapter_idx, int nr_threads , qsched_funtype fun ) {
#ifdef ARGM_SHOW_TRACE
  printf("entered qsargmproxy0_run_openmp \n");
  fflush(stdout);
#endif /* ARGM_SHOW_TRACE */

#if defined( HAVE_OPENMP )
#ifdef ARGM_SHOW_TRACE
  printf("qsargmproxy0_run_openmp: entering omp section \n");
  fflush(stdout);
#endif /* ARGM_SHOW_TRACE */

    /* Prepare the scheduler. */
    // this call has been converted to a remote command
    // qsched_prepare( s );
#ifdef TIMER_CLIENT_REMOTE_PREPARE    
    TIMER_TIC
#endif
    struct argmessage_message* prepare_message = packunpack_dummypayload();
    prepare_message->functionid = prepare;
    prepare_message->adapterid = adapter_idx;
    argmessage_proxysendfunction(prepare_message);  // note that this does not expect a reply so is hard to time here - but should not start un
    // recevie a dummy message which says that prepare has finished, so can move on to gettasks.
    //struct argmessage_message* dummyreply = 
    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    struct argmessage_message* prepare_reply = argmessage_proxyreceivefunctionresult(prepare_message);
    free(prepare_message);
    free(prepare_reply);
    #else
    argmessage_proxyreceivefunctionresult(prepare_message);
    free(prepare_message);
    #ifdef ARGM_SHOW_TRACE
    printf("PROXY: result of prepare received\n");
    fflush(stdout);
    #endif
    #endif

    //dummyreply = NULL; // to suppress unused warning from compiler
#ifdef TIMER_CLIENT_REMOTE_PREPARE    
    TIMER_TOC( proxy0sched , qsargm_client_remote_prepare )
#endif

    /* Parallel loop. */
    //#pragma omp parallel num_threads( nr_threads ) 
    // TODO: just 1 thread for now - fix this parallel loop when serial works
    // code elim #3
    #pragma omp parallel num_threads( nr_threads )
    {
        /* Local variable. */
        // struct task *t;
        
        /* Get the ID of the current thread. */
#ifdef ARGM_SHOW_TRACE
        printf("proxy0sched->nr_queues=%i\n", proxy0sched->nr_queues);
        fflush(stdout);
#endif        
        //code elim #3 
        int qid = omp_get_thread_num() % proxy0sched->nr_queues;
        fprintf(stderr,"Thread v qid %d v %d\n", omp_get_thread_num(), qid);
        //int qid =0;
#ifdef ARGM_SHOW_TRACE
        fprintf(stderr, "in _run_openmp - using thread no = %d\n", qid);
        //fflush(stdout);
#endif /* ARGM_SHOW_TRACE */

#ifdef TIMER_CLIENT_REMOTE_GETTASK    
    TIMER_TIC
#endif

        /* Loop as long as there are tasks. */
        //  _gettask but remotely
        struct argmessage_message* task_message = packunpack_packoneint(qid); //so using openmp thread id as queue id 
        task_message->adapterid = adapter_idx; // proxy->myrank * ARGMESSAGE_ADAPTERS_PER_PROXY; // in this example we are using the zeroth adapter of this proxy (for strategy0)
        task_message->functionid = gettask;
        //fprintf(stderr, "Thread(qid) %d : calling gettask\n", qid);
        argmessage_proxysendfunction(task_message);
        atomic_inc(&proxy0sched->count_proxy_gettask_sent);

        // now wait for the reply
        // buffer of incoming message is [task type, task data size and data]
        struct argmessage_message* task_reply = argmessage_proxyreceivefunctionresult(task_message);

#ifdef ARGM_SHOW_TRACE
        printf("in _run_openmp: returned from gettask's _proxyreceivefunctionresult \n");
        fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
        
        // unpack the message
        char* bytearray = task_reply->argsbuffer;
        int task_idx = *((int*)bytearray);
        if (task_idx == -1){
            atomic_inc(&proxy0sched->count_proxy_gettask_recv_terminate);
        } else
        {
            atomic_inc(&proxy0sched->count_proxy_gettask_recv_normal);
        }
        //fprintf(stderr, "Thread(qid) %d : task returned = %d\n", qid, task_idx);
        int task_type = *((int*)(bytearray + sizeof(int)));
        //int task_data_size; // not used in this implementation
        void* data;
#ifdef ARGM_SHOW_TRACE
        printf("_qsargmproxy0_run_openmp: qid %d: remote getttask result taskidx: %i\n", qid, task_idx);
        fflush(stdout);
#endif
        // task_idx==-1 signals a null task on server i.e. none left
        while ( task_idx != -1 ) {

            // task data only is provided by server if task exists, i.e. task_type != -1
            //task_data_size = *((int*)(bytearray+2*sizeof(int))); // not used in this implementation
            data = bytearray + 3*sizeof(int);
            /* Call the user-supplied function on the task with its data. */
#ifdef ARGM_SHOW_TRACE
            printf("in _run_openmp: calling the kernel with task_type=%d, \n", task_type);
            fflush(stdout);
  //          printf("in qsargmproxy0_run_openmp - got task #%i, from queue #%i, at time #%.12f\n", task_idx ,qid, omp_get_wtick());
  //          fflush(stdout);
  //          int* dat = (int*)data;
  /*          printf("task details: {\"task_idx\": %i, \"qid\": %i, \"type\": %i, \"data\": (%i, %i, %i)}\n", \
                    task_idx, qid, task_type, dat[0], dat[1], dat[2]); */
  //          fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
#ifdef TIMER_CLIENT_REMOTE_GETTASK    
    TIMER_TOC( proxy0sched , qsargm_client_remote_gettask )
#endif

#ifdef TIMER_CLIENT_KERNELS
    TIMER_TIC2
#endif
            fun( task_type , data );
#ifdef TIMER_CLIENT_KERNELS    
    TIMER_TOC( proxy0sched , qsargm_client_kernels )
#endif
#ifdef ARGM_SHOW_TRACE
            printf("in _run_openmp: returned from the kernel \n");
            printf("in qsargmproxy0_run_openmp - done task #%i, from queue #%i, at time #%.12f\n", task_idx ,qid, omp_get_wtick());
            fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
            
            /* Mark that task as done. */
            //qsched_done( s , t ); but remotely
#ifdef TIMER_CLIENT_REMOTE_DONE
    TIMER_TIC2
#endif

            #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
            free(task_message);
            free(task_reply);
            #endif

            struct argmessage_message* done_message = packunpack_packoneint(task_idx);
            done_message->adapterid = adapter_idx;  // proxy->myrank * ARGMESSAGE_ADAPTERS_PER_PROXY; // in this example we are using the zeroth adapter of this proxy (for strategy0)
            done_message->functionid = taskdone; 
            //fprintf(stderr, "Thread(qid) %d : task id done = %d\n", qid, task_idx);

#ifdef ARGM_SHOW_TRACE
            printf("in _run_openmp: reporting taskid=%d as done\n", task_idx);
            fflush(stdout);
#endif /* ARGM_SHOW_TRACE */
            argmessage_proxysendfunction(done_message);
            atomic_inc(&proxy0sched->count_proxy_done_sent);
#ifdef TIMER_CLIENT_REMOTE_DONE    
    TIMER_TOC( proxy0sched , qsargm_client_remote_done )
#endif

#ifdef TIMER_CLIENT_REMOTE_GETTASK    
    TIMER_TIC2
#endif

            #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
            free(done_message);
            #endif
            // Repeat of the section before the while loop
            task_message = packunpack_packoneint(qid);  //using queue id = thread id
            task_message->adapterid = adapter_idx; //proxy->myrank * ARGMESSAGE_ADAPTERS_PER_PROXY; // in this example we are using the zeroth adapter of this proxy (for strategy0)
            task_message->functionid = gettask;
            //fprintf(stderr, "Thread(qid) %d : calling gettask\n", qid);
            argmessage_proxysendfunction(task_message);
            atomic_inc(&proxy0sched->count_proxy_gettask_sent);
            // and wait for the reply
            task_reply = argmessage_proxyreceivefunctionresult(task_message);
            // unpack the message
            bytearray = task_reply->argsbuffer;
            task_idx = *((int*)bytearray);
            if (task_idx == -1){
                atomic_inc(&proxy0sched->count_proxy_gettask_recv_terminate);
            } else
            {
                atomic_inc(&proxy0sched->count_proxy_gettask_recv_normal);
            }
            //fprintf(stderr, "Thread(qid) %d : task returned = %d\n", qid, task_idx);
            task_type = *((int*)(bytearray + sizeof(int)));
            //free(reply);
        } /* loop as long as there are tasks. */
    #ifdef ARGM_SHOW_TRACE
    printf("QSARGM PROXY: run thread %d terminating\n", qid);
    fflush(stdout);
    #endif

    #if !defined QSARGM_OPTIM_01 || !defined HAVE_OPENMP
    free(task_message);
    free(task_reply);
    #endif


//TODO - should there be a gettask toc here - to add in the time for no task returned - compare to what original does. 
    } /* parallel loop. */
    #ifdef ARGM_SHOW_TRACE
    printf("QSARGM PROXY: leaving openmp run\n");
    fflush(stdout);
    #endif
#else
    error( "QuickSched was not compiled with OpenMP support." );
#endif

}


void qsargmproxy0_reset( int adapter_idx ){
    // remotely call this function on the server side
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = schedreset;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
 
    // this should clear whatever of sched object exists on the proxy side
    /* Simply clear the counts, leave the buffers allocated. */
    // proxy0sched->count = 0;
    // proxy0sched->waiting = 0;
    // proxy0sched->count_data = 0;
    // proxy0sched->count_deps = 0;
    // proxy0sched->count_locks = 0;
    // proxy0sched->count_uses = 0;
    // proxy0sched->count_res = 0;

    /* Clear the timers. */
    #ifdef TIMERS
        bzero( proxy0sched->timers , sizeof(ticks) * qsched_timer_count );
    #endif
    
}

// LATER: later addtask_dynamic
//void qsargmproxy0_addtask_dynamic( struct qsched *s , int type , unsigned int flags , void *data , int data_size , int cost , qsched_res_t *locks , int nr_locks , qsched_res_t *uses , int nr_uses );
// LATER: void* getdata - if used by runner to retrieve from s - where is the data set - task?


void qsargmproxy0_ensure( int adapter_idx , int nr_tasks , int nr_res , int nr_deps , int nr_locks , int nr_uses , int size_data ){
    // all of this function is server side
    struct argmessage_message* message = packunpack_packsixint(nr_tasks , nr_res , nr_deps , nr_locks , nr_uses , size_data );
    message->functionid = ensure;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
}
void qsargmproxy0_res_own( int adapter_idx , qsched_res_t res , int owner ){
        // all of this function is server side
    struct argmessage_message* message = packunpack_packtwoint(res, owner);
    message->functionid = resown;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
}

void qsargmproxy0_debugdumptasks(int adapter_idx){
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = debugdumptask;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
}


void qsargmproxy0_debugdumpcounters( int adapter_idx ){
    struct argmessage_message* message = packunpack_dummypayload();
    message->functionid = debugdumpcounters;
    message->adapterid = adapter_idx;
    argmessage_proxysendfunction(message);
}