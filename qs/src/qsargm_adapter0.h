#ifndef QSARGM_ADAPTER0_H
#define QSARGM_ADAPTER0_H

#include "quicksched_local.h"
#include "argmessage.h"



// TODO: unpdate declarations to signatures of definitions


/* Strategy 0 entry points for qsched external functions. */
void qsargm_serversinit0(struct argmessage_serverengine* engine);

void qsargmadapter0_init ( int adapter_idx , int nr_queues , int flags );
struct argmessage_message* qsargmadapter0_addres ( struct qsched *s , int owner , qsched_res_t parent );
void qsargmadapter0_addlock ( int adapter_idx , qsched_task_t t , qsched_res_t res );
void qsargmadapter0_addunlock ( int adapter_idx , qsched_task_t ta , qsched_task_t tb );
struct argmessage_message* qsargmadapter0_addtask ( int adapter_idx , int type , unsigned int flags , void *data , int data_size , int cost );
void qsargmadapter0_adduse ( int adapter_idx , qsched_task_t t , qsched_res_t res );
void qsargmadapter0_free ( int adapter_idx  );
// qsched_run is not called on the server; some of its component functions are called separately from the client
void qsargmadapter0_reset ( int adapter_idx );
// TODO: implement this function later
//void qsargmadapter0_addtask_dynamic ( struct qsched *s , int type , unsigned int flags , void *data , int data_size , int cost , qsched_res_t *locks , int nr_locks , qsched_res_t *uses , int nr_uses );
void qsargmadapter0_ensure ( int adapter_idx , int nr_tasks , int nr_res , int nr_deps , int nr_locks , int nr_uses , int size_data );
void qsargmadapter0_res_own (int adapter_idx , qsched_res_t res , int owner );
void qsargmadapter0_prepare(int adapter_idx);
struct argmessage_message* qsargmadapter0_getdata(int adapter_idx, int task_idx);
struct argmessage_message* qsargmadapter0_packtaskdata(void * data, int datasize);
struct argmessage_message* qsargmadapter0_gettask(int adapter_idx, int qid);
struct argmessage_message* qsargmadapter0_packtask(struct qsched* s, struct task* t);
void qsargmadapter0_done(int adapter_idx, int task_idx);
void qsargmadapter0_debugdumptask (int adapter_idx);
void qsargmadapter0_debugdumpcounters (int adapter_idx);

void qsargmadapter0_serverregisterfunctions(int adapter_idx);

#endif /* QSARGM_ADAPTER0_H */