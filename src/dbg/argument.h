#ifndef _ARGUMENT_H
#define _ARGUMENT_H

#include "addrinfo.h"

struct ARGUMENTSINFO
{
    char mod[MAX_MODULE_SIZE];
    duint start;
    duint end;
    bool manual;
    duint instructioncount;
};

bool ArgumentAdd(duint Start, duint End, bool Manual, duint InstructionCount = 0);
bool ArgumentGet(duint Address, duint* Start = nullptr, duint* End = nullptr, duint* InstrCount = nullptr);
bool ArgumentOverlaps(duint Start, duint End);
bool ArgumentDelete(duint Address);
void ArgumentDelRange(duint Start, duint End, bool DeleteManual = false);
void ArgumentCacheSave(JSON Root);
void ArgumentCacheLoad(JSON Root);
void ArgumentClear();
void ArgumentGetList(std::vector<ARGUMENTSINFO> & list);
bool ArgumentGetInfo(duint Address, ARGUMENTSINFO & info);

#endif // _ARGUMENT_H