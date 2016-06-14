#include "QBeaEngine.h"

QBeaEngine::QBeaEngine(int maxModuleSize)
    : _tokenizer(maxModuleSize), mCodeFoldingManager(nullptr)
{
    CapstoneTokenizer::UpdateColors();
}

/**
 * @brief       Return the address of the nth instruction before the instruction pointed by ip.                 @n
 *              This function has been grabbed from OllyDbg ("Disassembleback" in asmserv.c)
 *
 * @param[in]   data    Address of the data to disassemble
 * @param[in]   base    Original base address of the memory page (Required to disassemble destination addresses)
 * @param[in]   size    Size of the data block pointed by data
 * @param[in]   ip      RVA of the current instruction (Relative to data pointer)
 * @param[in]   n       Number of instruction back
 *
 * @return      Return the RVA (Relative to the data pointer) of the nth instruction before the instruction pointed by ip
 */
ulong QBeaEngine::DisassembleBack(byte_t* data, duint base, duint size, duint ip, int n)
{
    int i;
    uint abuf[128], addr, back, cmdsize;
    unsigned char* pdata;

    // Reset Disasm Structure
    Capstone cp;

    // Check if the pointer is not null
    if(data == NULL)
        return 0;

    // Round the number of back instructions to 127
    if(n < 0)
        n = 0;
    else if(n > 127)
        n = 127;

    // Check if the instruction pointer ip is not outside the memory range
    if(ip >= size)
        ip = size - 1;

    // Obvious answer
    if(n == 0)
        return ip;

    //if(ip < (uint)n)
    //    return ip;

    //TODO: buffer overflow due to unchecked "back" value
    back = MAX_DISASM_BUFFER * (n + 3); // Instruction length limited to 16

    if(ip < back)
        back = ip;

    addr = ip - back;
    if(mCodeFoldingManager && mCodeFoldingManager->isFolded(addr + base))
    {
        duint newback = mCodeFoldingManager->getFoldBegin(addr + base);
        if(newback >= base && newback < size + base)
            addr = newback - base;
    }

    pdata = data + addr;

    for(i = 0; addr < ip; i++)
    {
        abuf[i % 128] = addr;

        if(mCodeFoldingManager && mCodeFoldingManager->isFolded(addr + base))
        {
            duint newaddr = mCodeFoldingManager->getFoldBegin(addr + base);
            if(newaddr >= base)
            {
                addr = newaddr - base;
            }
            cmdsize = mCodeFoldingManager->getFoldEnd(addr + base) - (addr + base) + 1;
        }
        else
        {
            if(!cp.Disassemble(addr + base, pdata, (int)size))
                cmdsize = 2; //heuristic for better output (FF FE or FE FF are usually part of an instruction)
            else
                cmdsize = cp.Size();
        }

        pdata += cmdsize;
        addr += cmdsize;
        //back -= cmdsize; // dead code
        size -= cmdsize;
    }

    if(i < n)
        return abuf[0];
    else
        return abuf[(i - n + 128) % 128];

}

/**
 * @brief       Return the address of the nth instruction after the instruction pointed by ip.                 @n
 *              This function has been grabbed from OllyDbg ("Disassembleforward" in asmserv.c)
 *
 * @param[in]   data    Address of the data to disassemble
 * @param[in]   base    Original base address of the memory page (Required to disassemble destination addresses)
 * @param[in]   size    Size of the data block pointed by data
 * @param[in]   ip      RVA of the current instruction (Relative to data pointer)
 * @param[in]   n       Number of instruction next
 *
 * @return      Return the RVA (Relative to the data pointer) of the nth instruction after the instruction pointed by ip
 */
ulong QBeaEngine::DisassembleNext(byte_t* data, duint base, duint size, duint ip, int n)
{
    int i;
    uint cmdsize;
    unsigned char* pdata;

    // Reset Disasm Structure
    Capstone cp;

    if(data == NULL)
        return 0;

    if(ip >= size)
        ip = size - 1;

    if(n <= 0)
        return ip;

    pdata = data + ip;
    size -= ip;

    for(i = 0; i < n && size > 0; i++)
    {
        if(mCodeFoldingManager && mCodeFoldingManager->isFolded(ip + base))
        {
            cmdsize = mCodeFoldingManager->getFoldEnd(ip + base) - (ip + base) + 1;
        }
        else
        {
            if(!cp.Disassemble(0, pdata, (int)size))
                cmdsize = 1;
            else
                cmdsize = cp.Size();
        }

        pdata += cmdsize;
        ip += cmdsize;
        size -= cmdsize;
    }

    return ip;
}

/**
 * @brief       Disassemble the instruction at the given ip RVA.
 *
 * @param[in]   data            Pointer to memory data (Can be either a buffer or the original data memory)
 * @param[in]   size            Size of the memory pointed by data (Can be the memory page size if data points to the original memory page base address)
 * @param[in]   instIndex       Offset to reach the instruction data from the data pointer
 * @param[in]   origBase        Original base address of the memory page (Required to disassemble destination addresses)
 * @param[in]   origInstRVA     Original Instruction RVA of the instruction to disassemble
 *
 * @return      Return the disassembled instruction
 */

Instruction_t QBeaEngine::DisassembleAt(byte_t* data, duint size, duint instIndex, duint origBase, duint origInstRVA)
{
    //tokenize
    CapstoneTokenizer::InstructionToken cap;
    _tokenizer.Tokenize(origBase + origInstRVA, data + instIndex, size, cap);
    int len = _tokenizer.Size();

    const auto & cp = _tokenizer.GetCapstone();
    bool success = cp.Success();

    auto branchType = Instruction_t::None;
    if(success && (cp.InGroup(CS_GRP_JUMP) || cp.IsLoop()))
    {
        switch(cp.GetId())
        {
        case X86_INS_JMP:
            branchType = Instruction_t::Unconditional;
            break;
        default:
            branchType = Instruction_t::Conditional;
            break;
        }
    }

    Instruction_t wInst;
    wInst.instStr = QString(cp.InstructionText().c_str());
    wInst.dump = QByteArray((const char*)data + instIndex, len);
    wInst.rva = origInstRVA;
    if(mCodeFoldingManager && mCodeFoldingManager->isFolded(origInstRVA))
        wInst.length = mCodeFoldingManager->getFoldEnd(origInstRVA + origBase) - (origInstRVA + origBase) + 1;
    else
        wInst.length = len;
    wInst.branchType = branchType;
    wInst.branchDestination = cp.BranchDestination();
    wInst.tokens = cap;

    return wInst;
}

void QBeaEngine::setCodeFoldingManager(CodeFoldingHelper* CodeFoldingManager)
{
    mCodeFoldingManager = CodeFoldingManager;
}

void QBeaEngine::UpdateConfig()
{
    _tokenizer.UpdateConfig();
}
