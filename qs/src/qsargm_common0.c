#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "quicksched_local.h"
#include "argmessagesizes.h"
#include "argmessageinternalhelpers.h"
#include "argmessage.h"
#include "qsargm_common0.h"
#include "waitfordebugger.h"


void qsargm_clearcounters(struct qsched* s){
    s->count_proxy_gettask_sent = 0;
    s->count_proxy_gettask_recv_normal = 0;
    s->count_proxy_gettask_recv_terminate = 0;
    s->count_proxy_done_sent = 0;
    s->count_adapter_gettask_recv = 0;
    s->count_adapter_gettask_sent_normal = 0;
    s->count_adapter_gettask_sent_terminate = 0;
    s->count_adapter_done_recv = 0;
    s->count_adapter_doevents = 0;
}


void qsargm_printcounters(struct qsched* s, char* setname){
    printf("COUNTERS START for set %s\n", setname);
    printf("count_proxy_gettask_sent             = %i\n",  s->count_proxy_gettask_sent);
    printf("count_proxy_gettask_recv_normal      = %i\n",  s->count_proxy_gettask_recv_normal);
    printf("count_proxy_gettask_recv_terminate   = %i\n",  s->count_proxy_gettask_recv_terminate);
    printf("count_proxy_done_sent                = %i\n",  s->count_proxy_done_sent);
    printf("count_adapter_gettask_recv           = %i\n",  s->count_adapter_gettask_recv);
    printf("count_adapter_gettask_sent_normal    = %i\n",  s->count_adapter_gettask_sent_normal);
    printf("count_adapter_gettask_sent_terminate = %i\n",  s->count_adapter_gettask_sent_terminate);
    printf("count_adapter_done_recv              = %i\n",  s->count_adapter_done_recv);
    printf("count_adapter_doevents               = %i\n",  s->count_adapter_doevents);
    printf("COUNTERS END for set %s\n", setname);
    fflush(stdout);
}
