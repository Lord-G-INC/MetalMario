#include "syati.h"
#include "MetalMario.h"
#include "MetalPlayerElectricPad.h"
MetalMario *gMetalPlayer;

extern "C" {
    void changePlayerModeRock__2MRFv();
    void removeAllClingingKarikari__2MRFv();
    void *getSubBgm__7AudWrapFv();
}

asm void setSubBGMState (s32 state, u32 fadeTime) {
    stwu      r1, -0x10(r1)
    mflr      r0
    stw       r0, 0x14(r1)
    stw       r31, 0xC(r1)
    mr        r31, r4
    stw       r30, 8(r1)
    mr        r30, r3
    bl        getSubBgm__7AudWrapFv
    cmpwi     r3, 0
    beq       End
    lwz       r12, 0(r3)
    mr        r4, r30
    mr        r5, r31
    lwz       r12, 0x30(r12)
    mtctr     r12
    bctrl
End:    
    lwz       r0, 0x14(r1)
    lwz       r31, 0xC(r1)
    lwz       r30, 8(r1)
    mtlr      r0
    addi      r1, r1, 0x10
    blr
}

void invalidateMetalOnStarGet (NerveExecutor *pExecutor, const Nerve *pNerve) {
    gMetalPlayer->invalidateMetal();
    pExecutor->setNerve(pNerve);
}
kmBranch(0x80451AF4, invalidateMetalOnStarGet);

bool disableWaterPhysics (Mario *pMario) {
    // Usually the game only checks for status 6, but here we also need to check for mode 14 (Metal Mario).
    return pMario->isStatusActive(6) || MarioAccess::getPlayerActor()->mPlayerMode == 14;
}
kmCall(0x803AB198, disableWaterPhysics);

void disableFireDamage (MarioActor *pMarioActor, u16 type) {
    if (pMarioActor->mPlayerMode != 14) 
        pMarioActor->decLife(type);
}
kmCall(0x80394990, disableFireDamage);

void slowPlayerUnderwater () {
    if (MR::isPlayerInAreaObj("WaterArea")) 
        MarioAccess::getPlayerActor()->mVelocity *= 0.6;
}
kmBranch(0x803C1C5C, slowPlayerUnderwater);

void createMetalPlayer () {
    gMetalPlayer = new MetalMario("MetalMario");
    gMetalPlayer->initWithoutIter();
}

bool electrifyPlayer (HitSensor *pReceiver, u32 msg, HitSensor *pSender) {
    if (MR::isSensorPlayer(pReceiver)) {
        MarioActor *ma = MarioAccess::getPlayerActor();
        if (ma->mPlayerMode == 14) {
            gMetalPlayer->mSpinCount = 0;
            MR::startSoundPlayer("SE_EM_LV_PETO_ELECTRIC_ATK", -1, -1);
            setSubBGMState(2, 0);
            gMetalPlayer->mIsPlayerElectric = true;
            for (int i = 0; i < 6; i++) 
                gMetalPlayer->mParts[i]->mScale = TVec3f(0.75);
            return false;
        } else 
            return pReceiver->receiveMessage(ACTMES_ENEMY_ATTACK_ELECTRIC, pSender);
    } else 
        return pReceiver->receiveMessage(ACTMES_ENEMY_ATTACK_ELECTRIC, pSender);
}
kmBranch(0x80016198, electrifyPlayer);

void preventDamage () {
    MarioActor *ma = MarioAccess::getPlayerActor();
    if (ma->mPlayerMode == 14) 
        ma->incLife(1);
}
kmBranch(0x803B9144, preventDamage);

bool attackEnemies (HitSensor *pReceiver, u32 msg, HitSensor *pSender) {
    if (MR::isSensorPlayer(pReceiver)) {
        MarioActor *ma = MarioAccess::getPlayerActor();
        if (ma->mPlayerMode == 14) {
            pSender->mActor->receiveMsgPlayerAttack(ACTMES_INVINCIBLE_ATTACK, pReceiver, pSender);
            pSender->mActor->receiveMsgPlayerAttack(ACTMES_PLAYER_PUNCH, pReceiver, pSender);
            return true;
        } else 
            return pReceiver->receiveMessage(ACTMES_ENEMY_ATTACK, pSender);
    } else 
        return pReceiver->receiveMessage(ACTMES_ENEMY_ATTACK, pSender);
}
kmBranch(0x80016058, attackEnemies);

