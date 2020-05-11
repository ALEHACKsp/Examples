import pefile

sectionName = []
sectionVA_Off = []
sectionVS_Off = []
sectionVA = []
sectionVS = []
EntryPoint_Off = 0
staticBase = 0

def PEParser(path):
    targetFile = pefile.PE(path)
    global sectionName
    global sectionVA_Off
    global sectionVS_Off
    global sectionVA
    global sectionVS
    global EntryPoint_Off
    global staticBase

    EntryPoint_Off = targetFile.OPTIONAL_HEADER.AddressOfEntryPoint
    staticBase = targetFile.OPTIONAL_HEADER.ImageBase
    for section in targetFile.sections:
        if section.Characteristics & 0x20000000:
             sectionName.append(section.Name)
             sectionVA_Off.append(section.VirtualAddress)
             sectionVS_Off.append(section.Misc_VirtualSize)

    for i in range(len(sectionName)):
        sectionName[i]=sectionName[i].rstrip("\x00")

def GetSectionInfo(baseAddress):
    global sectionName
    global sectionVA_Off
    global sectionVS_Off

    for i in range(len(sectionName)):
        sectionVA.append(baseAddress+sectionVA_Off[i])
        sectionVS.append(baseAddress+sectionVA_Off[i]+sectionVS_Off[i])
    return

def GetSectionName(rip):
    global sectionName
    global sectionVA
    global sectionVS

    currentSection = ""
    for i in range(len(sectionName)):
        if rip > sectionVA[i] and rip < sectionVS[i]:
            return sectionName[i]

    if currentSection == "":
            currentSection = "Not Found Section"

    return currentSection
