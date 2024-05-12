/* Config parameters. */
#include "config.h"

/* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* OpenMP headers, only if available. */
#ifdef HAVE_OPENMP
    #include <omp.h>
#endif

// /* Pthread headers, only if available. */
#ifdef HAVE_PTHREAD
    #include <pthread.h>
#endif



#include "quicksched_local.h"
#include "argmessagesizes.h"
#include "argmessageinternalhelpers.h"
#include "argmessage.h"
#include "src/packunpack.h"
#include "qsargm_common0.h"
#include "waitfordebugger.h"


struct argmessage_serverengine* argm_engine = NULL;

void qsargm_serversinit0(struct argmessage_serverengine* engine){
    argm_engine = engine;  
    #ifdef ARGM_SHOW_TRACE
    printf("QSARGM ADPATER: in serversinit0, argm_engine->gomultihtreaded %d, engine->gomultihtreaded %d\n", argm_engine->gomultithreaded, engine->gomultithreaded);
    fflush(stdout);
    #endif
    /* only using zeroth adapter of each proxy */
    for (int adapter0_idx=0; adapter0_idx<ARGMESSAGE_MAX_ADAPTERS_PER_ENGINE; adapter0_idx += ARGMESSAGE_ADAPTERS_PER_PROXY){
            struct qsched* state = malloc(sizeof(struct qsched));
            struct qsched* s = (struct qsched*) state;
            state->init_idx = -1;
            s->doevents_callback = &argmessage_consumemessage;
            argm_engine->adapters[adapter0_idx].applicationstate = (void*) state;
        }
    }