void initMetalPlayer () {
    MarioActor *ma = MarioAccess::getPlayerActor();
    ma->mScale = TVec3f(0);
    gMetalPlayer->mScale = TVec3f(1);
    ma->setPlayerMode(14, true, true);
}

void validateMetalPlayer () {
    MR::startSubBGM("MBGM_SMG2_GALAXY_01", false);
    removeAllClingingKarikari__2MRFv();
    gMetalPlayer->mElapseTimer = 2400;
}

asm void hookingMoment () {
    blelr
    cmplwi r27, 0xE
    beq validateMetal
    cmplwi r27, 0xD
    beq validateRock
    blr
validateRock:
    li r27, 0xC
    blr
validateMetal:
    bl validateMetalPlayer
    cmplwi r27, 0xC
    lis r15, 0x803C
    ori r15, r15, 0xBA78
    mtlr r15
    blr
}
kmCall(0x803CB57C, hookingMoment);

// Here we hook right after a cmpwi instruction that checks for Item #8 (Rock Mushroom)
asm void checkForMetal () {
    beq RockMushroom
MetalMushroom:
    b initMetalPlayer
RockMushroom:
    b changePlayerModeRock__2MRFv
}
kmCall(0x802D341C, checkForMetal);

// So you might be wondering what this horrendous function does.
// Well, the game has a hardcoded table at 0x806C7780 for all power up names.
// The game checks this table every frame ???
// It is not possible to add a new entry to this table, and thus the Metal Mushroom will result in an invalid string
// that would usually crash the game.
// That is why this function exists.
bool replaceHstForMetal (HashSortTable *hst, const char *searchStr, u32 *destination) {
    u32 hstCheckStr = (u32)searchStr;
    if (hstCheckStr % 4 || hstCheckStr >= 0x81000000) 
        return hst->search("Normal", destination);
    else 
        return hst->search(searchStr, destination);
}
kmCall(0x80403C64, replaceHstForMetal);

void playMetalSound (LiveActor *pActor, const char *soundName, s32 pitch, s32 velocity, s32 volume) {}
kmCall(0x802D33B0, playMetalSound);

class CollisionCode {
public:
    u32 getSoundCode(const JMapInfoIter &);
};

u32 setSoundCodeToMetal (CollisionCode *pCode, const JMapInfoIter &rIter) {
    u32 soundCode = pCode->getSoundCode(rIter);
    return MarioAccess::getPlayerActor()->mPlayerMode == 14 ? 7 : soundCode;
}
kmCall(0x80039840, setSoundCodeToMetal);

// Coin HitSensors have the type ATYPE_COIN, which does not interact with our custom sensor.
// Therefore, we change their type to ATYPE_MAP_OBJ.
kmWrite32(0x8028BDBC, 0x38A00040); // li r5, 0x40

// ----------------

MetalMario::MetalMario(const char *pName) : LiveActor(pName) {
    mIsSpinActive = false;
    mIsPlayerElectric = false;
    mIsElectricWaterActive = false;
}

void MetalMario::initPartsModels () {
    mParts[0] = MR::createPartsModelMapObj(this, "MetalMarioBall", "MetalMarioBall", MR::getJointMtx(this, "HandL"));
    mParts[0]->initFixedPosition(TVec3f(0), TVec3f(0), "HandL");
    mParts[0]->mScale = TVec3f(0);
    mParts[1] = MR::createPartsModelMapObj(this, "MetalMarioBall", "MetalMarioBall", MR::getJointMtx(this, "HandR"));
    mParts[1]->initFixedPosition(TVec3f(0), TVec3f(0), "HandR");
    mParts[1]->mScale = TVec3f(0);
    mParts[2] = MR::createPartsModelMapObj(this, "MetalMarioBall", "MetalMarioBall", MR::getJointMtx(this, "FootL"));
    mParts[2]->initFixedPosition(TVec3f(0), TVec3f(0), "FootL");
    mParts[2]->mScale = TVec3f(0);
    mParts[3] = MR::createPartsModelMapObj(this, "MetalMarioBall", "MetalMarioBall", MR::getJointMtx(this, "FootR"));
    mParts[3]->initFixedPosition(TVec3f(0), TVec3f(0), "FootR");
    mParts[3]->mScale = TVec3f(0);
    mParts[4] = MR::createPartsModelMapObj(this, "MetalMarioBall", "MetalMarioBall", MR::getJointMtx(this, "Center"));
    mParts[4]->initFixedPosition(TVec3f(0), TVec3f(0), "Center");
    mParts[4]->mScale = TVec3f(0);
    mParts[5] = MR::createPartsModelMapObj(this, "MetalMarioBall", "MetalMarioBall", MR::getJointMtx(this, "Head"));
    mParts[5]->initFixedPosition(TVec3f(0), TVec3f(0), "Head");
    mParts[5]->mScale = TVec3f(0);
}

