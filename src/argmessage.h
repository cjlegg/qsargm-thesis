#ifndef ARGMESSAGE_H
#define ARGMESSAGE_H

#include <mpi.h>
#include <stdbool.h>

#include "argmessagesizes.h"

/* enums of internal functions callable by 
 * application programmer is to draw up similar enum for payload functions
*/

enum argmessage_server_functions {
    nooperation = ARGMESSAGE_SERVER_FUNCTIONS_BASE,
    enlivenadapter,
    killadapter,
    killserverrequest,
    stdoutmessage,
    echoenginestatus,
    howmanyadapters,
    verbosityfilteredstdoutmessage,
    echofunctionnames,
    setunknownfunctionnumberbehaviour,
    startlog,
    logmessage,
    closelog,
    restoreserversinglethreaded
};


/* object structs */

struct argmessage_adpater {
    int live;
    //int adapterid;  // FORNOW: adapters are indexed by rank - LATER: ? more than one adapter per client
    int myrank;
    int proxyrank;
    int requestshandled;
    /* array of function pointers */
    void (*targetfunctions[ARGMESSAGE_MAX_FUNCTION_NAMES])();
    /* corresponding array of functions to call those with appropriate argument pick the caller with
    * "appropraite" means that the user the right paramter signature
    * singnature matches the model functions in packunpack.c/h
    */
    void (*functioncallers[ARGMESSAGE_MAX_FUNCTION_NAMES])();
    bool funcisvoid[ARGMESSAGE_MAX_FUNCTION_NAMES];
    void* applicationstate; /* for the application program's state */
};

struct argmessage_serverengine {
    int myrank;
    int ranks; // ie.e a count - FORNOW: world LATER: size of comm for the client server set
    bool killifnoadaptersisrequested;
    bool waitingforfirstadapter;
    int liveadapters; // i.e. a count
    long requestshandled; // i.e. a count
    long requestshandledbyproxy[ARGMESSAGE_MAX_RANKS];
    long maxrequests; // 0 means ignore the maximum
    bool terminate;
    bool gomultithreaded;
    int activethreads; 
    // FORNOW: indexed by MPIRank
    struct argmessage_adpater adapters[ARGMESSAGE_MAX_ADAPTERS_PER_ENGINE];
};



/* public interface */
struct argmessage_serverengine* argmessage_serverenginegetobject(int argc, char* argv[]);
void argmessage_serverengineregisterfunction(int adapter_idx, int func_idx, void (*targetfunc)(), void (*callerfunc)(), bool functionisvoid);
int argmessage_serverenginerun(long maxrequests);
int argmessage_serverenginefree();
void argmessage_serverenginenullfuncerror(int adapter_idx);
void argmessage_serverenginenullfunclog(int adapter_idx);

void argmessage_serverenginenoop(int adapter_idx);
void argmessage_serverengineenlivenadapter(int adapter_idx);
struct argmessage_message* argmessage_serverenginekilladapter(int adapter_idx);  // returns numbers of noops to send if engine is terminating. 0 if none. 
struct argmessage_message* argmessage_serverenginekillserverrequest(int adapter_idx);  // returns a dummy to make sure it is processed before killadapter, which is sent next
void argmessage_serverenginestdoutmessage(int adapter_idx, int string0len, char* string0);
void argmessage_serverengineechoenginestatus(int adapter_idx);
struct argmessage_message* argmessage_serversendhowmanyadapters(int adapter_idx);
void argmessage_consumemessage();
void argmessage_checkandconsumemessage(int tag);
void argmessage_gomultithreaded();
struct argmessage_message* argmessage_restoreserversinglethreaded(int adapter_idx);


#endif /* ARGMESSAGE_H */