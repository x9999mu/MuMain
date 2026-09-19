#pragma once

#include <list>
#include "Core/Globals/_types.h" // BuffStateMap, eBuffState, DWORD

// A master skill tree upgrade replaces its base buff with a new effect id, but
// the buff stays the same for the player - only stronger: Swell Life arrives as
// Greater Life Enhanced/Mastered, Expansion of Wizardry as Magic Circle
// Improved/Enhanced, and so on. Code asking about "the buff" has to accept
// every tier of it.
eBuffState GetBaseBuffState(eBuffState buff);

// Upgrade tier of a buff id: 0 for a base buff, 1/2 for its upgrades.
int GetBuffUpgradeTier(eBuffState buff);

SmartPointer(Buff);

class Buff
{
public:
    static BuffPtr Make();
    Buff();
    virtual ~Buff();

public:
    Buff& operator=(const Buff& buff);

public:
    void RegisterBuff(eBuffState buffstate);
    void RegisterBuff(std::list<eBuffState> buffstate);
    void UnRegisterBuff(eBuffState buffstate);
    void UnRegisterBuff(std::list<eBuffState> buffstate);

    bool isBuff();
    bool isBuff(eBuffState buffstate);
    // isBuff(), accepting every master skill tree upgrade of the buff as well.
    bool isBuffActive(eBuffState buffstate);
    const eBuffState isBuff(std::list<eBuffState> buffstatelist);
    void TokenBuff(eBuffState curbufftype);

public:
    const DWORD GetBuffSize();
    const eBuffState GetBuff(int iterindex);
    const DWORD GetBuffCount(eBuffState buffstate);
    void ClearBuff();
    bool IsEqualBuffType(IN int iBuffType, OUT wchar_t* szBuffName);

public:
    BuffStateMap m_Buff;
};

inline Buff& Buff::operator=(const Buff& buff)
{
    m_Buff = buff.m_Buff;
    return *this;
}