void MetalMario::initWithoutIter() {
    MR::processInitFunctionWithAnimArc(this, "MetalMario", "MarioAnime", false);
    initPartsModels();
    MR::connectToSceneMapObj(this);
    initHitSensor(3);
    MR::addHitSensor(this, "ElectricAttackWater", ATYPE_RECEIVER, 32, 2000.0f, TVec3f(0));
    MR::addHitSensor(this, "AttackSpin", ATYPE_RECEIVER, 32, 500.0f, TVec3f(0));
    MR::addHitSensor(this, "CoinPullSensor", ATYPE_RECEIVER, 32, 500.0f, TVec3f(0));
    MR::invalidateHitSensor(getSensor("ElectricAttackWater"));
    MR::invalidateHitSensor(getSensor("AttackSpin"));
    MR::invalidateHitSensor(getSensor("CoinPullSensor"));
    LiveActor::makeActorAppeared();
    MR::invalidateClipping(this);
    MR::onCalcGravity(this);
    this->mScale = TVec3f(0);
}

void MetalMario::control() {
    MarioActor *ma = MarioAccess::getPlayerActor();
    if (ma->mPlayerMode == 14) {
        MarioAccess::getPlayerActor()->mMario->resetSleepTimer();
        switch (--mElapseTimer) {
            case 0:
                invalidateMetal();
                break;
            case 2399:
                MR::startAction(mParts[0], "Anim");
                break;
            case 2394:
                MR::startAction(mParts[1], "Anim");
                break;
            case 2389:
                MR::startAction(mParts[2], "Anim");
                break;
            case 2384:
                MR::startAction(mParts[3], "Anim");
                break;
            case 2379:
                MR::startAction(mParts[4], "Anim");
                break;
            case 2374:
                MR::startAction(mParts[5], "Anim");
                break;
        }

        if (MR::isPlayerInAreaObj("WaterArea")) {
            if (mIsPlayerElectric && !mIsElectricWaterActive) {
                // KABOOM!
                MR::startSoundPlayer("SE_PM_ELEC_DAMAGE", -1, -1);
                mIsElectricWaterActive = true;
                MR::validateHitSensor(this, "ElectricAttackWater");
                mWaterElapseTimer = 60;
            }
            if (mWaterElapseTimer) 
                mWaterElapseTimer--;
            else 
                invalidateElectricity();
        }

        if (MarioAccess::isSwingAction() && mIsPlayerElectric) {
            mIsSpinActive = true;
            MR::validateHitSensor(this, "AttackSpin");
        } else if (!MarioAccess::isSwingAction() && mIsSpinActive) {
            mIsSpinActive = false;
            MR::invalidateHitSensor(this, "AttackSpin");
            switch (++mSpinCount) {
                case 1:
                    mParts[0]->mScale = TVec3f(0);
                    mParts[1]->mScale = TVec3f(0);
                    break;
                case 2:
                    mParts[2]->mScale = TVec3f(0);
                    mParts[3]->mScale = TVec3f(0);
                    break;
                case 3:
                    mParts[4]->mScale = TVec3f(0);
                    mParts[5]->mScale = TVec3f(0);
                    invalidateElectricity();
                    break;
            }
        }
    }
    MR::setPosition(this, *MR::getPlayerPos());
    MR::setRotation(this, *MR::getPlayerRotate());

    // For some reason, getPlayingBckName returns WalkSoft for normal walking,
    // soft walking and waiting, so we need to distinguish. 
    // We will ignore soft walking because idk how this game works.
    if (MR::isNullOrEmptyString(this->mModelManager->getPlayingBckName()) || !MR::isEqualString(ma->mModelManager->getPlayingBckName(), this->mModelManager->getPlayingBckName()) && !MR::isEqualString(ma->mModelManager->getPlayingBckName(), "WalkSoft")) 
        MR::startBck(this, ma->mModelManager->getPlayingBckName(), 0);
    else if (MR::isEqualString(ma->mModelManager->getPlayingBckName(), "WalkSoft")) {
        if (MarioAccess::getVelocity()->isZero() && !MR::isEqualString(this->mModelManager->getPlayingBckName(), "Wait")) 
            MR::startBck(this, "Wait", 0);
        else if (!MarioAccess::getVelocity()->isZero() && !MR::isEqualString(this->mModelManager->getPlayingBckName(), "Walk")) {
            MR::startBck(this, "Walk", 0);
        }
    }
    MR::setBckRate(this, MR::getBckRate(ma));
}

