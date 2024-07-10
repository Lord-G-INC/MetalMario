#pragma once

#include "Game/LiveActor/LiveActor.h"
#include "Game/LiveActor/PartsModel.h"

class MetalMario : public LiveActor {
public:
    MetalMario(const char *pName);
    void initPartsModels();
    virtual void initWithoutIter();
    virtual void control();
    virtual void attackSensor(HitSensor *, HitSensor *);
    void invalidateMetal();
    void invalidateElectricity();
    u32 mElapseTimer;
    u32 mWaterElapseTimer;
    bool mIsPlayerElectric;
    bool mIsSpinActive;
    bool mIsElectricWaterActive;
    u32 mSpinCount;
    PartsModel *mParts[6];
};

void createMetalPlayer();