/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Janalai
SD%Complete: 75
SDComment:
SDCategory: Zul'Aman
EndScriptData */

#include "precompiled.h"
#include "def_zulaman.h"

enum
{
    SAY_AGGRO                       = -1568000,
    SAY_FIRE_BOMBS                  = -1568001,
    SAY_SUMMON_HATCHER              = -1568002,
    SAY_ALL_EGGS                    = -1568003,
    SAY_BERSERK                     = -1568004,
    SAY_SLAY_1                      = -1568005,
    SAY_SLAY_2                      = -1568006,
    SAY_DEATH                       = -1568007,
    SAY_EVENT_STRANGERS             = -1568008,
    SAY_EVENT_FRIENDS               = -1568009,

    //Jan'alai
    SPELL_FLAME_BREATH              = 43140,
    SPELL_FIRE_WALL                 = 43113,
    SPELL_ENRAGE                    = 44779,
    SPELL_TELETOCENTER              = 43098,
    SPELL_SUMMONALL                 = 43097,
    SPELL_BERSERK                   = 47008,
    SPELL_SUMMON_HATCHER_1          = 43962,
    SPELL_SUMMON_HATCHER_2          = 45340,

    //Fire Bob Spells
    SPELL_FIRE_BOMB_SUMMON          = 42622,
    SPELL_FIRE_BOMB_CHANNEL         = 42621,
    SPELL_FIRE_BOMB_THROW           = 42628,
    SPELL_FIRE_BOMB_DUMMY           = 42629,
    SPELL_FIRE_BOMB_DAMAGE          = 42630,

    //NPC's
    NPC_FIRE_BOMB                   = 23920,
    NPC_AMANI_HATCHER_1             = 23818,
    NPC_AMANI_HATCHER_2             = 24504,
    NPC_HATCHLING                   = 23598,

    //Hatcher Spells
    SPELL_HATCH_EGG                 = 43734,                //spell 42471 also exist
    SPELL_HATCH_ALL_EGGS            = 43144,

    //Hatchling Spells
    SPELL_FLAMEBUFFED               = 43299
};

const int area_dx = 44;
const int area_dy = 51;

float JanalainPos[1][3] =
{
    {-33.93, 1149.27, 19}
};

float FireWallCoords[4][4] =
{
    {-10.13, 1149.27, 19, M_PI},
    {-33.93, 1123.90, 19, 0.5*M_PI},
    {-54.80, 1150.08, 19, 0},
    {-33.93, 1175.68, 19, 1.5*M_PI}
};

struct WaypointDef
{
    float m_fX, m_fY, m_fZ;
};

WaypointDef m_aHatcherRight[]=
{
    {-86.203, 1136.834, 5.594},                             //this is summon point, not regular waypoint
    {-74.783, 1145.827, 5.420},
    {-56.957, 1146.713, 18.725},
    {-45.428, 1141.697, 18.709},
    {-34.002, 1124.427, 18.711},
    {-34.085, 1106.158, 18.711}
};

WaypointDef m_aHatcherLeft[]=
{
    {-85.420, 1167.321, 5.594},                             //this is summon point, not regular waypoint
    {-73.569, 1154.960, 5.510},
    {-56.985, 1153.373, 18.608},
    {-45.515, 1158.356, 18.709},
    {-33.314, 1174.816, 18.709},
    {-33.097, 1195.359, 18.709}
};

float hatcherway_l[5][3] =
{
    {-87.46,1170.09,6},
    {-74.41,1154.75,6},
    {-52.74,1153.32,19},
    {-33.37,1172.46,19},
    {-33.09,1203.87,19}
};

float hatcherway_r[5][3] =
{
    {-86.57,1132.85,6},
    {-73.94,1146.00,6},
    {-52.29,1146.51,19},
    {-33.57,1125.72,19},
    {-34.29,1095.22,19}
};

