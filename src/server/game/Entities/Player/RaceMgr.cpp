/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "RaceMgr.h"
#include "AccountMgr.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Optional.h"
#include "Player.h"
#include "StringConvert.h"
#include "Tokenize.h"
#include "Util.h"
#include "WorldPacket.h"
#include "WorldSession.h"

uint8  RaceMgr::_maxRaces = 0;

uint32 RaceMgr::_playableRaceMask = 0;
uint32 RaceMgr::_allianceRaceMask = 0;
uint32 RaceMgr::_hordeRaceMask = 0;

std::map<uint32, uint32> RaceMgr::_donors;

RaceMgr::RaceMgr()
{
}

RaceMgr::~RaceMgr()
{
}

RaceMgr* RaceMgr::instance()
{
    static RaceMgr instance;
    return &instance;
}

void RaceMgr::LoadRaces()
{
    SetMaxRaces(0 + 1);
    _playableRaceMask = 0;
    _hordeRaceMask    = 0;
    _allianceRaceMask = 0;

    for (auto const& raceEntry : sChrRacesStore)
    {
        if (!raceEntry)
            continue;

        uint8 alliance = raceEntry->alliance;
        uint8 raceId = raceEntry->RaceID;

        if (raceEntry->Flags & CHRRACES_FLAGS_NOT_PLAYABLE)
            continue;

        if (GetMaxRaces() <= raceId)
            SetMaxRaces(raceId + 1);

        uint32 raceBit = (1 << (raceId - 1));

        _playableRaceMask |= raceBit;

        if (alliance == ALLIANCE_HORDE)
            _hordeRaceMask |= raceBit;
        else if (alliance == ALLIANCE_ALLIANCE)
            _allianceRaceMask |= raceBit;
    }

    LoadDonors();
}

// Config "Races.Donors": space separated "<race>:<donor race>" pairs, e.g. "9:2 12:1".
// A race listed here (and playable per ChrRaces.dbc) gets the same multi-race world data masks as its donor.
void RaceMgr::LoadDonors()
{
    _donors.clear();

    std::string const donorConfig = sConfigMgr->GetOption<std::string>("Races.Donors", "9:2 12:1");

    for (std::string_view const entry : Acore::Tokenize(donorConfig, ' ', false))
    {
        std::vector<std::string_view> const pair = Acore::Tokenize(entry, ':', true);

        Optional<uint32> raceId;
        Optional<uint32> donorId;
        if (pair.size() == 2)
        {
            raceId = Acore::StringTo<uint32>(pair[0]);
            donorId = Acore::StringTo<uint32>(pair[1]);
        }

        // race ids index a 32 bit mask
        if (!raceId || !donorId || *raceId < 1 || *raceId > 32 || *donorId < 1 || *donorId > 32 || *raceId == *donorId)
        {
            LOG_ERROR("server.loading", "Races.Donors: invalid entry '{}' (expected <race>:<donor race>, ids 1-32, different), ignored.", entry);
            continue;
        }

        uint32 const raceBit = 1u << (*raceId - 1);
        uint32 const donorBit = 1u << (*donorId - 1);

        // normal on a server whose ChrRaces.dbc does not carry the race
        if (!(_playableRaceMask & raceBit))
        {
            LOG_DEBUG("server.loading", "Races.Donors: race {} is not playable, entry '{}' not used.", *raceId, entry);
            continue;
        }

        if (!(_playableRaceMask & donorBit))
        {
            LOG_ERROR("server.loading", "Races.Donors: donor race {} of race {} is not playable, entry '{}' ignored.", *donorId, *raceId, entry);
            continue;
        }

        if (!_donors.emplace(*raceId, *donorId).second)
        {
            LOG_ERROR("server.loading", "Races.Donors: race {} is listed twice, entry '{}' ignored.", *raceId, entry);
            continue;
        }

        LOG_INFO("server.loading", ">> Race {} follows donor race {} for race masks.", *raceId, *donorId);
    }
}

uint32 RaceMgr::GetDonorRace(uint32 race)
{
    auto itr = _donors.find(race);
    return itr != _donors.end() ? itr->second : 0;
}

uint32 RaceMgr::WidenRaceMask(uint32 mask) const
{
    // 0 and masks with one playable race bit keep their meaning (racial flavour stays with its race)
    uint32 const playableBits = mask & _playableRaceMask;
    if (!(playableBits & (playableBits - 1)))
        return mask;

    uint32 widened = mask;
    for (auto const& [raceId, donorId] : _donors)
    {
        uint32 const raceBit = 1u << (raceId - 1);
        uint32 const donorBit = 1u << (donorId - 1);

        // judged on the original mask, so the result does not depend on the order of the entries
        if (!(mask & raceBit) && (mask & donorBit))
            widened |= raceBit;
    }

    return widened;
}
