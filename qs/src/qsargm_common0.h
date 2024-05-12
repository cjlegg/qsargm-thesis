#ifndef QSARGM_COMMON0_H
#define QSARGM_COMMON0_H

enum qsargm_remotefunctions{
    schedinit,      // 0
    addres,
    addlock,
    addunlock,
    addtask,
    addtaskdynamic, // 5
    adduse,
    schedfree,
    schedreset,
    ensure,
    resown,         // 0xA
    getdata,
    prepare,
    gettask,        // 0xD
    taskdone,        // 0xE 
    debugdumptask,
    debugdumpcounters
};

void qsargm_clearcounters(struct qsched* s);
void qsargm_printcounters(struct qsched* s, char* setname);

#endif /* QSARGM_COMMON0_H */