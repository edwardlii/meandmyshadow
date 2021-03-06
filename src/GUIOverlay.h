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

#ifndef GUI_OVERLAY_H
#define GUI_OVERLAY_H

#include <SDL.h>
#include "GameState.h"
#include "GUIObject.h"


//The GUIOverlay state, this is a special state since it doens't appear in the states list.
//It is used to properly handle GUIs that overlay the current state, dialogs for example.
class GUIOverlay : public GameState{
private:
	//A pointer to the current state to put back when needed.
	GameState* parentState;

	//Pointer to the GUI root of the overlay.
	GUIObject* root;
	//Pointer to the previous GUIObjectRoot.
	GUIObject* tempGUIObjectRoot;

	//Boolean if the screen should be dimmed.
	bool dim;

public:
	//Enables default keyboard navigation code.
	//See the enum KeyboardNavigationMode for more info.
	int keyboardNavigationMode;

public:
	//Constructor.
	//root: Pointer to the new GUIObjectRoot.
	//dim: Boolean if the parent state should be dimmed.
    GUIOverlay(SDL_Renderer &renderer, GUIObject* root, bool dim=true);
	//Destructor.
	~GUIOverlay();

	//Method that can be used to create a "sub gameloop".
	//This is usefull in case the GUI that is overlayed is used for userinput which the function needs to return.
	//NOTE: This loop should be kept similar to the main loop.
	//skip: Boolean if this GUIOverlay can be "skipped", meaning it can be exited quickly by pressing escape or return.
	void enterLoop(ImageManager &imageManager, SDL_Renderer& renderer, bool skipByEscape = false, bool skipByReturn = false);

    //Inherited from GameState.
    void handleEvents(ImageManager&, SDL_Renderer&) override;
    void logic(ImageManager&, SDL_Renderer&) override;
    void render(ImageManager& imageManager, SDL_Renderer& renderer) override;
    void resize(ImageManager& imageManager, SDL_Renderer& renderer) override;
};

class GUIButton;
class GUITextArea;

// A subclass of GUIOverlay which has:
// - an optional text area which is scrolled by arrow keys
// - an optional cancel button which will be clicked by pressing ESC key.
class AddonOverlay : public GUIOverlay {
private:
	GUIButton *cancelButton;
	GUITextArea *textArea;

public:
	AddonOverlay(SDL_Renderer &renderer, GUIObject* root, GUIButton *cancelButton, GUITextArea *textArea, int keyboardNavigationMode);

	void handleEvents(ImageManager& imageManager, SDL_Renderer& renderer) override;
};


#endif
