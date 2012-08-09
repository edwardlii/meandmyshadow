/*
 * Copyright (C) 2012 Me and My Shadow
 *
 * This file is part of Me and My Shadow.
 *
 * Me and My Shadow is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Me and My Shadow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Me and My Shadow.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STATISTICSMANAGER_H
#define STATISTICSMANAGER_H

#include <SDL/SDL.h>
#include <string>
#include <vector>
#include <map>

//internal struct for achievement info
struct AchievementInfo;

class StatisticsScreen;

class StatisticsManager{
	friend class StatisticsScreen;
public:
	//Player and shadow traveling distance (m), 1 block = 1 meter
	float playerTravelingDistance,shadowTravelingDistance;
	//Player and shadow jumps
	int playerJumps,shadowJumps;
	//Player and shadow dies
	int playerDies,shadowDies;
	//Player and shadow squashed
	int playerSquashed,shadowSquashed;
	//Completed levels. Note: this is dynamically calculated, and doesn't save to file.
	int completedLevels,silverLevels,goldLevels;
	//Record times
	int recordTimes;
	//number of switched pulled
	int switchTimes;
	//swap times
	int swapTimes;
	//play time (s)
	int playTime;
	//level edit time (s)
	int levelEditTime;
	//created levels
	int createdLevels;
private:
	//current achievement displayed time
	int achievementTime;
	//some picture
	SDL_Surface *bmDropShadow;
	//SDL_Surface for current achievement (excluding drop shadow)
	SDL_Surface *bmAchievement;
	//currently owned achievements
	std::map<std::string,AchievementInfo*> achievements;
	//queued achievements for display
	std::vector<AchievementInfo*> queuedAchievements;
	//currently displayed achievement
	int currentAchievement;
	//starting time
	time_t startTime;
	//level edit starting time
	time_t levelEditStartTime;
public:
	StatisticsManager();
	~StatisticsManager();

	//clear the statistics and achievements.
	void clear();
	//load needed picture
	void loadPicture();
	//register avaliable achievements
	static void registerAchievements();
	//load statistics file.
	void loadFile(const std::string& fileName);
	//save statistics file.
	void saveFile(const std::string& fileName);
	//add or display a new achievement.
	//name: the achievement id. if can't find it in avaliable achievement, nothing happens.
	//save: if true then save to currently owned achievements. if it already exists in
	//currently owned achievements, nothing happens.
	//if false then just added it to queue, including duplicated achievements.
	void newAchievement(const std::string& id,bool save=true);
	//if there are new achievements, draw it on the screen,
	//otherwise do nothing.
	void render();

	//Call this function to update completed levels.
	//Note: Level progress files are reloaded, so it's slow.
	void reloadCompletedLevelsAndAchievements();

	//Call this function to update other achievements at game startup.
	void reloadOtherAchievements();

	//Update level specified achievements.
	//Make sure the completed level count is correct.
	void updateLevelAchievements();

	//Update tutorial specified achievements.
	//Make sure the level progress of tutorial is correct.
	void updateTutorialAchievements();

	//Call when level edit is start
	void startLevelEdit();

	//Call when level edit is end
	void endLevelEdit();

	//update in-game time
	void updatePlayTime();

	//create a SDL_Surface contains specified achievements or draw to existing surface.
	//info: achievement info.
	//surface: specifies SDL_Surface to draw on. if NULL then new surface will be created.
	//rect [in, out, optional]: specifies position and optionally width to draw on. height will be returned.
	//if NULL then will be drawn on top-left corner. if surface is NULL then rect->x and rect->y are ignored.
	//showTip: shows "New achievement" tip
	//return value: SDL_Surface contains specified achievements or NULL if any error occured.
	SDL_Surface* createAchievementSurface(AchievementInfo* info,SDL_Surface* surface=NULL,SDL_Rect* rect=NULL,bool showTip=true);
private:
	//internal function
	//flags: a bit-field value indicates which achievements we have.
	void updateTutorialAchievementsInternal(int flags);
	//internal function. alpha should be 1-5, 5 means fully opaque (not really)
	void drawAchievement(int alpha);
};

extern StatisticsManager statsMgr;

#endif