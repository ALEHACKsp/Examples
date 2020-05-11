import VMPTracingSub
import pykd
import os, collections,time,sys

DriverObject = 0
ImageBase = 0
NtImageEnd = 0

LogPath = [r"\return",r"\jump",r"\jumpR",r"\call"]

def InitTracer():
    global DriverObject
    global ImageBase
    global NtImageEnd

    NtModule    = pykd.module("nt")
    NtImageBase = NtModule.begin()
    NtImageEnd  = NtModule.end()

    pykd.dbgCommand("ba e1 IopLoadDriver+4bd")
    pykd.dbgCommand("ba e1 IopLoadDriver+4c2")
    pykd.go()

    while(1):
        regPath = pykd.dbgCommand("du /c40 @rdx+10")
        if "VmpDriver.vmp" in regPath:
            print "[*] Find VMP Driver"
            DriverObject = pykd.reg("rcx")
            print "\t[-] Driver Object : 0x{:X}".format(DriverObject)
            ImageBase =pykd.ptrPtr(DriverObject+0x18)    # DriverObject.DriverStart
            print "\t[-] ImageBase Address : 0x{:X}".format(ImageBase)
            VMPTracingSub.GetSectionInfo(ImageBase)
            EntryPoint = ImageBase + VMPTracingSub.EntryPoint_Off
            strEntryPoint = hex(EntryPoint).rstrip("L")
            pykd.dbgCommand("ba e1 "+strEntryPoint)
            pykd.go()
            pykd.dbgCommand("bc 2")
            return

        pykd.go()

def Tracer():
    global ImageBase
    print "[*] VMP Entrypoint\n\t[-] " + pykd.dbgCommand("u @rip l2")
    EndIopLoadDriver = pykd.getBp(1).getOffset()
    pykd.dbgCommand("eb KdDebuggerEnabled 0")
    count = 0
    while(1):
        ReturnLogPath = PathInform(LogPath[0])
        JumpLogPath = PathInform(LogPath[1])
        JumpRLogPath = PathInform(LogPath[2])
        CallLogPath = PathInform(LogPath[3])

        Disassem = pykd.disasm()
        Instruction = Disassem.instruction()
        CurrentOffset = pykd.reg("rip") - ImageBase
        CurrentInstruction = pykd.reg("rip")
        pCallStack = pykd.reg("rsp")

        # IopLoadDriver+4c2, End driver load
        if CurrentInstruction == EndIopLoadDriver:
            break

        # Another module
        CurrentSection = VMPTracingSub.GetSectionName(CurrentInstruction)
        if CurrentSection == "Not Found Section":
            print "[*] Check Log.."
            pykd.dbgCommand("pt")
            continue

        if "call" in Instruction:
            CallLog = open(CallLogPath,'a+')
            CurrentSection = VMPTracingSub.GetSectionName(CurrentInstruction)

            # Call register
            if "call    r" in Instruction:
                idx = Instruction.find("call    r")
                reg = Instruction[idx+8:]
                regOffset = pykd.reg(reg)-ImageBase
                data = "\n[*] Call Register\n\t[*] Current Section : %s\n\t[*] Current Instruction offset : %X\n\t[-] Count : %d\n\t[-] Registry : %s(Offset : %X, Value : %X)\n\n[*] Current Instruction : %s\n\n"%(CurrentSection,CurrentOffset,count+1,reg,regOffset,pykd.reg(reg),Instruction)
                CallLog.write(data)
                CallLog.write(pykd.dbgCommand("r"))
                CallLog.write("\n\n[*] Current Disassembly\n\n")
                CallLog.write(pykd.dbgCommand("u @"+reg+" L10"))
                CallLog.close()
                pykd.dbgCommand("th")
                count+=1
                continue
            # Call address
            else:
                data = "\n[*] Call Instruction\n\t[*] Current Section : %s\n\t[*] Current Instruction Offset : %X\n\t[-] Count : %d\n\n[*] Current Instruction :%s\n\n"%(CurrentSection,CurrentOffset,count+1,Instruction)
                CallLog.write(data)
                CallLog.write(pykd.dbgCommand("r"))
                CallLog.write("\n\n[*] Current Disassembly\n\n")
                CallLog.write(pykd.dbgCommand("u @rip L5"))
                CallLog.close()
                pykd.dbgCommand("th")
                count+=1
                continue

        if "ret" in Instruction:
            ReturnLog = open(ReturnLogPath,'a+')
            CallStack = pykd.ptrPtr(pCallStack)
            CallStackOffset = CallStack - ImageBase
            CurrentSection = VMPTracingSub.GetSectionName(CurrentInstruction)
            returnSection = VMPTracingSub.GetSectionName(CallStack)

            data = "\n[*] Return Instruction\n\t[*] Current Section : %s\n\t[*] Return Section : %s\n\t[+] Current Instruction Offset : %X \n\t[-] Count :%d\n\t[*] Disassembly Offset : %X\n\n"%(CurrentSection,returnSection,CurrentOffset,count+1,CallStackOffset)
            ReturnLog.write(data)
            ReturnLog.write("\n")
            ReturnLog.write(pykd.dbgCommand("r"))
            ReturnLog.write("\n\n[*] Return Disassembly\n")
            ReturnLog.write(pykd.dbgCommand("u poi(@rsp) L10"))
            ReturnLog.close()
            pykd.dbgCommand("th")
            count+=1
            continue

        pykd.dbgCommand("th")
        count+=1

    return

def PathInform(Path):
    dirPath = r"C:\VMPTracingLog"
    if not os.path.isdir(dirPath):
        os.mkdir(dirPath)
    i = 0
    while(1):
        LogPath = dirPath + Path + "%.2d"%i + ".log"
        if os.path.exists(LogPath):
            if os.stat(LogPath).st_size >= 20971520:
                i+=1
                continue

            return LogPath

        else:
            return LogPath

if __name__ == '__main__':
    print "[*] Shh0ya VMP Tracer"
    if len(sys.argv) != 2:
        print "[*] Usage : VMPTracing.py <path>"
        exit()
    VMPTracingSub.PEParser(sys.argv[1])
    InitTracer()
    print "[*] Initialize Complete"
    Tracer()
    exit()