struct MANGOS_DLL_DECL boss_janalaiAI : public ScriptedAI
{
    boss_janalaiAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_uiHatcher1GUID = 0;
        m_uiHatcher2GUID = 0;
        pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
        Reset();
    }

    ScriptedInstance *pInstance;

    uint32 fire_breath_timer;
    uint32 bomb_timer;
    uint32 throw_timer;
    uint32 enrage_timer;
    uint32 finishedbomb_timer;
    uint32 bombcounter;
    uint32 hatchertime;
    uint32 eggs;
    uint32 wipetimer;
    uint32 reset_timer;
    bool noeggs;
    bool enraged;
    bool enragetime;

    uint64 m_uiHatcher1GUID;
    uint64 m_uiHatcher2GUID;
    uint64 FireBombGUIDs[40];
    uint64 ThrowControllerGUID;

    bool bombing;

    void Reset()
    {
        if (Creature* pUnit = (Creature*)Unit::GetUnit(*m_creature, m_uiHatcher1GUID))
        {
            pUnit->AI()->EnterEvadeMode();
            pUnit->setDeathState(JUST_DIED);
            m_uiHatcher1GUID = 0;
        }

        if (Creature* pUnit = (Creature*)Unit::GetUnit(*m_creature, m_uiHatcher2GUID))
        {
            pUnit->AI()->EnterEvadeMode();
            pUnit->setDeathState(JUST_DIED);
            m_uiHatcher2GUID = 0;
        }

        if (pInstance)
            pInstance->SetData(TYPE_JANALAI, NOT_STARTED);

        fire_breath_timer = 8000;
        bomb_timer = 30000;
        enrage_timer = MINUTE*5*IN_MILISECONDS;
        finishedbomb_timer = 6000;
        throw_timer = 1000;
        bombcounter = 0;
        noeggs = false;
        hatchertime = 10000;
        wipetimer = 600000;                                 // 10 mins
        bombing =false;
        reset_timer = 5000;
        enraged = false;
        enragetime = false;

        ThrowControllerGUID = 0;

        for(uint8 i = 0; i < 40; i++)
            FireBombGUIDs[i] = 0;
    }

    void JustDied(Unit* Killer)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (pInstance)
            pInstance->SetData(TYPE_JANALAI, DONE);
    }

    void KilledUnit(Unit* victim)
    {
        switch(rand()%2)
        {
            case 0: DoScriptText(SAY_SLAY_1, m_creature); break;
            case 1: DoScriptText(SAY_SLAY_2, m_creature); break;
        }
    }

    void Aggro(Unit *who)
    {
        DoScriptText(SAY_AGGRO, m_creature);

        if (pInstance)
            pInstance->SetData(TYPE_JANALAI, IN_PROGRESS);
    }

    void JustSummoned(Creature* pSummoned)
    {
        switch(pSummoned->GetEntry())
        {
            case NPC_AMANI_HATCHER_1:
                m_uiHatcher1GUID = pSummoned->GetGUID();
                break;
            case NPC_AMANI_HATCHER_2:
                m_uiHatcher2GUID = pSummoned->GetGUID();
                break;
        }
    }

    void FireWall()                                         // Create Firewall
    {
        Creature* wall = NULL;
        wall = m_creature->SummonCreature(NPC_FIRE_BOMB,FireWallCoords[0][0],FireWallCoords[0][1],FireWallCoords[0][2],FireWallCoords[0][3],TEMPSUMMON_TIMED_DESPAWN,11500);
        if (wall)
            wall->CastSpell(wall,SPELL_FIRE_WALL,false);

        wall = m_creature->SummonCreature(NPC_FIRE_BOMB,FireWallCoords[0][0],FireWallCoords[0][1]+5,FireWallCoords[0][2],FireWallCoords[0][3],TEMPSUMMON_TIMED_DESPAWN,11500);
        if (wall)
            wall->CastSpell(wall,SPELL_FIRE_WALL,false);

        wall = m_creature->SummonCreature(NPC_FIRE_BOMB,FireWallCoords[0][0],FireWallCoords[0][1]-5,FireWallCoords[0][2],FireWallCoords[0][3],TEMPSUMMON_TIMED_DESPAWN,11500);
        if (wall)
            wall->CastSpell(wall,SPELL_FIRE_WALL,false);

        wall = m_creature->SummonCreature(NPC_FIRE_BOMB,FireWallCoords[1][0]-2,FireWallCoords[1][1]-2,FireWallCoords[1][2],FireWallCoords[1][3],TEMPSUMMON_TIMED_DESPAWN,11500);
        if (wall)
            wall->CastSpell(wall,SPELL_FIRE_WALL,false);

        wall = m_creature->SummonCreature(NPC_FIRE_BOMB,FireWallCoords[1][0]+2,FireWallCoords[1][1]+2,FireWallCoords[1][2],FireWallCoords[1][3],TEMPSUMMON_TIMED_DESPAWN,11500);
        if (wall)
            wall->CastSpell(wall,SPELL_FIRE_WALL,false);

        wall = m_creature->SummonCreature(NPC_FIRE_BOMB,FireWallCoords[2][0],FireWallCoords[2][1],FireWallCoords[2][2],FireWallCoords[2][3],TEMPSUMMON_TIMED_DESPAWN,11500);
        if (wall)
            wall->CastSpell(wall,SPELL_FIRE_WALL,false);

        wall = m_creature->SummonCreature(NPC_FIRE_BOMB,FireWallCoords[2][0],FireWallCoords[2][1]-5,FireWallCoords[2][2],FireWallCoords[2][3],TEMPSUMMON_TIMED_DESPAWN,11500);
        if (wall)
            wall->CastSpell(wall,SPELL_FIRE_WALL,false);

        wall = m_creature->SummonCreature(NPC_FIRE_BOMB,FireWallCoords[2][0],FireWallCoords[2][1]+5,FireWallCoords[2][2],FireWallCoords[2][3],TEMPSUMMON_TIMED_DESPAWN,11500);
        if (wall)
            wall->CastSpell(wall,SPELL_FIRE_WALL,false);

        wall = m_creature->SummonCreature(NPC_FIRE_BOMB,FireWallCoords[3][0]-2,FireWallCoords[3][1],FireWallCoords[3][2],FireWallCoords[3][3],TEMPSUMMON_TIMED_DESPAWN,11500);
        if (wall)
            wall->CastSpell(wall,SPELL_FIRE_WALL,false);

        wall = m_creature->SummonCreature(NPC_FIRE_BOMB,FireWallCoords[3][0]+2,FireWallCoords[3][1],FireWallCoords[3][2],FireWallCoords[3][3],TEMPSUMMON_TIMED_DESPAWN,11500);
        if (wall)
            wall->CastSpell(wall,SPELL_FIRE_WALL,false);
    }

    void throwBombs()                                       // create Bombs
    {
        float dx;
        float dy;
        for (int i(0); i < 40; i++)
        {
            dx = (rand()%(area_dx))-(area_dx/2);
            dy = (rand()%(area_dy))-(area_dy/2);

            Creature* bomb = DoSpawnCreature(NPC_FIRE_BOMB, dx, dy, 0, 0, TEMPSUMMON_TIMED_DESPAWN, 13000);
            if (bomb)
                FireBombGUIDs[i] = bomb->GetGUID();
        }

        Creature* ThrowController = DoSpawnCreature(NPC_FIRE_BOMB,0,0,1,0,TEMPSUMMON_TIMED_DESPAWN,10000);
        if (ThrowController)
            ThrowControllerGUID = ThrowController->GetGUID();

        bombcounter = 0;
    }

    void throw5Bombs()                                      //throwanimation
    {
        for (int i(0); i < 5; i++)
        {
            Unit* ThrowController = NULL;
            Unit* FireBomb = NULL;
            if (ThrowControllerGUID)
                ThrowController = Unit::GetUnit((*m_creature), ThrowControllerGUID);

            if (FireBombGUIDs[bombcounter])
                FireBomb = Unit::GetUnit((*m_creature), FireBombGUIDs[bombcounter]);

            if (ThrowController && FireBomb)
            {
                ThrowController->CastSpell(FireBomb,SPELL_FIRE_BOMB_THROW,true);
                FireBomb->CastSpell(FireBomb,SPELL_FIRE_BOMB_DUMMY,false);
                bombcounter ++;
            }
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;

        if (!bombing)                                        // every Spell if NOT Bombing
        {
            //FIRE BREATH  several videos says every 8Secounds
            if (fire_breath_timer < diff)
            {
                if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                    DoCast(target,SPELL_FLAME_BREATH);
                fire_breath_timer = 8000;
            }else fire_breath_timer -=diff;

            if (bomb_timer < diff)
            {
                DoScriptText(SAY_FIRE_BOMBS, m_creature);

                FireWall();
                bomb_timer = 20000+rand()%20000;
                m_creature->Relocate(JanalainPos[0][0],JanalainPos[0][1],JanalainPos[0][2],0);
                m_creature->SendMonsterMove(JanalainPos[0][0], JanalainPos[0][1],JanalainPos[0][2],0,0,100);

                throwBombs();
                bombing = true;

                //Teleport every Player into the middle
                Unit* Temp = NULL;
                std::list<HostilReference*>::iterator i = m_creature->getThreatManager().getThreatList().begin();
                for (; i != m_creature->getThreatManager().getThreatList().end(); ++i)
                {
                    Temp = Unit::GetUnit((*m_creature),(*i)->getUnitGuid());
                    if (Temp && m_creature->IsWithinDist(Temp, 20.0f))
                        DoTeleportPlayer(Temp, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0);
                }

                DoCast(m_creature,SPELL_TELETOCENTER,true); // only Effect Spell
                DoCast(m_creature,SPELL_FIRE_BOMB_CHANNEL,false);
                finishedbomb_timer = 11000;
            }else bomb_timer -=diff;

            //enrage if under 25% hp before 5 min.
            if (((m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) < 25) && !enraged)
            {
                enragetime = true;
                enrage_timer = 600000;
            }

            //Enrage but only if not bombing
            if (enragetime && !enraged)
            {
                DoScriptText(SAY_BERSERK, m_creature);

                m_creature->InterruptNonMeleeSpells(false);
                DoCast(m_creature,SPELL_ENRAGE);
                enraged = true;
            }
        }

        //Enrage after 5 minutes
        if (enrage_timer < diff)
        {
            enragetime = true;
            enrage_timer = 600000;
        }else enrage_timer -=diff;

        if (bombing)                                        // every Spell if Bombing
        {
            if (bombcounter < 40)
            {
                if (throw_timer < diff)
                {
                    throw5Bombs();
                    throw_timer = 1000;
                }else throw_timer -=diff;
            }

            if (finishedbomb_timer < diff)
            {
                bombing = false;
                finishedbomb_timer = 6000;
                m_creature->RemoveAura(SPELL_FIRE_BOMB_CHANNEL,0);
                m_creature->RemoveAura(SPELL_FIRE_BOMB_CHANNEL,1);
            }else finishedbomb_timer -=diff;
        }

        //Call Hatcher
        if (!noeggs)
        {
            if (hatchertime < diff)
            {
                if (pInstance->GetData(DATA_J_EGGSLEFT) > 0 || pInstance->GetData(DATA_J_EGGSRIGHT) > 0)
                {
                    DoScriptText(SAY_SUMMON_HATCHER, m_creature);

                    Unit* pHatcer1 = Unit::GetUnit(*m_creature, m_uiHatcher1GUID);
                    Unit* pHatcer2 = Unit::GetUnit(*m_creature, m_uiHatcher2GUID);

                    if (!pHatcer1 || (pHatcer1 && !pHatcer1->isAlive()))
                    {
                        if (Creature* pHatcher = m_creature->SummonCreature(NPC_AMANI_HATCHER_1, m_aHatcherRight[0].m_fX, m_aHatcherRight[0].m_fY, m_aHatcherRight[0].m_fZ, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 0))
                            pHatcher->GetMotionMaster()->MovePoint(1, m_aHatcherRight[1].m_fX, m_aHatcherRight[1].m_fY, m_aHatcherRight[1].m_fZ);
                    }

                    if (!pHatcer2 || (pHatcer2 && !pHatcer2->isAlive()))
                    {
                        if (Creature* pHatcher = m_creature->SummonCreature(NPC_AMANI_HATCHER_2, m_aHatcherLeft[0].m_fX, m_aHatcherLeft[0].m_fY, m_aHatcherLeft[0].m_fZ, 0.0f, TEMPSUMMON_CORPSE_DESPAWN, 0))
                            pHatcher->GetMotionMaster()->MovePoint(1, m_aHatcherLeft[1].m_fX, m_aHatcherLeft[1].m_fY, m_aHatcherLeft[1].m_fZ);
                    }

                    hatchertime = 90000;
                }
                else
                {
                    noeggs = true;
                }
            }else hatchertime -=diff;
        }

        //WIPE after 10 minutes
        if (wipetimer < diff)
        {
            DoScriptText(SAY_BERSERK, m_creature);
            DoCast(m_creature,SPELL_ENRAGE);
            wipetimer = 30000;
        }else wipetimer -=diff;

        //Hatch All
        if (!noeggs && (m_creature->GetHealth()*100) / m_creature->GetMaxHealth() < 35)
        {
            DoScriptText(SAY_ALL_EGGS, m_creature);

            if (pInstance)
                eggs = pInstance->GetData(DATA_J_EGGSLEFT);

            int i;
            for(i=1;i<=eggs;i=i+1)
            {
                int r = (rand()%20 - 10);
                int s = (rand()%20 - 10);
                m_creature->SummonCreature(NPC_HATCHLING,JanalainPos[0][0]+s,JanalainPos[0][1]+r,JanalainPos[0][2],0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,15000);

                if (pInstance)
                    pInstance->SetData(DATA_J_HATCHLEFT,1);
            }

            if (pInstance)
                eggs = pInstance->GetData(DATA_J_EGGSRIGHT);

            for(i=1;i<=eggs;i=i+1)
            {
                int r = (rand()%20 - 10);
                int s = (rand()%20 - 10);
                m_creature->SummonCreature(NPC_HATCHLING,JanalainPos[0][0]+s,JanalainPos[0][1]+r,JanalainPos[0][2],0,TEMPSUMMON_CORPSE_TIMED_DESPAWN,15000);
                if (pInstance)
                    pInstance->SetData(DATA_J_HATCHRIGHT,1);
            }

            noeggs = true;
        }

        //check for reset ... exploit preventing ... pulled from his podest
        if (reset_timer < diff)
        {
            if (m_creature->GetPositionX() < -70 || m_creature->GetPositionX() > 0)
            {
                EnterEvadeMode();
                reset_timer = 5000;                         //every 5 Seca
            }
        }else reset_timer -=diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_janalaiAI(Creature* pCreature)
{
    return new boss_janalaiAI(pCreature);
}

struct MANGOS_DLL_DECL mob_jandalai_firebombAI : public ScriptedAI
{
    mob_jandalai_firebombAI(Creature* pCreature) : ScriptedAI(pCreature){Reset();}

    uint32 bomb_timer;

    void Reset()
    {
        bomb_timer = 12000;
    }

    void AttackStart(Unit* who) {}

    void MoveInLineOfSight(Unit* who) {}

    void UpdateAI(const uint32 diff)
    {
        if (bomb_timer < diff)                               //Boom
        {
            m_creature->CastSpell(m_creature,SPELL_FIRE_BOMB_DAMAGE,false);
            bomb_timer = 1800000;
        }else bomb_timer -=diff;
    }
};

CreatureAI* GetAI_mob_jandalai_firebombAI(Creature* pCreature)
{
    return new mob_jandalai_firebombAI(pCreature);
}

struct MANGOS_DLL_DECL mob_amanishi_hatcherAI : public ScriptedAI
{
    mob_amanishi_hatcherAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiWaypoint;
    uint32 m_uiHatchlingTimer;
    uint32 m_uiHatchlingCount;
    bool m_bCanMoveNext;
    bool m_bWaypointEnd;

    void Reset()
    {
        m_uiWaypoint = 0;
        m_uiHatchlingTimer = 1000;
        m_uiHatchlingCount = 1;
        m_bCanMoveNext = false;
        m_bWaypointEnd = false;

        if (m_creature->HasUnitMovementFlag(MOVEMENTFLAG_WALK_MODE))
            m_creature->RemoveUnitMovementFlag(MOVEMENTFLAG_WALK_MODE);
    }

    void MoveInLineOfSight(Unit* pWho) {}

    void AttackStart(Unit* pWho)
    {
        if (!pWho)
            return;

        if (m_creature->Attack(pWho, false))
        {
            m_creature->AddThreat(pWho, 0.0f);
            m_creature->SetInCombatWith(pWho);
            pWho->SetInCombatWith(m_creature);
        }
    }

    void MovementInform(uint32 uiType, uint32 uiPointId)
    {
        if (uiType != POINT_MOTION_TYPE || m_bWaypointEnd)
            return;

        uint32 uiCount = (m_creature->GetEntry() == NPC_AMANI_HATCHER_1) ?
            (sizeof(m_aHatcherRight)/sizeof(WaypointDef)) : (sizeof(m_aHatcherLeft)/sizeof(WaypointDef));

        m_uiWaypoint = uiPointId+1;

        if (uiCount == m_uiWaypoint)
            m_bWaypointEnd = true;

        m_bCanMoveNext = true;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_bCanMoveNext)
        {
            m_bCanMoveNext = false;

            if (m_bWaypointEnd)
                m_creature->GetMotionMaster()->Clear();
            else
            {
                if (m_creature->GetEntry() == NPC_AMANI_HATCHER_1)
                    m_creature->GetMotionMaster()->MovePoint(m_uiWaypoint, m_aHatcherRight[m_uiWaypoint].m_fX, m_aHatcherRight[m_uiWaypoint].m_fY, m_aHatcherRight[m_uiWaypoint].m_fZ);
                else
                    m_creature->GetMotionMaster()->MovePoint(m_uiWaypoint, m_aHatcherLeft[m_uiWaypoint].m_fX, m_aHatcherLeft[m_uiWaypoint].m_fY, m_aHatcherLeft[m_uiWaypoint].m_fZ);
            }
        }

        if (m_bWaypointEnd)
        {
            if (m_uiHatchlingTimer < uiDiff)
            {
                if (!m_pInstance)
                    return;

                uint32 uiEggsLeftCount = (m_creature->GetEntry() == NPC_AMANI_HATCHER_1) ?
                    m_pInstance->GetData(DATA_J_EGGSRIGHT) : m_pInstance->GetData(DATA_J_EGGSLEFT);

                if (uiEggsLeftCount > 0)
                    DoCast(m_creature,SPELL_HATCH_EGG);

                //This is initially wrong way
                //Spell above should hit a certain amount of eggs.
                //Those who are hit will then cast summon hatchling (which is also insta kill self)
                for(uint8 i = 0; i < m_uiHatchlingCount; ++i)
                {
                    DoSpawnCreature(NPC_HATCHLING, rand()%4-2, rand()%4-2, 0.0f, 0.0f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000);

                    uint32 uiSaveRightOrLeft = (m_creature->GetEntry() == NPC_AMANI_HATCHER_1) ?
                        DATA_J_HATCHRIGHT : DATA_J_HATCHLEFT;

                    m_pInstance->SetData(uiSaveRightOrLeft,1);
                }

                //as a workaround, this counter is also wrong way to count
                ++m_uiHatchlingCount;

                m_uiHatchlingTimer = 15000;
            }else m_uiHatchlingTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_mob_amanishi_hatcherAI(Creature* pCreature)
{
    return new mob_amanishi_hatcherAI(pCreature);
}

struct MANGOS_DLL_DECL mob_hatchlingAI : public ScriptedAI
{
    mob_hatchlingAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        pInstance = ((ScriptedInstance*)pCreature->GetInstanceData());
        Reset();
    }

    ScriptedInstance *pInstance;

    uint32 buffer_timer;
    uint32 delete_timer;
    bool start;

    void Reset()
    {
        buffer_timer = 7000;
        delete_timer = 10000;
        start = false;
    }

    void UpdateAI(const uint32 diff)
    {
        if (!start)
        {
            if (m_creature->GetPositionY() > 1150)
                m_creature->GetMotionMaster()->MovePoint(0, hatcherway_l[3][0]+rand()%4-2,hatcherway_l[3][1]+rand()%4-2,hatcherway_l[3][2]);
            else
                m_creature->GetMotionMaster()->MovePoint(0,hatcherway_r[3][0]+rand()%4-2,hatcherway_r[3][1]+rand()%4-2,hatcherway_r[3][2]);
            start = true;
        }

        if (delete_timer < diff && (pInstance && !(pInstance->GetData(TYPE_JANALAI) == IN_PROGRESS)))
        {
            if (!(m_creature->getVictim()))
                m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            delete_timer = 10000;
        }else delete_timer -=diff;

        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;

        if (buffer_timer < diff)
        {
            if (Unit* target = SelectUnit(SELECT_TARGET_RANDOM,0))
                DoCast(target,SPELL_FLAMEBUFFED);

            buffer_timer = 7000;
        }else buffer_timer -=diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_mob_hatchlingAI(Creature* pCreature)
{
    return new mob_hatchlingAI(pCreature);
}

void AddSC_boss_janalai()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name = "boss_janalai";
    newscript->GetAI = &GetAI_boss_janalaiAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_jandalai_firebomb";
    newscript->GetAI = &GetAI_mob_jandalai_firebombAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_amanishi_hatcher";
    newscript->GetAI = &GetAI_mob_amanishi_hatcherAI;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "mob_hatchling";
    newscript->GetAI = &GetAI_mob_hatchlingAI;
    newscript->RegisterSelf();
}
