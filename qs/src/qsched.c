/*******************************************************************************
 * This file is part of QuickSched.
 * Coypright (c) 2013 Pedro Gonnet (pedro.gonnet@durham.ac.uk)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 ******************************************************************************/

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

/* Local includes. */
#include "argmessagesizes.h"
#include "argmessage.h"
#include "argmessageproxy.h"
/* stargegy = -1 = all local */
#include "quicksched_local.h"
/* strategy = 0 = tasks and queues on remote server */
#include "qsargm_common0.h"
#include "qsargm_proxy0.h"

// /** Timer names. */
// char *qsched_timer_names[ qsched_timer_count ] = 
//     { "queue" , "qlock" , "lock" , "gettask" , "done" , "prepare" };
    
int activestrategy;
int adapter0_idx;
//int myrank;
extern struct argmessage_proxy* proxy;
extern struct qsched* proxy0sched;

/* temp text


*/

void qsched_init( struct qsched *s , int nr_queues , int flags ){
    // activestrategy = 0; client program should set this according to its requirements 
    printf("QSARGM: in qsched_init() with activestrategy = %d\n", activestrategy);
    adapter0_idx = proxy->myrank * ARGMESSAGE_ADAPTERS_PER_PROXY + 0; // i.e. zeroth of block for this rank 
    switch ( activestrategy ) {
        case 0:
            // the s version of the scheduler holds some timers also     
            #ifdef TIMERS
                bzero( s->timers , sizeof(ticks) * qsched_timer_count );
            #endif        
            qsargmproxy0_init (adapter0_idx, nr_queues , flags );
            break;
        default:
            loc_qsched_init ( s , nr_queues , flags );
    }
}

qsched_res_t qsched_addres( struct qsched *s , int owner , qsched_res_t parent ){
    switch ( activestrategy ) {
        case 0:
            return qsargmproxy0_addres ( adapter0_idx, owner , parent );
            break;
        default:
            return loc_qsched_addres ( s , owner , parent );
    }
}


void qsched_addlock( struct qsched *s , qsched_task_t t , qsched_res_t res ){
    switch ( activestrategy ) {
        case 0:
            qsargmproxy0_addlock ( adapter0_idx, t , res );
            break;
        default:
            loc_qsched_addlock ( s , t , res );
    }
}

void qsched_addunlock( struct qsched *s , qsched_task_t ta , qsched_task_t tb ){
    switch ( activestrategy ) {
        case 0:
            qsargmproxy0_addunlock ( adapter0_idx , ta , tb );
            break;
        default:
            loc_qsched_addunlock ( s , ta , tb );
    }
}

qsched_task_t qsched_addtask( struct qsched *s , int type , unsigned int flags , void *data , int data_size , int cost ){
    switch ( activestrategy ) {
        case 0:
            return qsargmproxy0_addtask ( adapter0_idx, type , flags ,data , data_size ,  cost );
            break;
        default:
            return loc_qsched_addtask ( s , type , flags , data , data_size , cost );
    }
}

void qsched_adduse( struct qsched *s , qsched_task_t t , qsched_res_t res ){
    switch ( activestrategy ) {
        case 0:
            qsargmproxy0_adduse ( adapter0_idx,t , res );
            break;
        default:
            loc_qsched_adduse ( s , t , res );
    }
}

void qsched_free( struct qsched *s ){
    switch ( activestrategy ) {
        case 0:
            qsargmproxy0_free ( adapter0_idx);
            break;
        default:
            loc_qsched_free ( s );
    }
}

void qsched_run( struct qsched *s , int nr_threads , qsched_funtype fun ){
    switch ( activestrategy ) {
        case 0:
            qsargmproxy0_run (adapter0_idx, nr_threads , fun );
            break;
        default:
            loc_qsched_run ( s , nr_threads , fun );
    }
}

void qsched_reset( struct qsched *s ){
    switch ( activestrategy ) {
        case 0:
            qsargmproxy0_reset ( adapter0_idx  );
            break;
        default:
            loc_qsched_reset ( s );
    }
}

void qsched_debugdumptasks( struct qsched *s ){
    switch ( activestrategy ){
        case 0:
            qsargmproxy0_debugdumptasks( adapter0_idx );
            break;
        default:
            loc_qsched_debugdumptasks( s );
    }
}

void qsched_debugdumpcounters( struct qsched *s ){  // 
    switch ( activestrategy ){
        case 0:
            qsargmproxy0_debugdumpcounters( adapter0_idx );  // this dumps the tasks on the server
            qsargm_printcounters(proxy0sched, "proxy");  // TO DO  this is wrong scheduler need to print here from proxy0sched -- how to pick up a reference??? 
            break;
        default:
            qsargm_printcounters(s, "local sched");
    }

}



// TODO: impement _addtask_dynamic later
// void qsched_addtask_dynamic ( struct qsched *s , int type , unsigned int flags , void *data , int data_size , int cost , qsched_res_t *locks , int nr_locks , qsched_res_t *uses , int nr_uses ){
//     switch ( activestrategy ) {
//         case 0:
//             qsargmproxy0_addtask_dynamic (  adapter0_idx ,  type ,  flags ,  data ,  data_size ,  cost ,  locks , nr_locks , uses , nr_uses );
//             break;
//         default:
//             loc_qsched_addtask_dynamic ( s , type ,  flags , data , data_size , cost , locks , nr_locks , uses , nr_uses );
//     }
// }

void qsched_ensure( struct qsched *s , int nr_tasks , int nr_res , int nr_deps , int nr_locks , int nr_uses , int size_data ){
    switch ( activestrategy ) {
        case 0:
            qsargmproxy0_ensure ( adapter0_idx  ,  nr_tasks , nr_res ,  nr_deps ,  nr_locks ,  nr_uses ,  size_data );
            break;
        default:
            loc_qsched_ensure ( s , nr_tasks , nr_res , nr_deps , nr_locks , nr_uses , size_data );
    }
}

void qsched_res_own( struct qsched *s , qsched_res_t res , int owner ){
    switch ( activestrategy ) {
        case 0:
            qsargmproxy0_res_own ( adapter0_idx ,  res , owner );
            break;
        default:
            loc_qsched_res_own ( s , res , owner );
    }
}