/* Strategy 0 entry points for qsched external functions. */
void qsargmadapter0_init ( int adapter_idx , int nr_queues , int flags ){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    /* for a zero based idx of how many times this sched has been used */
    s->init_idx++;
    /* these lines provide a delimiter in std out for prints of timers etc that are per 'repeat'*/
    printf("**********************************************************\n");
    printf("    Qsargm remote scheduler initialisation index = %i\n", s->init_idx);
    printf("**********************************************************\n");
    fflush(stdout);
    /* Set the flags to begin with. */
    // QUESTION: what uses the flags? proxy or server stuff?
    s->flags = flags;
    s->event_count=0U;
    fprintf(stderr, "initial event count:%u\n", s->event_count);
    printf("initial event count:%u\n", s->event_count);
    fflush(stdout);
    
    /* Allocate and clear the queues (will init when sched is
       finalized. */
    // queues are going to be on server
    if ( ( s->queues = (struct queue *)malloc( sizeof(struct queue) * nr_queues ) ) == NULL )
        error( "Failed to allocate memory for queues." );
    bzero( s->queues , sizeof(struct queue) * nr_queues );
    s->nr_queues = nr_queues;
    
    /* Allocate the task list. */
    // this is going to be on the server
    s->size = qsched_size_init;
    if ( ( s->tasks = (struct task *)malloc( sizeof(struct task) * s->size ) ) == NULL )
        error( "Failed to allocate memory for tasks." );
    s->count = 0;
    
    /* Allocate the initial deps. */
    // this is going to be on the server
    s->size_deps = qsched_init_depspertask * s->size;
    if ( ( s->deps = (int *)malloc( sizeof(int) * s->size_deps ) ) == NULL ||
         ( s->deps_key = (int *)malloc( sizeof(int) * s->size_deps ) ) == NULL )
        error( "Failed to allocate memory for deps." );
    s->count_deps = 0;

    /* Allocate the initial locks. */
    // this is going to be on the server
    s->size_locks = qsched_init_lockspertask * s->size;
    if ( ( s->locks = (int *)malloc( sizeof(int) * s->size_locks ) ) == NULL ||
         ( s->locks_key = (int *)malloc( sizeof(int) * s->size_locks ) ) == NULL )
        error( "Failed to allocate memory for locks." );
    s->count_locks = 0;
    
    /* Allocate the initial res. */
    // this is going to be on the server
    s->size_res = qsched_init_respertask * s->size;
    //if ( ( s->res = (struct res *)malloc( sizeof(s->res) * s->size_res ) ) == NULL )
    // sizeof(struct res) to sizeof(s->res) following comppiler complaint - standard correction
    if ( ( s->res = (struct res *)malloc( sizeof(s->res) * s->size_res ) ) == NULL )
        error( "Failed to allocate memory for res." );
    s->count_res = 0;
    
    /* Allocate the initial uses. */
    // this is going to be on the server
    s->size_uses = qsched_init_usespertask * s->size;
    if ( ( s->uses = (int *)malloc( sizeof(int) * s->size_uses ) ) == NULL ||
         ( s->uses_key = (int *)malloc( sizeof(int) * s->size_uses ) ) == NULL )
        error( "Failed to allocate memory for uses." );
    s->count_uses = 0;
    
    /* Allocate the initial data. */
    // this is going to be on the server
    s->size_data = qsched_init_datapertask * s->size;
    if ( ( s->data = malloc( s->size_data ) ) == NULL )
        error( "Failed to allocate memory for data." );
    s->count_data = 0;
    
    /* Init the pthread stuff. */
    // threads remain local
    // QUESTION: which of the s properties are also used on the server - hopefully none

    //#ifdef HAVE_PTHREAD
    //    if ( pthread_cond_init( &s->cond , NULL ) != 0 ||
    //        pthread_mutex_init( &s->mutex , NULL ) != 0 )
    //        error( "Error initializing yield cond/mutex pair." );
    //    if ( pthread_cond_init( &s->barrier_cond , NULL ) != 0 ||
    //         pthread_mutex_init( &s->barrier_mutex , NULL ) != 0 )
    //        error( "Error initializing barrier cond/mutex pair." );
    //    s->runners_count = 0;
    //    s->runners_size = qsched_init_runners;
    //    if ( ( s->runners = malloc( sizeof(struct qsched_pthread_runner) * s->runners_size ) ) == NULL )
    //     error( "Failed to allocate runners." );
    //    s->barrier_running = 0;
    //    s->barrier_count = 0;
    //    s->barrier_launchcount = 0;
    //    if ( pthread_mutex_lock( &s->barrier_mutex ) != 0 )
    //        error( "Failed to lock barrier mutex." );
    //#endif
    
    /* Clear the timers. */
    #ifdef TIMERS
        bzero( s->timers , sizeof(ticks) * qsched_timer_count );
    #endif
    
    // does the client program use members of the struct sched
    // 
    // this is needed on the server
    /* Init the sched lock. */
    lock_init( &s->lock );

    }

struct argmessage_message* qsargmadapter0_addres ( int adapter_idx , int owner , qsched_res_t parent ){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    int id = loc_qsched_addres(s, owner, parent);
    /* Return the res ID. */
    // pack the ID into the message
    struct argmessage_message* message = packunpack_packoneint(id);
    return message;
}

void qsargmadapter0_addlock ( int adapter_idx, qsched_task_t t , qsched_res_t res ){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    loc_qsched_addlock(s, t, res);    
}

void qsargmadapter0_addunlock (int adapter_idx , qsched_task_t ta , qsched_task_t tb ){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    loc_qsched_addunlock(s, ta, tb);
    }

struct argmessage_message* qsargmadapter0_addtask ( int adapter_idx , int type , unsigned int flags , void *data , int data_size , int cost ){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    
    void *temp;
    struct task *t;
    int id, data_size2;

    /* Lock the sched. */
    lock_lock( &s->lock );
    
    /* Do the tasks need to be re-allocated? */
    if ( s->count == s->size ) {
    
        /* Scale the task list size. */
        s->size *= qsched_stretch;
        
        /* Allocate a new task list. */
        if ( ( temp = malloc( sizeof(struct task) * s->size ) ) == NULL )
            error( "Failed to allocate new task list." );
            
        /* Copy the tasks over to the new list. */
        memcpy( temp , s->tasks , sizeof(struct task) * s->count );
        
        /* Free the old task list. */
        free( s->tasks );
        
        /* Set the new task list. */
        s->tasks = (struct task *)temp;
    
        }
        
