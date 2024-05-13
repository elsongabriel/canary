/**
 * Canary - A free and open-source MMORPG server emulator
 * Copyright (©) 2019-2024 OpenTibiaBR <opentibiabr@outlook.com>
 * Repository: https://github.com/opentibiabr/canary
 * License: https://github.com/opentibiabr/canary/blob/main/LICENSE
 * Contributors: https://github.com/opentibiabr/canary/graphs/contributors
 * Website: https://docs.opentibiabr.com/
 */

#include "pch.hpp"

#include "player_cyclopedia.hpp"

#include "creatures/players/player.hpp"
#include "game/game.hpp"
#include "kv/kv.hpp"

PlayerCyclopedia::PlayerCyclopedia(Player &player) :
	m_player(player) { }

void PlayerCyclopedia::loadSummaryData() {

	loadRecentKills();
	loadDeathHistory();
}

void PlayerCyclopedia::loadRecentKills() {
	Benchmark bm_check;

	Database &db = Database::getInstance();
	const std::string &escapedName = db.escapeString(m_player.getName());
	DBResult_ptr result = db.storeQuery(fmt::format("SELECT `d`.`time`, `d`.`killed_by`, `d`.`mostdamage_by`, `d`.`unjustified`, `d`.`mostdamage_unjustified`, `p`.`name` FROM `player_deaths` AS `d` INNER JOIN `players` AS `p` ON `d`.`player_id` = `p`.`id` WHERE ((`d`.`killed_by` = {} AND `d`.`is_player` = 1) OR (`d`.`mostdamage_by` = {} AND `d`.`mostdamage_is_player` = 1))", escapedName, escapedName));
	if (result) {
		do {
			std::string cause1 = result->getString("killed_by");
			std::string cause2 = result->getString("mostdamage_by");
			std::string name = result->getString("name");

			uint8_t status = CYCLOPEDIA_CHARACTERINFO_RECENTKILLSTATUS_JUSTIFIED;
			if (m_player.getName() == cause1) {
				if (result->getNumber<uint32_t>("unjustified") == 1) {
					status = CYCLOPEDIA_CHARACTERINFO_RECENTKILLSTATUS_UNJUSTIFIED;
				}
			} else if (m_player.getName() == cause2) {
				if (result->getNumber<uint32_t>("mostdamage_unjustified") == 1) {
					status = CYCLOPEDIA_CHARACTERINFO_RECENTKILLSTATUS_UNJUSTIFIED;
				}
			}

			std::ostringstream description;
			description << "Killed " << name << '.';
			insertPvpKillOnHistory(std::move(description.str()), result->getNumber<uint32_t>("time"), status);
		} while (result->next());
	}

	g_logger().debug("Checking and updating recent kill of player {} took {} milliseconds.", m_player.getName(), bm_check.duration());
}

void PlayerCyclopedia::loadDeathHistory() {
	Benchmark bm_check;

	Database &db = Database::getInstance();
	DBResult_ptr result = db.storeQuery(fmt::format("SELECT `time`, `level`, `killed_by`, `mostdamage_by` FROM `player_deaths` WHERE `player_id` = {} ORDER BY `time` DESC", m_player.getGUID()));
	if (result) {
		do {
			std::string cause1 = result->getString("killed_by");
			std::string cause2 = result->getString("mostdamage_by");
			std::ostringstream description;
			description << "Died at Level " << result->getNumber<uint32_t>("level") << " by";
			if (!cause1.empty()) {
				description << getArticle(cause1) << cause1;
			}

			if (!cause2.empty()) {
				if (!cause1.empty()) {
					description << " and";
				}
				description << getArticle(cause2) << cause2;
			}
			description << '.';

			insertDeathOnHistory(std::move(description.str()), result->getNumber<uint32_t>("time"));
		} while (result->next());
	}

	g_logger().debug("Checking and updating death history of player {} took {} milliseconds.", m_player.getName(), bm_check.duration());
}

std::vector<RecentDeathEntry> PlayerCyclopedia::getDeathHistory() const {
	return m_deathHistory;
}

void PlayerCyclopedia::insertDeathOnHistory(std::string cause, uint32_t timestamp) {
	m_deathHistory.emplace_back(std::move(cause), timestamp);
}

std::vector<RecentPvPKillEntry> PlayerCyclopedia::getPvpKillsHistory() const {
	return m_pvpKillsHistory;
}

void PlayerCyclopedia::insertPvpKillOnHistory(std::string cause, uint32_t timestamp, uint8_t status) {
	m_pvpKillsHistory.emplace_back(std::move(cause), timestamp, status);
}

void PlayerCyclopedia::updateStoreSummary(uint8_t type, uint16_t count, uint8_t itemType, uint8_t offerId, uint8_t blessId) {
}

// const std::shared_ptr<KV> &PlayerTitle::getSummary(std::string &key) {
//	return m_player.kv()->scoped("titles")->scoped("summary")->get(key);
// }
//
// uint16_t PlayerAchievement::getPoints() const {
//	return m_player.kv()->scoped("achievements")->get("points")->getNumber();
// }
//
// void PlayerAchievement::addPoints(uint16_t toAddPoints) {
//	auto oldPoints = getPoints();
//	m_player.kv()->scoped("achievements")->set("points", oldPoints + toAddPoints);
// }
