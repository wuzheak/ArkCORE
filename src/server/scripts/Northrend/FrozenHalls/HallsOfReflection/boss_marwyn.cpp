/*
 * Copyright (C) 2005-2011 MaNGOS <http://www.getmangos.com/>
 *
 * Copyright (C) 2008-2011 Trinity <http://www.trinitycore.org/>
 *
 * Copyright (C) 2006-2011 ScriptDev2 <http://www.scriptdev2.com/>
 *
 * Copyright (C) 2010-2011 ProjectSkyfire <http://www.projectskyfire.org/>
 * 
 * Copyright (C) 2011 ArkCORE <http://www.arkania.net/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ScriptPCH.h"
#include "halls_of_reflection.h"

enum Yells {
	SAY_AGGRO = -1668060,
	SAY_SLAY_1 = -1668061,
	SAY_SLAY_2 = -1668062,
	SAY_DEATH = -1668063,
	SAY_CORRUPTED_FLESH_1 = -1668064,
	SAY_CORRUPTED_FLESH_2 = -1668065,
};

enum Spells {
	SPELL_OBLITERATE = 72360,
	SPELL_WELL_OF_CORRUPTION = 72362,
	SPELL_CORRUPTED_FLESH = 72363,
	SPELL_SHARED_SUFFERING = 72368,
};

enum Events {
	EVENT_NONE,
	EVENT_OBLITERATE,
	EVENT_WELL_OF_CORRUPTION,
	EVENT_CORRUPTED_FLESH,
	EVENT_SHARED_SUFFERING,
};

class boss_marwyn: public CreatureScript {
public:
	boss_marwyn() :
			CreatureScript("boss_marwyn") {
	}

	CreatureAI* GetAI(Creature* pCreature) const {
		return new boss_marwynAI(pCreature);
	}

	struct boss_marwynAI: public boss_horAI {
		boss_marwynAI(Creature *pCreature) :
				boss_horAI(pCreature, DATA_MARWYN_EVENT) {
			me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK,
					true);
			me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
		}

		void Reset() {
			boss_horAI::Reset();

			if (instance)
				instance->SetData(DATA_MARWYN_EVENT, NOT_STARTED);
		}

		void EnterCombat(Unit* /*who*/) {
			DoScriptText(SAY_AGGRO, me);
			if (instance)
				instance->SetData(DATA_MARWYN_EVENT, IN_PROGRESS);

			events.ScheduleEvent(EVENT_OBLITERATE, 30000); // TODO Check timer
			events.ScheduleEvent(EVENT_WELL_OF_CORRUPTION, 13000);
			events.ScheduleEvent(EVENT_CORRUPTED_FLESH, 20000);
			events.ScheduleEvent(EVENT_SHARED_SUFFERING, 20000); // TODO Check timer
		}

		void JustDied(Unit* /*killer*/) {
			_JustDied();
			DoScriptText(SAY_DEATH, me);

			if (instance)
				instance->SetData(DATA_MARWYN_EVENT, DONE);
		}

		void KilledUnit(Unit * /*victim*/) {
			DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
		}

		void UpdateAI(const uint32 diff) {
			// Return since we have no target
			if (!UpdateVictim())
				return;

			events.Update(diff);

			if (me->HasUnitState(UNIT_STAT_CASTING))
				return;

			switch (events.ExecuteEvent()) {
			case EVENT_OBLITERATE:
				DoCast(SPELL_OBLITERATE);
				events.ScheduleEvent(EVENT_OBLITERATE, 30000);
				break;
			case EVENT_WELL_OF_CORRUPTION:
				DoCast(SPELL_WELL_OF_CORRUPTION);
				events.ScheduleEvent(EVENT_WELL_OF_CORRUPTION, 13000);
				break;
			case EVENT_CORRUPTED_FLESH:
				DoScriptText(RAND(SAY_CORRUPTED_FLESH_1, SAY_CORRUPTED_FLESH_2),
						me);
				DoCast(SPELL_CORRUPTED_FLESH);
				events.ScheduleEvent(EVENT_CORRUPTED_FLESH, 20000);
				break;
			case EVENT_SHARED_SUFFERING:
				if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM))
					DoCast(pTarget, SPELL_SHARED_SUFFERING);
				events.ScheduleEvent(EVENT_SHARED_SUFFERING, 20000);
				break;
			}

			DoMeleeAttackIfReady();
		}
	};
};

void AddSC_boss_marwyn() {
	new boss_marwyn();
}
