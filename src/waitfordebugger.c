#include "config.h"
#if ARGM_WITH_SNAPI != 1

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <mpi.h>

#include "waitfordebugger.h"

// see http://www.sci.utah.edu/~tfogal/academic/Fogal-ParallelDebugging.pdf
// for this next function

// this compiles with: mpicc -ggdb test1.c -o test1
// an mpi error is cleared using: sudo sysctl -w kernel.yama.ptrace_scope=0
// running with: mpirun -np 4 ./test1  run without the pause
// running with mpirun -np 4 -x ARGM_MPI_DEBUG=1 ./test1
// run this in another termnial and attach debugger here with 
// vscode Debug menu / Run (gdb) Attach, which reads in launch.json as below:
// you need to pause i..e button: || in debugger and set proc_hold to non-zero
// then use the debuugger as normal
/*
        {
            "name": "(gdb) Attach",
            "type": "cppdbg",
            "request": "attach",
            "program": "/home/jlegg/PhDProject/expt-gdb-practice/test1",
            "processId": "${command:pickProcess}",
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        },
*/
 
void wait_for_debugger()
{
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    if( getenv("ARGM_MPI_DEBUG") != NULL && world_rank == 0 ) {
        volatile int proc_hold = 0;
        printf ("pid %ld waiting for debugger \n", ( long ) getpid() ) ;
        fflush(stdout);
        while ( proc_hold == 0 ) { /* in the debugger do: set variable proc_hold=1 */ }
    }
    MPI_Barrier ( MPI_COMM_WORLD ) ;
    printf("Rank %i continuing after barrier\n", world_rank);
    fflush(stdout);
}

#endif /* ARGM_WITH_SNAPI != 1 */