    /* Round-up the data size. */
    data_size2 = ( data_size + (qsched_data_round-1) ) & ~(qsched_data_round-1);
        
    /* Do the task data need to be re-allocated? */
    if ( s->count_data + data_size2 > s->size_data ) {
    
        /* Scale the task list size. */
        s->size_data *= qsched_stretch;
        
        /* Allocate a new task list. */
        if ( ( temp = malloc( s->size_data ) ) == NULL )
            error( "Failed to allocate new task list." );
            
        /* Copy the tasks over to the new list. */
        memcpy( temp , s->data , s->count_data );
        
        /* Free the old task list. */
        free( s->data );
        
        /* Set the new task list. */
        s->data = temp;
    
        }
        
    /* Store the new task ID. */
    id = s->count;
        
    /* Init the new task. */
    t = &s->tasks[ id ];
    t->type = type;
    t->flags = flags;
    t->cost = cost;
    t->wait = 0;
    t->nr_conflicts = 0;
    t->nr_unlocks = 0;
    t->nr_locks = 0;
    t->nr_uses = 0;
    /* ADDITION FOR QSARGM: 2 line below */
    t->task_data_size = data_size;
    t->task_idx = id;
    //FOR DUMP:
    t->progress = task_progress_waiting;
#ifdef ARGM_SHOW_TRACE
    printf("adapter0_addtask: task->progress: %u", t->progress);
    fflush(stdout);
#endif    
    /* Add a relative pointer to the data. */
    memcpy( &s->data[ s->count_data ] , data , data_size );
    t->data = &s->data[ s->count_data ] - s->data;
    s->count_data += data_size2;
    
    /* Increase the task counter. */
    s->count += 1;
    
    /* Unlock the sched. */
    lock_unlock_blind( &s->lock );
    
    /* Return the task ID. */
    struct argmessage_message* message = packunpack_packoneint(id);
    
    return message;

    }

// target:
//struct argmessage_message* qsargmadapter0_addtask ( int adapter_idx , int type , unsigned int flags , void *data , int data_size , int cost )
void qsarmadapter0_call_addtask(int adapter_idx, void (*func)(), struct argmessage_message* message){
    char* bytearray = (char*) message->argsbuffer;
    int type = *((int*)bytearray);
    bytearray += sizeof(int);
    unsigned int flags = *((int*)bytearray);
    bytearray += sizeof(int);
    int datasize = *((int*)bytearray);
    bytearray += sizeof(int);
    void* data = bytearray;
    bytearray += datasize;
    int cost = *((int*)bytearray)  ;
    func(adapter_idx,  type , flags , data , datasize , cost); 
    return;
}

void qsargmadapter0_adduse ( int adapter_idx, qsched_task_t t , qsched_res_t res ){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    loc_qsched_adduse(s, t, res);
    }


void qsargmadapter0_free ( int adapter_idx){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;

    int k;
    #ifdef TIMERS
    message("server local s timers:")
    for (k = 0; k < qsched_timer_count; k++)
    message("timer %s is %lli ticks.", qsched_timer_names[k],
            s->timers[k]);
    #endif

    /* Clear all the buffers if allocated. */
    if ( s->tasks != NULL ) { free( s->tasks ); s->tasks = NULL; }
    if ( s->deps != NULL ) { free( s->deps ); s->deps = NULL; }
    if ( s->deps_key != NULL ) { free( s->deps_key ); s->deps_key = NULL; }
    if ( s->locks != NULL ) { free( s->locks ); s->locks = NULL; }
    if ( s->locks_key != NULL ) { free( s->locks_key ); s->locks_key = NULL; }
    if ( s->uses != NULL ) { free( s->uses ); s->uses = NULL; }
    if ( s->uses_key != NULL ) { free( s->uses_key ); s->uses_key = NULL; }
    if ( s->res != NULL ) { free( (void *)s->res ); s->res = NULL; }
    if ( s->data != NULL ) { free( s->data ); s->data = NULL; }
    
    /* Loop over the queues and free them too. */
    for ( k = 0 ; k < s->nr_queues ; k++ )
        queue_free( &s->queues[k] );
    free( s->queues );
    s->queues = NULL;
    
    /* Destroy the mutex and condition. */
    // threads are on the proxy side
    // #ifdef HAVE_PTHREAD
    //     if ( pthread_cond_destroy( &s->cond ) != 0 ||
    //          pthread_mutex_destroy( &s->mutex ) != 0 )
    //         error( "Error destroying pthread cond/mutex pair." );
    // #endif
        
    /* Clear the flags. */
    s->flags = qsched_flag_none;

    }


