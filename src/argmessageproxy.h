#ifndef ARGMESSAGEPROXY_H
#define ARGMESSAGEPROXY_H

#include "argmessageinternalhelpers.h"
#include "argmessagesizes.h"

struct argmessage_proxy {
    int myrank;
    int ranks;
    int serverrank;
    long maxrequests;
    long requestshandled;
#if defined QSARGM_OPTIM_01 && defined HAVE_OPENMP
    // pointers to spaces for send and receive buffers - these will be alternate send and recv to keep pair in cache for core 
    void* sendrecvbuffers;
#endif
    //char commonreceivebuffer[ARGMESSAGE_RECEIVE_BUFFER_BYTES];
 };

/* function prototypes */
struct argmessage_proxy* argmessage_proxygetobject(int argc, char* argv[]);
int argmessage_proxyfree();
void argmessage_proxyechoproxystatusonproxy();
void argmessage_proxyrun(int maxrequests);
void argmessage_proxysendnoop();
void argmessage_proxyenlivenadapter(int adapter_idx);
void argmessage_proxykilladapter(int adapter_idx);
void argmessage_proxykillserverrequest();
void argmessage_proxysendstdoutmessage(char* string0);
int argmessage_proxyhowmanyliveadapters();
void argmessage_proxyrestoreserversinglethreaded(int adapter_idx);
void argmessage_proxysendfunction(struct argmessage_message* message);
struct argmessage_message* argmessage_proxyreceivefunctionresult(struct argmessage_message* sentmessage);







#endif /* ARGMESSAGEPROXY_H */