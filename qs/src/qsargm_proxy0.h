#ifndef QSARGM_PROXY_H
#define QSARGM_PROXY_H

#include "qsched_local.h"




/* local helper functions */
void qsargmproxy0_run_pthread( int adapter_idx, int nr_threads , qsched_funtype fun );
void *qsargmproxy0_pthread_run( void *in );
void qsargmproxy0_run_openmp( int adapter_idx, int nr_threads , qsched_funtype fun );

/* Strategy 0 entry points for qsched external functions. */
void qsargmproxy0_init ( int adapter_idx, int nr_queues , int flags );
qsched_res_t qsargmproxy0_addres ( int adapter_idx , int owner , qsched_res_t parent );
void qsargmproxy0_addlock ( int adapter_idx , qsched_task_t t , qsched_res_t res );
void qsargmproxy0_addunlock ( int adapter_idx , qsched_task_t ta , qsched_task_t tb );
qsched_task_t qsargmproxy0_addtask ( int adapter_idx, int type , unsigned int flags , void *data , int data_size , int cost );
struct argmessage_message* qsargmproxy0_addtask_pack(int type , unsigned int flags , void *data , int data_size , int cost );
void qsargmproxy0_adduse ( int adapter_idx , qsched_task_t t , qsched_res_t res );
void qsargmproxy0_free ( int adapter_idx  );
void qsargmproxy0_run (  int adapter_idx,  int nr_threads , qsched_funtype fun );
void qsargmproxy0_reset (  int adapter_idx  );
// LATER:void qsargmproxy0_addtask_dynamic ( struct qsched *s , int type , unsigned int flags , void *data , int data_size , int cost , qsched_res_t *locks , int nr_locks , qsched_res_t *uses , int nr_uses );
void qsargmproxy0_ensure ( int adapter_idx  , int nr_tasks , int nr_res , int nr_deps , int nr_locks , int nr_uses , int size_data );
void qsargmproxy0_res_own ( int adapter_idx  , qsched_res_t res , int owner );

void qsargmproxy0_debugdumptasks ( int adapter_idx);
void qsargmproxy0_debugdumpcounters( int adapter_idx );

#endif /* QSARGM_PROXY_H */