// this function is not called over the local-remote link per se
//   but rather various of its subsidiary functions are. 
// so it appears only on the client side
// namely --> void qsargmadapter0_run ( struct qsched *s , int nr_threads , qsched_funtype fun );



void qsargmadapter0_reset ( int adapter_idx ){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    loc_qsched_reset(s);    
    }

// TO DO: later
//void qsargmadapter0_addtask_dynamic ( struct qsched *s , int type , unsigned int flags , void *data , int data_size , int cost , qsched_res_t *locks , int nr_locks , qsched_res_t *uses , int nr_uses );

void qsargmadapter0_ensure ( int adapter_idx , int nr_tasks , int nr_res , int nr_deps , int nr_locks , int nr_uses , int size_data ){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    loc_qsched_ensure(s, nr_tasks , nr_res , nr_deps , nr_locks , nr_uses , size_data);
    }

void qsargmadapter0_res_own ( int adapter_idx , qsched_res_t res , int owner ){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    loc_qsched_res_own(s, res, owner);
    }

// this function could be void but need to be sure it is finished before doing gettask (also for meaningful timers)
struct argmessage_message* qsargmadapter0_prepare(int adapter_idx){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    loc_qsched_prepare(s);
    qsargm_clearcounters(s);
// reutrn dummy message to allow sychronisation at client (before gettasks start)
    #ifdef ARGM_SHOW_TRACE
    printf("QSARGM PROXY:qsargmadapter0_prepare: arming engine to go multithreaded\n");
    fflush(stdout);
    #endif
    //argmessage_gomultithreaded();
    argm_engine->gomultithreaded = true;
    #ifdef ARGM_SHOW_TRACE
    printf("QSARGM PROXY:qsargmadapter0_prepare: signalling to client that prepare done \n");
    fflush(stdout);
    #endif
    struct argmessage_message* message = packunpack_dummypayload();
    return message;
    /* time to switch to multithreaded operation of the scheduler on the server side - next operations from client will be gettask and done */
}



struct argmessage_message* qsargmadapter0_packtaskdata(void * data, int datasize){
    struct argmessage_message* message;
    int argsbuffersize = sizeof(int) + datasize; 
    message = argmessage_messagecreate(argsbuffersize);
    char* bytearray = (char*) message->argsbuffer;
    *((int*)bytearray) = datasize;
    bytearray += sizeof(int);
    memcpy(bytearray, data, datasize );
    return message;
}
struct argmessage_message* qsargmadapter0_getdata(int adapter_idx, int task_idx){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    struct task* t = &(s->tasks[task_idx]); 
    void* data = loc_qsched_getdata(s, t);
    struct argmessage_message* message = qsargmadapter0_packtaskdata(data,t->task_data_size);
    return message;
} 



