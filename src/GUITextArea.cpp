/*
 * Copyright (C) 2011-2012 Me and My Shadow
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

#include "GUITextArea.h"
#include <cmath>
using namespace std;

GUITextArea::GUITextArea(int left,int top,int width,int height,bool enabled,bool visible):
	GUIObject(left,top,width,height,0,NULL,-1,enabled,visible){
	
	//Set some default values.
	state=value=currentLine=0;
	setFont(fontText);
	
	//Add empty text.
	lines.push_back("");
	linesCache.push_back(NULL);
}

GUITextArea::~GUITextArea(){
	//Free cached images.
	for(int i=0;i<linesCache.size();i++){
		SDL_FreeSurface(linesCache[i]);
	}
	linesCache.clear();
}

void GUITextArea::setFont(TTF_Font* font){
	//NOTE: This fuction shouldn't be called after adding items, so no need to update the whole cache.
	widgetFont=font;
	fontHeight=TTF_FontHeight(font)+1;
}

bool GUITextArea::handleEvents(int x,int y,bool enabled,bool visible,bool processed){
	//Boolean if the event is processed.
	bool b=processed;
	
	//The GUIObject is only enabled when he and his parent are enabled.
	enabled=enabled && this->enabled;
	//The GUIObject is only enabled when he and his parent are enabled.
	visible=visible && this->visible;
	
	//Get the absolute position.
	x+=left;
	y+=top;
	
	//NOTE: We don't reset the state to have a "focus" effect.  
	//Only check for events when the object is both enabled and visible.
	if(enabled&&visible){
		//Check if there's a key press and the event hasn't been already processed.
		if(state==2 && event.type==SDL_KEYDOWN && !b){
			//Get the keycode.
			int key=(int)event.key.keysym.unicode;
			
			//Check if the key is supported.
			if(key>=32&&key<=126){
				//Add the key to the string.
				string* str=&lines.at(currentLine);
				str->insert((size_t)value,1,char(key));
				value++;
				
				//Update cache.
				SDL_Surface** c=&linesCache.at(currentLine);
				if(*c) SDL_FreeSurface(*c);
				SDL_Color black={0,0,0,0};
				*c=TTF_RenderUTF8_Blended(widgetFont,str->c_str(),black);
				
				//If there is an event callback then call it.
				if(eventCallback){
					GUIEvent e={eventCallback,name,this,GUIEventChange};
					GUIEventQueue.push_back(e);
				}
			}else if(event.key.keysym.sym==SDLK_BACKSPACE){
				//Set the key values correct.
				this->key=SDLK_BACKSPACE;
				keyHoldTime=0;
				keyTime=5;
				
				//Delete one character direct to prevent a lag.
				backspaceChar();
			}else if(event.key.keysym.sym==SDLK_DELETE){
				//Set the key values correct.
				this->key=SDLK_DELETE;
				keyHoldTime=0;
				keyTime=5;
				
				//Delete one character direct to prevent a lag.
				deleteChar();
			}else if(event.key.keysym.sym==SDLK_RETURN){				
				//Split the current line and update.
				string str2=lines.at(currentLine).substr(value);
				lines.at(currentLine)=lines.at(currentLine).substr(0,value);
				
				SDL_Surface** c=&linesCache.at(currentLine);
				if(*c) SDL_FreeSurface(*c);
				SDL_Color black={0,0,0,0};
				*c=TTF_RenderUTF8_Blended(widgetFont,lines.at(currentLine).c_str(),black);
				
				//Add the rest in a new line.
				currentLine++;
				value=0;
				lines.insert(lines.begin()+currentLine,str2);
				
				SDL_Surface* c2;
				c2=TTF_RenderUTF8_Blended(widgetFont,str2.c_str(),black);
				linesCache.insert(linesCache.begin()+currentLine,c2);
				
				//If there is an event callback then call it.
				if(eventCallback){
					GUIEvent e={eventCallback,name,this,GUIEventChange};
					GUIEventQueue.push_back(e);
				}
			}else if(event.key.keysym.sym==SDLK_TAB){
				//Add a tabulator or here just 2 spaces to the string.
				string* str=&lines.at(currentLine);
				str->insert((size_t)value,2,char(' '));
				value+=2;
				
				//Update cache.
				SDL_Surface** c=&linesCache.at(currentLine);
				if(*c) SDL_FreeSurface(*c);
				SDL_Color black={0,0,0,0};
				*c=TTF_RenderUTF8_Blended(widgetFont,str->c_str(),black);
			}else if(event.key.keysym.sym==SDLK_RIGHT){
				//Set the key values correct.
				this->key=SDLK_RIGHT;
				keyHoldTime=0;
				keyTime=5;
				
				//Move the carrot once to prevent a lag.
				moveCarrotRight();
			}else if(event.key.keysym.sym==SDLK_LEFT){
				//Set the key values correct.
				this->key=SDLK_LEFT;
				keyHoldTime=0;
				keyTime=5;
				
				//Move the carrot once to prevent a lag.
				moveCarrotLeft();
			}else if(event.key.keysym.sym==SDLK_DOWN){
				//Set the key values correct.
				this->key=SDLK_DOWN;
				keyHoldTime=0;
				keyTime=5;
				
				//Move the carrot once to prevent a lag.
				moveCarrotDown();
			}else if(event.key.keysym.sym==SDLK_UP){
				//Set the key values correct.
				this->key=SDLK_UP;
				keyHoldTime=0;
				keyTime=5;
				
				//Move the carrot once to prevent a lag.
				moveCarrotUp();
			}
			
			//The event has been processed.
			b=true;
		}else if(state==2 && event.type==SDL_KEYUP && !b){
			//Check if released key is the same as the holded key.
			if(event.key.keysym.sym==key){
				//It is so stop the key.
				key=-1;
			}
		}
		
		//The mouse location (x=i, y=j) and the mouse button (k).
		int i,j,k;
		k=SDL_GetMouseState(&i,&j);
		
		//Check if the mouse is inside the GUIObject.
		if(i>=x&&i<x+width&&j>=y&&j<y+height){
			//We can only increase our state. (nothing->hover->focus).
			if(state!=2){
				state=1;
			}

			//Also update the cursor type.
			currentCursor=CURSOR_CARROT;
			
			//Check for a mouse button press.
			if(k&SDL_BUTTON(1)){
				//We have focus.
				state=2;
				
				//Move carrot to the place clicked.
				currentLine=clamp(floor((j-y)/fontHeight),0,lines.size()-1);
				string* str=&lines.at(currentLine);
				value=str->length();
				
				int clickX=i-x;
				int xPos=0;
				
				for(int i=0;i<str->length();i++){
					int advance;
					TTF_GlyphMetrics(widgetFont,str->at(i),NULL,NULL,NULL,NULL,&advance);
					xPos+=advance;
					
					if(clickX<xPos-advance/2){
						value=i;
						break;
					}
				}
			}
		}else{
			//The mouse is outside the TextBox.
			//If we don't have focus but only hover we lose it.
			if(state==1){
				state=0;
			}
			
			//If it's a click event outside the textbox then we blur.
			if(event.type==SDL_MOUSEBUTTONUP && event.button.button==SDL_BUTTON_LEFT){
				//Set state to 0.
				state=0;
			}
		}
	}
	
	//Process child controls event except for the scrollbar.
	//That's why i starts at one.
	for(unsigned int i=1;i<childControls.size();i++){
		bool b1=childControls[i]->handleEvents(x,y,enabled,visible,b);
		
		//The event is processed when either our or the childs is true (or both).
		b=b||b1;
	}
	return b;
}

void GUITextArea::deleteChar(){
	//Remove a character after the carrot.
	if(value<lines.at(currentLine).length()){
		//Normal delete inside of a line.
		//Update string.
		string* str=&lines.at(currentLine);
		str->erase((size_t)value,1);
		
		//Update cache.
		SDL_Surface** c=&linesCache.at(currentLine);
		if(*c) SDL_FreeSurface(*c);
		SDL_Color black={0,0,0,0};
		*c=TTF_RenderUTF8_Blended(widgetFont,str->c_str(),black);
	}else{
		//Make sure there's a line after currentLine.
		if(currentLine<lines.size()-1){
		//Append next line.
		string* str=&lines.at(currentLine);
			str->append(lines.at(currentLine+1));
			
			//Remove the unused line.
			SDL_Surface** c=&linesCache.at(currentLine+1);
			if(*c) SDL_FreeSurface(*c);
			lines.erase(lines.begin()+currentLine+1);
			linesCache.erase(linesCache.begin()+currentLine+1);
			
			//Update cache.
			c=&linesCache.at(currentLine);
			if(*c) SDL_FreeSurface(*c);
			SDL_Color black={0,0,0,0};
			*c=TTF_RenderUTF8_Blended(widgetFont,str->c_str(),black);
		}
	}
	
	//If there is an event callback.
	if(eventCallback){
		GUIEvent e={eventCallback,name,this,GUIEventChange};
		GUIEventQueue.push_back(e);
	}
}

void GUITextArea::backspaceChar(){
	//Remove a character before the carrot.
	value--;
	if(value<0){
		//Remove a line but append it's content to the previous.
		//However we can't do this to the first line.
		if(currentLine>0){
			//Remove line from display but store the string.
			string str=lines.at(currentLine);
			lines.erase(lines.begin()+currentLine);
			SDL_Surface** c=&linesCache.at(currentLine);
			if(*c) SDL_FreeSurface(*c);
			linesCache.erase(linesCache.begin()+currentLine);
			
			//Append that string to the previous line.
			currentLine--;
			string* str2=&lines.at(currentLine);
			value=str2->length();
			str2->append(str);
			
			//Update cache.
			c=&linesCache.at(currentLine);
			if(*c) SDL_FreeSurface(*c);
			SDL_Color black={0,0,0,0};
			*c=TTF_RenderUTF8_Blended(widgetFont,str2->c_str(),black);
		}else{
			//Don't let the value become negative.
			value=0;
		}
	}else{
		//Normal delete inside of a line.
		//Update string.
		string* str=&lines.at(currentLine);
		str->erase((size_t)value,1);
		
		//Update cache.
		SDL_Surface** c=&linesCache.at(currentLine);
		if(*c) SDL_FreeSurface(*c);
		SDL_Color black={0,0,0,0};
		*c=TTF_RenderUTF8_Blended(widgetFont,str->c_str(),black);
	}
		
	//If there is an event callback.
	if(eventCallback){
		GUIEvent e={eventCallback,name,this,GUIEventChange};
		GUIEventQueue.push_back(e);
	}
}

void GUITextArea::moveCarrotRight(){
	value++;
	if(value>lines.at(currentLine).length()){
		if(currentLine==lines.size()-1){
			value=lines.at(currentLine).length();
		}else{
			currentLine++;
			value=0;
		}
	}
}

void GUITextArea::moveCarrotLeft(){
	value--;
	if(value<0){
		if(currentLine==0){
			value=0;
		}else{
			currentLine--;
			value=lines.at(currentLine).length();
		}
	}
}

void GUITextArea::moveCarrotUp(){
	if(currentLine==0){
		value=0;
	}else{
		//Calculate carrot x position.
		int carrotX=0;
		for(int n=0;n<value;n++){
			int advance;
			TTF_GlyphMetrics(widgetFont,lines.at(currentLine).at(n),NULL,NULL,NULL,NULL,&advance); 
			carrotX+=advance;
		}
		
		//Find out closest match.
		currentLine--;
		string* str=&lines.at(currentLine);
		value=str->length();
		
		int xPos=0;
		for(int i=0;i<str->length();i++){
			int advance;
			TTF_GlyphMetrics(widgetFont,str->at(i),NULL,NULL,NULL,NULL,&advance);
			xPos+=advance;
			
			if(carrotX<xPos-advance/2){
				value=i;
				break;
			}
		}
	}
}
	
void GUITextArea::moveCarrotDown(){
	if(currentLine==lines.size()-1){
		value=lines.at(currentLine).length();
	}else{
		//Calculate carrot x position.
		int carrotX=0;
		for(int n=0;n<value;n++){
			int advance;
			TTF_GlyphMetrics(widgetFont,lines.at(currentLine).at(n),NULL,NULL,NULL,NULL,&advance); 
			carrotX+=advance;
		}
		
		//Find out closest match.
		currentLine++;
		string* str=&lines.at(currentLine);
		value=str->length();
		
		int xPos=0;
		for(int i=0;i<str->length();i++){
			int advance;
			TTF_GlyphMetrics(widgetFont,str->at(i),NULL,NULL,NULL,NULL,&advance);
			xPos+=advance;
			
			if(carrotX<xPos-advance/2){
				value=i;
				break;
			}
		}
	}
}

void GUITextArea::render(int x,int y,bool draw){
	//FIXME: Logic in the render method since that is update constant.
	if(key!=-1){
		//Increase the key time.
		keyHoldTime++;
		//Make sure the deletionTime isn't to short.
		if(keyHoldTime>=keyTime){
			keyHoldTime=0;
			keyTime--;
			if(keyTime<1)
				keyTime=1;
			
			//Now check the which key it was.
			switch(key){
				case SDLK_BACKSPACE:
					backspaceChar();
					break;
				case SDLK_DELETE:
					deleteChar();
					break;
				case SDLK_LEFT:
					moveCarrotLeft();
					break;
				case SDLK_RIGHT:
					moveCarrotRight();
					break;
				case SDLK_UP:
					moveCarrotUp();
					break;
				case SDLK_DOWN:
					moveCarrotDown();
					break;
			}
		}
	}
	
	//There's no need drawing the GUIObject when it's invisible.
	if(!visible||!draw) 
		return;
	
	//Get the absolute x and y location.
	x+=left;
	y+=top;
	
	//Default background opacity
	int clr=128;
	//If hovering or focused make background more visible.
	if(state==1) 
		clr=255;
	else if (state==2)
		clr=230;
	
	//Draw the box.
	Uint32 color=0xFFFFFF00|clr;
	drawGUIBox(x,y,width,height,screen,color);
	
	//Draw text.
	//TODO: support scrolling
	int lineY=0;
	for(std::vector<SDL_Surface*>::iterator it=linesCache.begin();it!=linesCache.end();++it){
		if(*it) applySurface(x+1,y+1+lineY,*it,screen,NULL);
		lineY+=fontHeight;
	}
	
	//Only draw the carrot when focus.
	if(state==2){
		SDL_Rect r;
		r.x=x;
		r.y=y+4+fontHeight*currentLine;
		r.w=2;
		r.h=fontHeight-4;
		
		for(int n=0;n<value;n++){
			int advance;
			TTF_GlyphMetrics(widgetFont,lines.at(currentLine).at(n),NULL,NULL,NULL,NULL,&advance); 
			r.x+=advance;
		}
		
		//Make sure that the carrot is inside the textbox.
		if(r.x<x+width)
			SDL_FillRect(screen,&r,0);
	}
	
	//We now need to draw all the children of the GUIObject.
	for(unsigned int i=0;i<childControls.size();i++){
		childControls[i]->render(x,y,draw);
	}
}

void GUITextArea::setString(std::string input){
	//Clear previous content if any.
	//Delete every line.
	lines.clear();
	//Free cached images.
	for(int i=0;i<linesCache.size();i++){
		SDL_FreeSurface(linesCache[i]);
	}
	linesCache.clear();
	
	size_t linePos=0,lineLen=0;
	SDL_Color black={0,0,0,0};
	SDL_Surface* bm=NULL;
	
	//Loop through the input string.
	for(size_t i=0;i<input.length();++i)
	{
		//Check when we come in end of a line.
		if(input.at(i)=='\n'){
			//Check if the line is empty.
			if(lineLen==0){
				lines.push_back("");
				linesCache.push_back(NULL);				
			}else{
				//Read the whole line.
				string line=input.substr(linePos,lineLen);
				lines.push_back(line);
				
				//Render and cache text.
				bm=TTF_RenderUTF8_Blended(widgetFont,line.c_str(),black);
				linesCache.push_back(bm);
			}
			//Skip '\n' in end of the line.
			linePos=i+1;
			lineLen=0;
		}else{
			lineLen++;
		}
	}
	
	//The string might not end with a newline.
	//That's why we're going to add end rest of the string as one line.
	string line=input.substr(linePos);
	lines.push_back(line);
	
	bm=TTF_RenderUTF8_Blended(widgetFont,line.c_str(),black);
	linesCache.push_back(bm);
}

string GUITextArea::getString(){
	string tmp;
	for(vector<string>::iterator it=lines.begin();it!=lines.end();++it){
		//Append a newline only if not the first line.
		if(it!=lines.begin())
			tmp.append(1,'\n');
		//Append the line.
		tmp.append(*it);
	}
	return tmp;
}
