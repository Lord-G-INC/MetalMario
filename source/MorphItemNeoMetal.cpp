#include "syati.h"
#include "MorphItemNeoMetal.h"
#include "MetalMario.h"

MorphItemNeoMetal::MorphItemNeoMetal(const char *pName) : MorphItemObjNeo(pName) {
    createMetalPlayer();
};
MorphItemNeoMetal::~MorphItemNeoMetal() {};
u32 MorphItemNeoMetal::getPowerUp () {
    return 9;
}

void replaceMorphItemStr (LiveActor *pActor, JMapInfoIter &rIter, const char *pModelName, bool) {
    if (MR::isEqualString(pModelName, "END")) 
        MR::processInitFunction(pActor, rIter, "MorphItemNeoMetal", false);
    else 
        MR::processInitFunction(pActor, rIter, pModelName, false);
}
kmCall(0x802D222C, replaceMorphItemStr);