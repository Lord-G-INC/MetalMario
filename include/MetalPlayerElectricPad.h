#pragma once

#include "Game/LiveActor/LiveActor.h"

class MetalPlayerElectricPad : public LiveActor {
public:
    MetalPlayerElectricPad(const char *pName);
    ~MetalPlayerElectricPad();
    virtual void init(const JMapInfoIter &rIter);
    virtual void attackSensor(HitSensor *, HitSensor *);
    //virtual bool receiveMsgPlayerAttack(u32, HitSensor *, HitSensor *);
    //virtual bool receiveMsgEnemyAttack(u32, HitSensor *, HitSensor *);
};