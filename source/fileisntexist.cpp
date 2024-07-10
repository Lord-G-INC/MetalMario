#include "syati.h"

void printFileNameIfMissing(const char* fileName) {
	if (!MR::isFileExist(fileName, 0)) 
		OSPanic("FileRipper.cpp", 118, "File \"%s\" isn't exist.", fileName);
}

kmCall(0x804B1FE0, printFileNameIfMissing);