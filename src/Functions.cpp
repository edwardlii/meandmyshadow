/****************************************************************************
** Copyright (C) 2011 Luka Horvat <redreaper132 at gmail.com>
** Copyright (C) 2011 Edward Lii <edward_iii at myway.com>
** Copyright (C) 2011 O. Bahri Gordebak <gordebak at gmail.com>
**
**
** This file may be used under the terms of the GNU General Public
** License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h> 
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <string>
#include "Globals.h"
#include "Functions.h"
#include "FileManager.h"
#include "Objects.h"
#include "Player.h"
#include "GameObjects.h"
#include "LevelPack.h"
#include "TitleMenu.h"
#include "LevelEditSelect.h"
#include "LevelEditor.h"
#include "Game.h"
#include "LevelPlaySelect.h"
#include "Addons.h"
#include "ImageManager.h"
#include "MusicManager.h"
#include "LevelPackManager.h"
#include "ThemeManager.h"
#include "GUIListBox.h"

#include "libs/tinyformat/tinyformat.h"
#include "libs/tinygettext/tinygettext.hpp"
extern "C" {
#include "libs/findlocale/findlocale.h"
}

#ifdef HARDWARE_ACCELERATION
#include <GL/gl.h>
#include <GL/glu.h>

//fix some Windows header bug
#ifndef GL_BGR
#define GL_BGR GL_BGR_EXT
#endif
#ifndef GL_BGRA
#define GL_BGRA GL_BGRA_EXT
#endif

#endif
using namespace std;

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#endif

//Initialise the imagemanager.
//The ImageManager is used to prevent loading images multiple times.
ImageManager imageManager;

//Initialise the musicManager.
//The MusicManager is used to prevent loading music files multiple times and for playing/fading music.
MusicManager musicManager;

//Initialise the levelPackManager.
//The LevelPackManager is used to prevent loading levelpacks multiple times and for the game to know which levelpacks there are.
LevelPackManager levelPackManager;

//Map containing changed settings using command line arguments.
map<string,string> tmpSettings;
//Pointer to the settings object.
//It is used to load and save the settings file and change the settings.
Settings* settings=0;

#ifdef HARDWARE_ACCELERATION
GLuint screenTexture;
#endif

SDL_Surface* loadImage(string file){
	//We use the imageManager to load the file.
	return imageManager.loadImage(file);
}

void applySurface(int x,int y,SDL_Surface* source,SDL_Surface* dest,SDL_Rect* clip){
	//The offset is needed to draw at the right location.
	SDL_Rect offset;
	offset.x=x;
	offset.y=y;
	
	//Let SDL do the drawing of the surface.
	SDL_BlitSurface(source,clip,dest,&offset);
}

void drawRect(int x,int y,int w,int h,SDL_Surface* dest,Uint32 color){
	//NOTE: We let SDL_gfx render it.
	rectangleRGBA(dest,x,y,x+w,y+h,color >> 24,color >> 16,color >> 8,255);
}

//Draw a box with anti-aliased borders using SDL_gfx.
void drawGUIBox(int x,int y,int w,int h,SDL_Surface* dest,Uint32 color){
	//Fill content's background color from function parameter
	boxRGBA(dest,x+1,y+1,x+w-2,y+h-2,color >> 24,color >> 16,color >> 8,color >> 0);

	//Draw first black borders around content and leave 1 pixel in every corner
	lineRGBA(dest,x+1,y,x+w-2,y,0,0,0,255);
	lineRGBA(dest,x+1,y+h-1,x+w-2,y+h-1,0,0,0,255);
	lineRGBA(dest,x,y+1,x,y+h-2,0,0,0,255);
	lineRGBA(dest,x+w-1,y+1,x+w-1,y+h-2,0,0,0,255);
	
	//Fill the corners with transperent color to create anti-aliased borders
	pixelRGBA(dest,x,y,0,0,0,160);
	pixelRGBA(dest,x,y+h-1,0,0,0,160);
	pixelRGBA(dest,x+w-1,y,0,0,0,160);
	pixelRGBA(dest,x+w-1,y+h-1,0,0,0,160);

	//Draw second lighter border around content
	rectangleRGBA(dest,x+1,y+1,x+w-2,y+h-2,0,0,0,64);
	
	//Create anti-aliasing in corners of second border
	pixelRGBA(dest,x+1,y+1,0,0,0,50);
	pixelRGBA(dest,x+1,y+h-2,0,0,0,50);
	pixelRGBA(dest,x+w-2,y+1,0,0,0,50);
	pixelRGBA(dest,x+w-2,y+h-2,0,0,0,50);
}

void drawLine(int x1,int y1,int x2,int y2,SDL_Surface* dest,Uint32 color){
	//NOTE: We let SDL_gfx render it.
	lineRGBA(dest,x1,y1,x2,y2,color >> 24,color >> 16,color >> 8,255);
}

void drawLineWithArrow(int x1,int y1,int x2,int y2,SDL_Surface* dest,Uint32 color,int spacing,int offset,int xsize,int ysize){
	//Draw line first
	drawLine(x1,y1,x2,y2,dest,color);

	//calc delta and length
	double dx=x2-x1;
	double dy=y2-y1;
	double length=sqrt(dx*dx+dy*dy);
	if(length<0.001) return;

	//calc the unit vector
	dx/=length; dy/=length;

	//Now draw arrows on it
	for(double p=offset;p<length;p+=spacing){
		drawLine(int(x1+p*dx+0.5),int(y1+p*dy+0.5),
			int(x1+(p-xsize)*dx-ysize*dy+0.5),int(y1+(p-xsize)*dy+ysize*dx+0.5),dest,color);
		drawLine(int(x1+p*dx+0.5),int(y1+p*dy+0.5),
			int(x1+(p-xsize)*dx+ysize*dy+0.5),int(y1+(p-xsize)*dy-ysize*dx+0.5),dest,color);
	}
}

bool createScreen(){
  	//Set the screen_width and height.
	SCREEN_WIDTH=atoi(settings->getValue("width").c_str());
	SCREEN_HEIGHT=atoi(settings->getValue("height").c_str());
	
	//Update the camera.
	camera.w=SCREEN_WIDTH;
	camera.h=SCREEN_HEIGHT;
	
	//Check if we should use gl or software rendering.
	if(settings->getBoolValue("gl")){
#ifdef HARDWARE_ACCELERATION
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
		
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE,32);
		
		SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,8);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE,8);
		
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,0);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
		
		//Set the video mode.
		Uint32 flags=SDL_HWSURFACE | SDL_OPENGL;
		if(settings->getBoolValue("fullscreen"))
			flags|=SDL_FULLSCREEN;
		if(SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP,flags)==NULL){
			fprintf(stderr,"FATAL ERROR: SDL_SetVideoMode failed\n");
			return false;
		}
		//Create a screen 
		screen=SDL_CreateRGBSurface(SDL_HWSURFACE,SCREEN_WIDTH,SCREEN_HEIGHT,32,0x00FF0000,0x0000FF00,0x000000FF,0);
		
		//Create a texture.
		glGenTextures(1,&screenTexture);
		
		//And set up gl correctly.
		glClearColor(0, 0, 0, 0);
		glClearDepth(1.0f);
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1, -1);
		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_TEXTURE_2D);
		glLoadIdentity();
#else
		//NOTE: Hardware accelerated rendering requested but compiled without.
		cerr<<"FATAL ERROR: Unable to use hardware acceleration (compiled without)."<<endl;
		return false;
#endif
	}else{
		Uint32 flags=SDL_HWSURFACE | SDL_DOUBLEBUF;
		if(settings->getBoolValue("fullscreen"))
			flags|=SDL_FULLSCREEN;
		screen=SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP,flags);
		if(screen==NULL){
			fprintf(stderr,"FATAL ERROR: SDL_SetVideoMode failed\n");
			return false;
		}
	}
	
	//Create the temp surface, just a replica of the screen surface.
	tempSurface=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,
		screen->w,screen->h,screen->format->BitsPerPixel,
		screen->format->Rmask,screen->format->Gmask,screen->format->Bmask,0);
	
	//Set the the window caption.
	SDL_WM_SetCaption(("Me and my shadow "+version).c_str(),NULL);
	SDL_EnableUNICODE(1);
	
	//Nothing went wrong so return true.
	return true;
}

bool init(){
	//Initialze SDL.
	if(SDL_Init(SDL_INIT_EVERYTHING)==-1) {
		fprintf(stderr,"FATAL ERROR: SDL_Init failed\n");
		return false;
	}

	//Initialze SDL_mixer (audio).
	if(Mix_OpenAudio(22050,MIX_DEFAULT_FORMAT,2,512)==-1){
		fprintf(stderr,"FATAL ERROR: Mix_OpenAudio failed\n");
		return false;
	}
	//Set the volume.
	Mix_Volume(-1,atoi(settings->getValue("sound").c_str()));

	//Initialze SDL_ttf (fonts).
	if(TTF_Init()==-1){
		fprintf(stderr,"FATAL ERROR: TTF_Init failed\n");
		return false;
	}

	//Create the screen.
	if(!createScreen())
		return false;
	
	//Init tinygettext for translations for the right language
	dictionaryManager = new tinygettext::DictionaryManager();
	dictionaryManager->add_directory(getDataPath()+"locale");
	dictionaryManager->set_charset("UTF-8");
	
	//Check if user have defined own language. If not, find it out for the player using findlocale
	string lang=getSettings()->getValue("lang");
	if(lang.length()>0){
		printf("Locale set by user to %s\n",lang.c_str());
		language=lang;
	}else{
		FL_Locale *locale;
		FL_FindLocale(&locale,FL_MESSAGES);
		printf("Locale isn't set by user: %s\n",locale->lang);

		language=locale->lang;
		if(locale->country!=NULL){
			language+=string("_")+string(locale->country);
		}
		if(locale->variant!=NULL){
			language+=string("@")+string(locale->variant);
		}

		FL_FreeLocale(&locale);
	}
	
	//Now set the language in the dictionaryManager.
	dictionaryManager->set_language(tinygettext::Language::from_name(language));

	//Create the types of blocks.
	for(int i=0;i<TYPE_MAX;i++){
		Game::blockNameMap[Game::blockName[i]]=i;
	}

	//Nothing went wrong so we return true.
	return true;
}

static TTF_Font* loadFont(const char* name,int size){
	TTF_Font* tmpFont=TTF_OpenFont((getDataPath()+"font/"+name+".ttf").c_str(),size);
	if(tmpFont){
		return tmpFont;
	}else{
		return TTF_OpenFont((getDataPath()+"font/freesans.ttf").c_str(),size);
	}
}

bool loadFonts(){
  	//Load the fonts.
  	//NOTE: This is a separate method because it will be called separately when re-initing in case of language change.
  	
  	//First close the fonts if needed.
  	if(!fontTitle)
		TTF_CloseFont(fontTitle);
	if(!fontGUI)
		TTF_CloseFont(fontGUI);
	if(!fontText)
		TTF_CloseFont(fontText);
  	
	/// TRANSLATORS: Font used in GUI:
	///  - Use "knewave" for languages using Latin and Latin-derived alphabets
	///  - "freesans" can be used for non-Latin writing systems
	fontTitle=loadFont(_("knewave"),55);
	fontGUI=loadFont(_("knewave"),32);
	/// TRANSLATORS: Font used for normal text:
	///  - Use "Blokletters-Viltstift" for languages using Latin and Latin-derived alphabets
	///  - "freesans" can be used for non-Latin writing systems
	fontText=loadFont(_("Blokletters-Viltstift"),16);
	if(fontTitle==NULL || fontGUI==NULL || fontText==NULL){
		printf("ERROR: Unable to load fonts! \n");
		return false;
	}
	
	//Nothing went wrong so return true.
	return true;
}

bool loadTheme(){
	//Load the menu background.
	menuBackground=loadImage(getDataPath()+"gfx/menu/background.png");
	if(menuBackground==NULL){
		printf("ERROR: Unable to load menu background.\n");
		return false;
	}
	//Check if the menu background needs to be scaled.
	if(menuBackground->w!=SCREEN_WIDTH || menuBackground->h!=SCREEN_HEIGHT){
		menuBackground=zoomSurface(menuBackground,double(SCREEN_WIDTH)/double(menuBackground->w),double(SCREEN_HEIGHT)/double(menuBackground->h),0);
	}

	//Load the default theme.
	if(objThemes.appendThemeFromFile(getDataPath()+"themes/Cloudscape/theme.mnmstheme")==NULL){
		printf("ERROR: Can't load default theme file\n");
		return false;
	}
	
	//Everything went fine so return true.
	return true;
}

bool loadFiles(){
	//Load the music and play it.
	if(musicManager.loadMusic((getDataPath()+"music/menu.music")).empty()){
		printf("WARNING: Unable to load background music! \n");
	}
	musicManager.playMusic("menu",false);
	//Always load the default music list for fallback.
	musicManager.loadMusicList((getDataPath()+"music/default.list"));
	
	//Load the fonts.
	if(!loadFonts())
		return false;
	
	//Now sum up all the levelpacks.
	vector<string> v=enumAllDirs(getDataPath()+"levelpacks/");
	for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
		levelPackManager.loadLevelPack(getDataPath()+"levelpacks/"+*i);
	}
	v=enumAllDirs(getUserPath(USER_DATA)+"levelpacks/");
	for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
		levelPackManager.loadLevelPack(getUserPath(USER_DATA)+"levelpacks/"+*i);
	}
	v=enumAllDirs(getUserPath(USER_DATA)+"custom/levelpacks/");
	for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
		levelPackManager.loadLevelPack(getUserPath(USER_DATA)+"custom/levelpacks/"+*i);
	}
	//Now we add a special levelpack that will contain the levels not in a levelpack.
	LevelPack* levelsPack=new LevelPack;
	levelsPack->levelpackName="Levels";
	LevelPack* customLevelsPack=new LevelPack;
	customLevelsPack->levelpackName="Custom Levels";

	//List the addon levels and add them one for one.
	v=enumAllFiles(getUserPath(USER_DATA)+"levels/");
	for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
		levelsPack->addLevel(getUserPath(USER_DATA)+"levels/"+*i);
		levelsPack->setLocked(levelsPack->getLevelCount()-1);
	}
	//List the custom levels and add them one for one.
	v=enumAllFiles(getUserPath(USER_DATA)+"custom/levels/");
	for(vector<string>::iterator i=v.begin(); i!=v.end(); ++i){
		levelsPack->addLevel(getUserPath(USER_DATA)+"custom/levels/"+*i);
		levelsPack->setLocked(levelsPack->getLevelCount()-1);
		
		customLevelsPack->addLevel(getUserPath(USER_DATA)+"custom/levels/"+*i);
		customLevelsPack->setLocked(customLevelsPack->getLevelCount()-1);
	}
	
	//Add them to the manager.
	levelPackManager.addLevelPack(levelsPack);
	levelPackManager.addLevelPack(customLevelsPack);

	
	//Load the theme, both menu and default.
	if(!loadTheme())
		return false;
	
	//Nothing failed so return true.
	return true;
}

bool loadSettings(){
	settings=new Settings(getUserPath(USER_CONFIG)+"meandmyshadow.cfg");
	settings->parseFile();
	
	//Now apply settings changed through command line arguments, if any.
	map<string,string>::iterator it;
	for(it=tmpSettings.begin();it!=tmpSettings.end();++it){
		settings->setValue(it->first,it->second);
	}
	tmpSettings.clear();
  
	//Always return true?
	return true;
}

bool saveSettings(){
	settings->save();

	//Always return true?
	return true;
}

Settings* getSettings(){
	return settings;
}

MusicManager* getMusicManager(){
	return &musicManager;
}

LevelPackManager* getLevelPackManager(){
	return &levelPackManager;
}

void flipScreen(){
	if(settings->getBoolValue("gl")){
#ifdef HARDWARE_ACCELERATION
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		
		//Create a texture from the screen surface.
		glBindTexture(GL_TEXTURE_2D,screenTexture);
 
		//Set the texture's stretching properties
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 
		glTexImage2D(GL_TEXTURE_2D,0,screen->format->BytesPerPixel,screen->w,screen->h,0,GL_BGRA,GL_UNSIGNED_BYTE,screen->pixels);
		
		glBegin(GL_QUADS);
			glTexCoord2i(0,0); glVertex3f(0,0,0);
			glTexCoord2i(1,0); glVertex3f(SCREEN_WIDTH,0,0);
			glTexCoord2i(1,1); glVertex3f(SCREEN_WIDTH,SCREEN_HEIGHT,0);
			glTexCoord2i(0,1); glVertex3f(0,SCREEN_HEIGHT,0);
		glEnd();
		
		SDL_GL_SwapBuffers();
#else
		//NOTE: Trying to flip the screen using gl while compiled without.
		cerr<<"FATAL ERROR: Unable to draw to screen using OpenGL (compiled without)."<<endl;
#endif
	}else{
		SDL_Flip(screen);
	}
}

void clean(){
	//We delete the settings.
	if(settings){
		delete settings;
		settings=NULL;
	}

	//Get rid of the currentstate/
	//NOTE: The state is probably already deleted by the changeState function.
	if(currentState)
		delete currentState;

	//Destroy the GUI if present.
	if(GUIObjectRoot){
		delete GUIObjectRoot;
		GUIObjectRoot=NULL;
	}
	
	//Destroy the imageManager.
	imageManager.destroy();
	
	//Destroy the musicManager.
	musicManager.destroy();
	
	//Destroy the levelPackManager.
	levelPackManager.destroy();
	levels=NULL;
	
	//Close the fonts and quit SDL_ttf.
	TTF_CloseFont(fontTitle);
	TTF_CloseFont(fontGUI);
	TTF_CloseFont(fontText);
	TTF_Quit();
	
	//Quit SDL.
	SDL_Quit();
	
	//And finally stop audio.
	Mix_CloseAudio();
}

void setNextState(int newstate){
	//Only change the state when we aren't already exiting.
	if(nextState!=STATE_EXIT){
		nextState=newstate;
	}
}

void changeState(){
	//Check if there's a nextState.
	if(nextState!=STATE_NULL){
		//Delete the currentState.
		delete currentState;
		currentState=NULL;

		//Set the currentState to the nextState.
		stateID=nextState;
		nextState=STATE_NULL;

		//Init the state.
		switch(stateID){
		case STATE_GAME:
			currentState=new Game();
			break;
		case STATE_MENU:
			currentState=new Menu();
			break;
		case STATE_LEVEL_SELECT:
			currentState=new LevelPlaySelect();
			break;
		case STATE_LEVEL_EDIT_SELECT:
			currentState=new LevelEditSelect();
			break;
		case STATE_LEVEL_EDITOR:
			currentState=new LevelEditor();
			break;
		case STATE_OPTIONS:
			currentState=new Options();
			break;
		case STATE_ADDONS:
			currentState=new Addons();
			break;  
		}
		//NOTE: STATE_EXIT isn't mentioned, meaning that currentState is null.
		//This way the game loop will break and the program will exit.
		
		//Fade out.
		int fade=255;
		SDL_BlitSurface(screen,NULL,tempSurface,NULL);
		while(fade>0){
			fade-=17;
			if(fade<0)
				fade=0;
			
			
			SDL_FillRect(screen,NULL,0);
			SDL_SetAlpha(tempSurface, SDL_SRCALPHA, fade);
			SDL_BlitSurface(tempSurface,NULL,screen,NULL);
			flipScreen();
			SDL_Delay(25);
		}
	}
}

void musicStoppedHook(){
	//We just call the musicStopped method of the MusicManager.
	musicManager.musicStopped();
}

bool checkCollision(const SDL_Rect& a,const SDL_Rect& b){
	//Check if the left side of box a isn't past the right side of b.
	if(a.x>=b.x+b.w){
		return false;
	}
	//Check if the right side of box a isn't left of the left side of b.
	if(a.x+a.w<=b.x){
		return false;
	}
	//Check if the top side of box a isn't under the bottom side of b.
	if(a.y>=b.y+b.h){
		return false;
	}
	//Check if the bottom side of box a isn't above the top side of b.
	if(a.y+a.h<=b.y){
		return false;
	}

	//We have collision.
	return true;
}

void setCamera(const SDL_Rect& r1,const SDL_Rect& r2){
	//SetCamera only works in the Level editor.
	if(stateID==STATE_LEVEL_EDITOR){
		//Get the mouse coordinates.
		int x,y;
		SDL_GetMouseState(&x,&y);
		
		//Make sure we avoid the toolbar.
		SDL_Rect mouse={x,y,0,0};
		//SDL_Rect toolbar={(SCREEN_WIDTH-460)/2,SCREEN_HEIGHT-50,460,50};
		if(checkCollision(mouse,r1))
			return;
		
		if(checkCollision(mouse,r2))
			return;

		//Check if the mouse is near the left edge of the screen.
		//Else check if the mouse is near the right edge.
		if(x<50){
			//We're near the left edge so move the camera.
			camera.x-=5;
		}else if(x>SCREEN_WIDTH-50){
			//We're near the right edge so move the camera.
			camera.x+=5;
		}

		//Check if the mouse is near the top edge of the screen.
		//Else check if the mouse is near the bottom edge.
		if(y<50){
			//We're near the top edge so move the camera.
			camera.y-=5;
		}else if(y>SCREEN_HEIGHT-50){
			//We're near the bottom edge so move the camera.
			camera.y+=5;
		}
	}
}

bool parseArguments(int argc, char** argv){
	//Loop through all arguments.
	//We start at one since 0 is the command itself.
	for(int i=1;i<argc;i++){
		string argument=argv[i];
		
		//Check if the argument is the data-dir.
		if(argument=="--data-dir"){
			//We need a second argument so we increase i.
			i++;
			if(i>=argc){
				printf("ERROR: Missing argument for command '%s'\n\n",argument.c_str());
				return false;
			}
			
			//Configure the dataPath with the given path.
			dataPath=argv[i];
			if(!getDataPath().empty()){
				char c=dataPath[dataPath.size()-1];
				if(c!='/'&&c!='\\') dataPath+="/";
			}
		}else if(argument=="--user-dir"){
			//We need a second argument so we increase i.
			i++;
			if(i>=argc){
				printf("ERROR: Missing argument for command '%s'\n\n",argument.c_str());
				return false;
			}
			
			//Configure the userPath with the given path.
			userPath=argv[i];
			if(!userPath.empty()){
				char c=userPath[userPath.size()-1];
				if(c!='/'&&c!='\\') userPath+="/";
			}
		}else if(argument=="-f" || argument=="-fullscreen" || argument=="--fullscreen"){
			tmpSettings["fullscreen"]="1";
		}else if(argument=="-w" || argument=="-windowed" || argument=="--windowed"){
			tmpSettings["fullscreen"]="0";
		}else if(argument=="-mv" || argument=="-music" || argument=="--music"){
			//We need a second argument so we increase i.
			i++;
			if(i>=argc){
				printf("ERROR: Missing argument for command '%s'\n\n",argument.c_str());
				return false;
			}
			
			//Now set the music volume.
			tmpSettings["music"]=argv[i];
		}else if(argument=="-sv" || argument=="-sound" || argument=="--sound"){
			//We need a second argument so we increase i.
			i++;
			if(i>=argc){
				printf("ERROR: Missing argument for command '%s'\n\n",argument.c_str());
				return false;
			}
			
			//Now set sound volume.
			tmpSettings["sound"]=argv[i];
		}else if(argument=="-set" || argument=="--set"){
			//We need a second and a third argument so we increase i.
			i+=2;
			if(i>=argc){
				printf("ERROR: Missing argument for command '%s'\n\n",argument.c_str());
				return false;
			}
			
			//And set the setting.
			tmpSettings[argv[i-1]]=argv[i];
		}else if(argument=="-v" || argument=="-version" || argument=="--version"){
			//Print the version.
			printf("Version: '%s'\n\n",version.c_str());
		}else if(argument=="-h" || argument=="-help" || argument=="--help"){
			//If the help is requested we'll return false without printing an error.
			//This way the usage/help text will be printed.
			return false;
		}else{
			//Any other argument is unknow so we return false.
			printf("ERROR: Unknown argument %s\n\n",argument.c_str());
			return false;
		}
	}
	
	//If everything went well we can return true.
	return true;
}

//Special structure that will recieve the GUIEventCallbacks of the messagebox.
struct msgBoxHandler:public GUIEventCallback{
public:
	//Integer containing the ret(urn) value of the messageBox.
	int ret;
public:
	//Constructor.
	msgBoxHandler():ret(0){}
	
	void GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType){
		//Make sure it's a click event.
		if(eventType==GUIEventClick){
			//Set the return value.
			ret=obj->value;
			
			//After a click event we can delete the GUI.
			if(GUIObjectRoot){
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}
		}
	}
};

msgBoxResult msgBox(string prompt,msgBoxButtons buttons,const string& title){
	//Create the event handler.
	msgBoxHandler objHandler;
	//The GUI objects.
	GUIObject* obj;
	//We keep a pointer to the original GUIObjectRoot for later.
	GUIObject* tmp=GUIObjectRoot;
	
	//Create the GUIObjectRoot, the height and y location is temp.
	//It depends on the content what it will be.
	GUIObjectRoot=new GUIObject((SCREEN_WIDTH-600)/2,200,600,200,GUIObjectFrame,title.c_str());
	
	//Integer containing the current y location used to grow dynamic depending on the content.
	int y=50;
	
	//Now process the prompt.
	{
		//Pointer to the string.
		char* lps=(char*)prompt.c_str();
		//Pointer to a character.
		char* lp=NULL;
		
		//We keep looping forever.
		//The only way out is with the break statement.
		for(;;){
			//As long as it's still the same sentence we continue.
			//It will stop when there's a newline or end of line.
			for(lp=lps;*lp!='\n'&&*lp!='\r'&&*lp!=0;lp++);
			
			//Store the character we stopped on. (End or newline)
			char c=*lp;
			//Set the character in the string to 0, making lps a string containing one sentence.
			*lp=0;
			
			//Integer used to center the sentence horizontally.
			int x;
			TTF_SizeText(fontText,lps,&x,NULL);
			x=(600-x)/2;
			
			//Add a GUIObjectLabel with the sentence.
			GUIObjectRoot->childControls.push_back(new GUIObject(x,y,584,25,GUIObjectLabel,lps));
			//Increase y with 25, about the height of the text.
			y+=25;
			
			//Check the stored character if it was a stop.
			if(c==0){
				//It was so break out of the for loop.
				lps=lp;
				break;
			}
			//It wasn't meaning more will follow.
			//We set lps to point after the "newline" forming a new string.
			lps=lp+1;
		}
	}
	//Add 70 to y to leave some space between the content and the buttons.
	y+=70;
	//Recalc the size of the message box.
	GUIObjectRoot->top=(SCREEN_HEIGHT-y)/2;
	GUIObjectRoot->height=y;
	
	//Now we need to add the buttons.
	//Integer containing the number of buttons to add.
	int count=0;
	//Array with the return codes for the buttons.
	int value[3]={0};
	//Array containing the captation for the buttons.
	string button[3]={"","",""};
	switch(buttons){
	case MsgBoxOKCancel:
		count=2;
		button[0]=_("OK");value[0]=MsgBoxOK;
		button[1]=_("Cancel");value[1]=MsgBoxCancel;
		break;
	case MsgBoxAbortRetryIgnore:
		count=3;
		button[0]=_("Abort");value[0]=MsgBoxAbort;
		button[1]=_("Retry");value[1]=MsgBoxRetry;
		button[2]=_("Ignore");value[2]=MsgBoxIgnore;
		break;
	case MsgBoxYesNoCancel:
		count=3;
		button[0]=_("Yes");value[0]=MsgBoxYes;
		button[1]=_("No");value[1]=MsgBoxNo;
		button[2]=_("Cancel");value[2]=MsgBoxCancel;
		break;
	case MsgBoxYesNo:
		count=2;
		button[0]=_("Yes");value[0]=MsgBoxYes;
		button[1]=_("No");value[1]=MsgBoxNo;
		break;
	case MsgBoxRetryCancel:
		count=2;
		button[0]=_("Retry");value[0]=MsgBoxRetry;
		button[1]=_("Cancel");value[1]=MsgBoxCancel;
		break;
	default:
		count=1;
		button[0]=_("OK");value[0]=MsgBoxOK;
		break;
	}
	
	//Now we start making the buttons.
	{
		//Calculate the x location (centered).
		int x=302-count*50;
		//Reduce y so that the buttons fit inside the frame.
		y-=40;
		
		//Loop to add the buttons.
		for(int i=0;i<count;i++,x+=100){
			obj=new GUIObject(x,y,96,36,GUIObjectButton,button[i].c_str(),value[i]);
			obj->eventCallback=&objHandler;
			GUIObjectRoot->childControls.push_back(obj);
		}
	}
	
	//Now we dim the screen and keep the GUI rendering/updating.
	SDL_FillRect(tempSurface,NULL,0);
	SDL_SetAlpha(tempSurface,SDL_SRCALPHA,155);
	SDL_BlitSurface(tempSurface,NULL,screen,NULL);
	while(GUIObjectRoot){
		while(SDL_PollEvent(&event)){
			GUIObjectHandleEvents(true);
			
			//Also check for the return, escape or backspace button.
			//escape = KEYUP.
			//backspace and return = KEYDOWN.
			if(count==1 && ((event.type==SDL_KEYUP && event.key.keysym.sym==SDLK_ESCAPE) ||
				(event.type==SDL_KEYDOWN && (event.key.keysym.sym==SDLK_RETURN || event.key.keysym.sym==SDLK_BACKSPACE)))){
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}
			
		}
		//Render the gui.
		if(GUIObjectRoot)
			GUIObjectRoot->render();
		flipScreen();
		SDL_Delay(30);
	}
	
	//We're done so set the original GUIObjectRoot back.
	GUIObjectRoot=tmp;
	//And return the result.
	return (msgBoxResult)objHandler.ret;
}

struct fileDialogHandler:public GUIEventCallback{
public:
	//The ret(urn) value, true=ok and false=cancel
	bool ret;
	//Boolean if it's a save dialog.
	bool isSave;
	//Boolean if the file should be verified.
	bool verifyFile;
	//Boolean if files should be listed instead of directories.
	bool files;
	
	//Pointer to the textfield containing the filename.
	GUIObject* txtName;
	//Pointer to the listbox containing the different files.
	GUIListBox* lstFile;
	
	//The extension the files listed should have.
	const char* extension;
	//The current filename.
	string fileName;
	//The current search path.
	string path;
	
	//Vector containing the search paths.
	vector<string> searchPath;
public:
	//Constructor.
	fileDialogHandler(bool isSave=false,bool verifyFile=false, bool files=true):ret(false),
		isSave(isSave),verifyFile(verifyFile),
		files(files),txtName(NULL){}
	
	void GUIEventCallback_OnEvent(std::string name,GUIObject* obj,int eventType){
		//Check for the ok event.
		if(name=="cmdOK"){
			//Get the entered fileName from the text field.
			std::string s=txtName->caption;

			//If it doesn't contain a slash we need to add the path to the fileName.
			if(s.find_first_of("/")==string::npos)
				s=path+s;
			
			//If the string empty we return.
			if(s.empty() || s.find_first_of("*?")!=string::npos)
				return;
			
			//We only need to check for extensions if it isn't a folder dialog.
			if(files){
				//If there isn't right extension add it.
				size_t found=s.find_first_of(".");
				if(found!=string::npos)
					s.replace(s.begin()+found+1,s.end(),extension);
				else if (s.substr(found+1)!=extension)
					s.append(string(".")+extension);
			}
			
			//Check if we should save or load the file.
			//
			if(isSave){
				//Open the file with read permission to check if it already exists.
				FILE* f;
				f=fopen(processFileName(s).c_str(),"rb");
				
				//Check if it exists.
				if(f){
					//Close the file.
					fclose(f);
					
					//Let the currentState render once to prevent multiple GUI overlapping and prevent the screen from going black.
					currentState->render();
					
					//Prompt the user with a Yes or No question.
					/// TRANSLATORS: Filename is coming before this text
					if(msgBox(tfm::format(_("%s already exists.\nDo you want to overwrite it?"),s),MsgBoxYesNo,_("Overwrite Prompt"))!=MsgBoxYes){
						//He answered no, so we return.
						return;
					}
				}
				
				//Check if we should verify the file.
				//Verifying only applies to files not to directories.
				if(verifyFile && files){
					//Open the file with write permission.
					f=fopen(processFileName(s).c_str(),"wb");
					
					//Check if their aren't problems.
					if(f){
						//Close the file.
						fclose(f);
					}else{
						//Let the currentState render once to prevent multiple GUI overlapping and prevent the screen from going black.
						currentState->render();
						
						//The file can't be opened so tell the user.
						msgBox(tfm::format(_("Can't open file %s."),s),MsgBoxOKOnly,_("Error"));
						return;
					}
				}
			}else if(verifyFile && files){
				//We need to verify a file for opening.
				FILE *f;
				f=fopen(processFileName(s).c_str(),"rb");
				
				//Check if it didn't fail.
				if(f){
					//Succes, so close the file.
					fclose(f);
				}else{
					//Let the currentState render once to prevent multiple GUI overlapping and prevent the screen from going black.
					currentState->render();
					
					//Unable to open file so tell the user.
					msgBox(tfm::format(_("Can't open file %s."),s),MsgBoxOKOnly,_("Error"));
					return;
				}
			}
			
			//If we haven't returned then it's fine.
			//Set the fileName to the chosen file.
			fileName=s;
			
			//Delete the GUI.
			if(GUIObjectRoot){
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}
			
			//Set return to true.
			ret=true;
		}else if(name=="cmdCancel"){
			//Cancel means we can kill the gui.
			if(GUIObjectRoot){
				delete GUIObjectRoot;
				GUIObjectRoot=NULL;
			}
		}else if(name=="lstFile"){
			//Get a pointer to the listbox.
			GUIListBox* obj1=lstFile;
			
			//Make sure the option exist and change textfield to it.
			if(obj1!=NULL && txtName!=NULL && obj1->value>=0 && obj1->value<(int)obj1->item.size()){
				txtName->caption=obj1->item[obj1->value];
			}
		}else if(name=="lstSearchIn"){
			//Get the searchpath listbox.
			GUISingleLineListBox *obj1=dynamic_cast<GUISingleLineListBox*>(obj);
			
			//Check if the entry exists.
			if(obj1!=NULL && lstFile!=NULL && obj1->value>=0 && obj1->value<(int)searchPath.size()){
				//Temp string.
				string s;
				
				//Get the new search path.
				path=searchPath[obj1->value];
				
				//Make sure it isn't empty.
				if(!path.empty()){
					//Process the filename.
					s=processFileName(path);
				}else{
					//It's empty so we give the userpath.
					s=getUserPath();
				}
				
				//Fill the list with files or directories.
				if(files) {
					lstFile->item=enumAllFiles(s,extension);
				}else
					lstFile->item=enumAllDirs(s);
				
				//Remove any selection from the list.
				lstFile->value=-1;
			}
		}
	}
};

bool fileDialog(string& fileName,const char* title,const char* extension,const char* path,bool isSave,bool verifyFile,bool files){
	//Pointer to GUIObject to make the GUI with.
	GUIObject* obj;
	//Pointer to the current GUIObjectRoot.
	//We keep it so we can put it back after closing the fileDialog.
	GUIObject* tmp=GUIObjectRoot;
	
	//Create the fileDialogHandler, used for event handling.
	fileDialogHandler objHandler(isSave,verifyFile,files);
	//Vector containing the pathNames.
	vector<string> pathNames;

	//Set the extension of the objHandler.
	objHandler.extension=extension;
	
	//We now need to splits the given path into multiple path names.
	if(path && path[0]){
		//The string isn't empty.
		//Pointer to the paths string.
		char* lp=(char*)path;
		//Pointer to the first newline.
		char* lps=strchr(lp,'\n');
		//Pointer used for checking if their's another newline.
		//It will indicate if it's the last set or not.
		char* lpe;
		
		//Check for a newline.
		if(lps){
			//We have newline(s) so loop forever.
			//We can only break out of the loop when the string ends.
			for(;;){
				//Add the first searchpath.
				//This is the beginning of the string (lp) to the first newline. (lps)
				objHandler.searchPath.push_back(string(lp,lps-lp));
				
				//We should have another newline so search for it.
				lpe=strchr(lps+1,'\n');
				if(lpe){
					//We found it so we add that to the pathname.
					pathNames.push_back(string(lps+1,lpe-lps-1));
					//And start over again by setting lp to the start of a new set of searchPath/pathName.
					lp=lpe+1;
				}else{
					//There is no newline anymore, meaning the last entry, the rest of the string must be the pathName.
					pathNames.push_back(string(lps+1));
					//And break out of the loop.
					break;
				}
				
				//We haven't broken out so search for a newline.
				lps=strchr(lp,'\n');
				//If there isn't a newline break.
				if(!lps) 
					break;
			}
		}else{
			//There is no newline thus the whole string is the searchPath.
			objHandler.searchPath.push_back(path);
		}
	}else{
		//Empty so put an empty string as searchPath.
		objHandler.searchPath.push_back(string());
	}
	
	//It's time to create the GUI.
	//If there are more than one pathNames we need to add a GUISingleLineListBox.
	int base_y=pathNames.size()>0?60:20;
	
	//Create the frame.
	GUIObjectRoot=new GUIObject(100,100-base_y/2,600,400+base_y,GUIObjectFrame,title?title:(isSave?_("Save File"):_("Load File")));
	
	//Create the search path list box if needed.
	if(pathNames.size()>0){
		GUIObjectRoot->childControls.push_back(new GUIObject(8,40,184,36,GUIObjectLabel,_("Search In")));
		GUISingleLineListBox* obj1=new GUISingleLineListBox(160,40,432,36);
		obj1->item=pathNames;
		obj1->value=0;
		obj1->name="lstSearchIn";
		obj1->eventCallback=&objHandler;
		GUIObjectRoot->childControls.push_back(obj1);
	}
	
	//Add the FileName label and textfield.
	GUIObjectRoot->childControls.push_back(new GUIObject(8,20+base_y,184,36,GUIObjectLabel,_("File Name")));
	{
		//Fill the textbox with the given fileName.
		string s=fileName;
		
		if(!isSave){
			//But only if it isn't empty.
			if(s.empty() && extension && extension[0])
				s=string("*.")+string(extension);
		}
		
		//Create the textbox and add it to the GUI.
		objHandler.txtName=new GUIObject(160,20+base_y,432,36,GUIObjectTextBox,s.c_str());
		GUIObjectRoot->childControls.push_back(objHandler.txtName);
	}
	
	//Now we add the ListBox containing the files or directories.
	{
		GUIListBox* obj1=new GUIListBox(8,60+base_y,584,292);
		
		//Get the searchPath.
		string s=objHandler.searchPath[0];
		//Make sure it isn't empty.
		if(!s.empty()){
			objHandler.path=s;
			s=processFileName(s);
		}else{
			s=getUserPath();
		}
		
		//Check if we should list files or directories.
		if(files){
			//Fill the list with files.
			obj1->item=enumAllFiles(s,extension);
		}else{
			//Fill the list with directories.
			obj1->item=enumAllDirs(s);
		}
		obj1->name="lstFile";
		obj1->eventCallback=&objHandler;
		GUIObjectRoot->childControls.push_back(obj1);
		objHandler.lstFile=obj1;
	}
	
	//Now create the OK and Cancel buttons.
	obj=new GUIObject(200,360+base_y,192,36,GUIObjectButton,_("OK"));
	obj->name="cmdOK";
	obj->eventCallback=&objHandler;
	GUIObjectRoot->childControls.push_back(obj);
	obj=new GUIObject(400,360+base_y,192,36,GUIObjectButton,_("Cancel"));
	obj->name="cmdCancel";
	obj->eventCallback=&objHandler;
	GUIObjectRoot->childControls.push_back(obj);

	//Now we keep rendering and updating the GUI.
	SDL_FillRect(tempSurface,NULL,0);
	SDL_SetAlpha(tempSurface,SDL_SRCALPHA,155);
	SDL_BlitSurface(tempSurface,NULL,screen,NULL);
	while(GUIObjectRoot){
		while(SDL_PollEvent(&event)) 
			GUIObjectHandleEvents(true);
		if(GUIObjectRoot)
			GUIObjectRoot->render();
		flipScreen();
		SDL_Delay(30);
	}
	
	//The while loop ended meaning we can restore the previous GUI.
	GUIObjectRoot=tmp;
	
	//Now determine what the return value is (and if there is one).
	if(objHandler.ret) 
		fileName=objHandler.fileName;
	return objHandler.ret;
}
