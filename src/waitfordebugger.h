#ifndef WAITFORDEBUGGER_H
#define WAITFORDEBUGGER_H

#include "config.h"
#if ARGM_WITH_SNAPI != 1
void wait_for_debugger();

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

#endif /* ARGM_WITH_SNAPI != 1 */

#endif /* WAITFORDEBUGGER_H */