/* the user code task runner calls for a calc type int and a pointer to the data (parameters of the calc) 
where are these retrieved from?
calc type is a simple data member of task
tasks hold a "payload offset" called data, and the sched has a byte array of data at member char* data 
getdata is return &s->data[ t->data ];
the runner for the task is sent the pointer to the task data and the task type - therefore could just pass the data block for the task in messages, keeping the data on the server 
but how do I know the size of data block allocated to each task (check pointer in next task and difference? -> now added to the task struct
SOLUTION: getttask should return message containing the calc type and the task data and send that back to _pthread_run (and _run_openmp) 
which shoul dconsume them directly rather than from the task object and the data array of the scheduler, respectively, as done in hte original qsched 
*/

struct argmessage_message* qsargmadapter0_packtask(struct qsched* s, struct task* t){
    struct argmessage_message* message;
    if (t != NULL){
        int argsbuffersize = 3*sizeof(int) + t->task_data_size; 
        message = argmessage_messagecreate(argsbuffersize);
        char* bytearray = (char*) message->argsbuffer;
        *((int*)bytearray) = t->task_idx;
        bytearray += sizeof(int);
        *((int*)bytearray) = t->type;
        bytearray += sizeof(int);
        *((int*)bytearray) = t->task_data_size;
        bytearray += sizeof(int);
        memcpy(bytearray, &s->data[ t->data ], t->task_data_size );
#ifdef ARGM_SHOW_TRACE
        printf("qsargmadapter0_packtask: packing task with id=%i on thread %i \n", t->task_idx, omp_get_thread_num());
        fflush(stdout);
#endif
    }
    else{
        int argsbuffersize = sizeof(int); 
        message = argmessage_messagecreate(argsbuffersize);
        char* bytearray = (char*) message->argsbuffer;
        *((int*)bytearray) = -1; // task id of -1 means task is NULL
#ifdef ARGM_SHOW_TRACE
        printf("qsargmadapter0_packtask: packing task with id=-1, i.e. none left, on thread %i\n", omp_get_thread_num());
        fflush(stdout);
#endif
    }
    return message;
}

struct argmessage_message* qsargmadapter0_gettask(int adapter_idx, int qid){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
#ifdef ARGM_SHOW_TRACE
    printf("QSARGM ADAPTER: in gettask adapter - calling local getttask on qid: %i\n", qid);
    fflush(stdout);
#endif
    atomic_inc(&s->count_adapter_gettask_recv);
    /* if multithreaded loop here, (i) checking for a task, (ii) checking for a done message with the argmessage function for that 
      termination is when the local sched is out of tasks
      but sticking with doevents mechanism for single threaded server */
    struct task* allocated_task;
    if (s->doevents_callback != NULL && omp_get_num_threads() > 1) {  // 

        while (s->waiting > 0) {
            allocated_task = loc_qsched_gettask(s, qid);
            if ( allocated_task != NULL){
                break;
            }
            argmessage_checkandconsumemessage(taskdone); // argument tag for an incoming task done message
        }
        if (s->waiting == 0) {
            allocated_task = NULL; // this cause the call to qsargmadapter0_packtask below to send a -1 task, i.e. terminate
            atomic_inc(&s->count_adapter_gettask_sent_terminate);
        }
        else{ // a task was found 
            atomic_inc(&s->count_adapter_gettask_sent_normal);
        }
    }
    else{ // option for single threaded server - using do events, or for control test with sched on client
        allocated_task = loc_qsched_gettask(s, qid);
        if (allocated_task == NULL) {
            atomic_inc(&s->count_adapter_gettask_sent_terminate);
        }
        else {
            atomic_inc(&s->count_adapter_gettask_sent_normal);
        }
    }
#ifdef ARGM_SHOW_TRACE
    if (allocated_task !=NULL) printf("QSARGM ADAPTER: in gettask adapter for qid: %d - taskid returned: %i\n", qid, allocated_task->task_idx);
    printf("QSARGM ADAPTER: packing task\n");
    fflush(stdout);
#endif
    struct argmessage_message* message = qsargmadapter0_packtask(s, allocated_task);
    return message;
}


void qsargmadapter0_done(int adapter_idx, int task_idx){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    struct task* t = &(s->tasks[task_idx]);
    atomic_inc(&s->count_adapter_done_recv);
    loc_qsched_done(s, t);
}

