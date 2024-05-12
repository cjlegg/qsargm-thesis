#include "../../qsoriginal/src/qsched_local.h"
/* More local includes - since this is the argmessage version of quicksched */
/* This is the argmessage version of quicksched so let's include argmessage */
#include "argmessagesizes.h"
#include "argmessageinternalhelpers.h"
#include "argmessage.h"
#include "argmessageproxy.h"
#include "packunpack.h"


/* These are for strategy 0 - task graph and queue on remote */
#include "qsargm_common0.h"
#include "qsargm_adapter0.h"
#include "qsargm_proxy0.h"
/* LATER: add headers for more strategies as they are implemented */ 




/* External functions. */
void qsched_init( struct qsched *s , int nr_queues , int flags );
qsched_res_t qsched_addres( struct qsched *s , int owner , qsched_res_t parent );
void qsched_addlock( struct qsched *s , qsched_task_t t , qsched_res_t res );
void qsched_addunlock( struct qsched *s , qsched_task_t ta , qsched_task_t tb );
qsched_task_t qsched_addtask( struct qsched *s , int type , unsigned int flags , void *data , int data_size , int cost );
void qsched_adduse( struct qsched *s , qsched_task_t t , qsched_res_t res );
void qsched_free( struct qsched *s );
void qsched_run( struct qsched *s , int nr_threads , qsched_funtype fun );
void qsched_reset( struct qsched *s );
void qsched_addtask_dynamic( struct qsched *s , int type , unsigned int flags , void *data , int data_size , int cost , qsched_res_t *locks , int nr_locks , qsched_res_t *uses , int nr_uses );
void qsched_ensure( struct qsched *s , int nr_tasks , int nr_res , int nr_deps , int nr_locks , int nr_uses , int size_data );
void qsched_res_own( struct qsched *s , qsched_res_t res , int owner );

void qsched_debugdumptasks( struct qsched *s );
void qsched_debugdumpcounters( struct qsched *s );
