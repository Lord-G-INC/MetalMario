#pragma once

#include "revolution.h"
#include "Game/LiveActor/LiveActor.h"
#include "Game/MapObj/MorphItemObjNeo.h"

#define MORPHMETAL

class MorphItemNeoMetal : public MorphItemObjNeo {
public:
    MorphItemNeoMetal(const char *);
    virtual ~MorphItemNeoMetal();
    //virtual void init(const JMapInfoIter &);
    virtual u32 getPowerUp();
};