void qsargmadapter0_debugdumptask (int adapter_idx){
    fprintf(stderr, "in qsargmadapter0_debugdumptask\n");
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    if (s->count == 0){
        printf("There are no tasks on the server to dump!!!\n");
        fflush(stdout);
    }
    else{
        printf("Tasks are:\n");
        fflush(stdout);
        printf("(type, qid, d[0], d[1], tic, toc)\n");
        for ( int k = 0 ; k < s->count ; k++ ) {
            int *d = (int *)&s->data[ s->tasks[k].data ];
            printf( " %i %i %i %i %lli %lli\n" , s->tasks[k].type , s->tasks[k].qid ,
            d[0] , d[1] , s->tasks[k].tic , s->tasks[k].toc );
        } 
        printf("end of tasks\n");
        fflush(stdout);
    }
}

void qsargmadapter0_debugdumpcounters (int adapter_idx){
    struct qsched* s = (struct qsched*) argm_engine->adapters[adapter_idx].applicationstate;
    qsargm_printcounters(s, "adapter");
}

void qsargmadapter0_serverregisterfunctions(int adapter_idx){
    // prototype is void argmessage_serverengineregisterfunction(int adapter_idx, int func_idx, void (*targetfunc)(), void (*callerfunc)(), bool functionisvoid);
    //TODO: will call functions need to use declared types rather than ordinary int?
    argmessage_serverengineregisterfunction(adapter_idx, schedinit, qsargmadapter0_init, packunpack_calltwoint, true);
    argmessage_serverengineregisterfunction(adapter_idx, addres, (void*)qsargmadapter0_addres, packunpack_calltwoint, false);
    argmessage_serverengineregisterfunction(adapter_idx, addlock, qsargmadapter0_addlock, packunpack_calltwoint, true);
    argmessage_serverengineregisterfunction(adapter_idx, addunlock, qsargmadapter0_addunlock, packunpack_calltwoint, true); 
    argmessage_serverengineregisterfunction(adapter_idx, addtask, (void*)qsargmadapter0_addtask, qsarmadapter0_call_addtask, false);
    // LATER: later
    //argmessage_serverengineregisterfunction(adapter_idx, addtaskdynamic, qsargmadapter0_
    argmessage_serverengineregisterfunction(adapter_idx, adduse, qsargmadapter0_adduse, packunpack_calltwoint, true);
    argmessage_serverengineregisterfunction(adapter_idx, schedfree, qsargmadapter0_free, packunpack_callnoargs, true);
    argmessage_serverengineregisterfunction(adapter_idx, schedreset, qsargmadapter0_reset, packunpack_callnoargs, true);
    argmessage_serverengineregisterfunction(adapter_idx, ensure, qsargmadapter0_ensure, packunpack_callsixint, true);
    argmessage_serverengineregisterfunction(adapter_idx, resown, qsargmadapter0_res_own, packunpack_calltwoint, true);
    argmessage_serverengineregisterfunction(adapter_idx, getdata, (void*)qsargmadapter0_getdata, packunpack_calloneint, false);
    argmessage_serverengineregisterfunction(adapter_idx, prepare, (void*)qsargmadapter0_prepare, packunpack_callnoargs, false);  // prepare could be void but need prepare finished before gettask is legal.
    argmessage_serverengineregisterfunction(adapter_idx, gettask, (void*)qsargmadapter0_gettask, packunpack_calloneint, false);
    argmessage_serverengineregisterfunction(adapter_idx, taskdone, qsargmadapter0_done, packunpack_calloneint, true);

    argmessage_serverengineregisterfunction(adapter_idx, debugdumptask, qsargmadapter0_debugdumptask, packunpack_callnoargs, true);
    argmessage_serverengineregisterfunction(adapter_idx, debugdumpcounters, qsargmadapter0_debugdumpcounters, packunpack_callnoargs, true);
    return;
}