void MetalMario::attackSensor (HitSensor *pSender, HitSensor *pReceiver) {
    if (mIsPlayerElectric && MR::isEqualSubString(pReceiver->mActor->mName, "ƒRƒCƒ“")) { // Coin
        TVec3f newPos = (pReceiver->mPosition - MarioAccess::getPlayerActor()->mTranslation) * 0.6f + MarioAccess::getPlayerActor()->mTranslation;
        MR::setPosition(pReceiver->mActor, newPos);
    }
    if (mIsElectricWaterActive) {
        if (MR::isInWater(pReceiver->mActor, TVec3f(0))) {
            pReceiver->mActor->receiveMsgPlayerAttack(ACTMES_INVINCIBLE_ATTACK, pSender, pReceiver);
            pReceiver->mActor->receiveMsgPlayerAttack(ACTMES_PLAYER_PUNCH, pSender, pReceiver);
        }
    }
    pReceiver->mActor->receiveMsgPlayerAttack(ACTMES_INVINCIBLE_ATTACK, pSender, pReceiver);
    pReceiver->mActor->receiveMsgPlayerAttack(ACTMES_PLAYER_PUNCH, pSender, pReceiver);
}

void MetalMario::invalidateMetal () {
    invalidateElectricity();
    MarioActor *ma = MarioAccess::getPlayerActor();
    MR::stopSubBGM(45);
    ma->mScale = TVec3f(1);
    mScale = TVec3f(0);
    ma->setPlayerMode(0, true, true);
}

void MetalMario::invalidateElectricity () {
    setSubBGMState(1, 0);
    mIsPlayerElectric = false;
    mIsElectricWaterActive = false;
    mSpinCount = 0;
    for (int i = 0; i < 6; i++) 
        mParts[i]->mScale = TVec3f(0);
    MR::stopSoundPlayer("SE_EM_LV_PETO_ELECTRIC_ATK", 0);
    MR::invalidateHitSensor(this, "ElectricAttackWater");
    MR::invalidateHitSensor(this, "CoinPullSensor");
}

// ----------------

MetalPlayerElectricPad::MetalPlayerElectricPad(const char *pName) : LiveActor(pName) {}
MetalPlayerElectricPad::~MetalPlayerElectricPad() {}
void MetalPlayerElectricPad::init (const JMapInfoIter &rIter) {
    MR::processInitFunction(this, rIter, false);
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneMapObj(this);
    makeActorAppeared();
    initHitSensor(1);
    MR::addHitSensorMapObj(this, "Body", 1, 50.0f, TVec3f(0, 0, 175)); // Definitely needs adjustment
    MR::validateHitSensors(this);
    MR::initCollisionParts(this, "MetalPlayerElectricPad", getSensor("Body"), 0);
    MR::validateCollisionParts(this);
    MR::initUseStageSwitchWriteA(this, rIter);
}
void MetalPlayerElectricPad::attackSensor (HitSensor *pReceiver, HitSensor *pSender) {
    if (MR::isSensorPlayer(pSender) && gMetalPlayer->mIsPlayerElectric) {
        gMetalPlayer->invalidateElectricity();
        MR::startBrk(this, "Enable");
        MR::onSwitchA(this);
    }
}