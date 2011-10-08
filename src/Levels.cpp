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
#include "Levels.h"
#include "Functions.h"
#include "TreeStorageNode.h"
#include "POASerializer.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

void Level::clear(){
	i_level_count = 0;
	i_current_level = 0;
	m_bLoaded = false;
	level_name.clear();
	level_files.clear();
	level_locked.clear();
	LevelPackName.clear();
	m_sLevelProgressFile.clear();
}

bool Level::load_levels(const std::string& level_list_file,const std::string& level_progress_file){
	clear();

	if(level_list_file.empty()) return false;

	string level_list_new=ProcessFileName(level_list_file);

	ifstream level ( level_list_new.c_str() );
	ifstream level_progress;

	if(!level){
		cerr<<"Error: Can't load level list "<<level_list_new<<endl;
		return false;
	}
	
	if(!level_progress_file.empty()){
		m_sLevelProgressFile=level_progress_file;
		level_progress.open( ProcessFileName(level_progress_file).c_str() );
	}

	TreeStorageNode obj;
	{
		POASerializer objSerializer;
		if(!objSerializer.ReadNode(level,&obj,true)){
			cerr<<"Error: Invalid file format of level list "<<level_list_new<<endl;
			return false;
		}
	}

	{
		vector<string> &v=obj.attributes["name"];
		if(v.size()>0) LevelPackName=v[0];
	}

	for(unsigned int i=0;i<obj.subNodes.size();i++){
		TreeStorageNode* obj1=obj.subNodes[i];
		if(obj1==NULL) continue;
		if(obj1->value.size()>=2 && obj1->name=="levelfile"){
			level_files.push_back(obj1->value[0]);
			level_name.push_back(obj1->value[1]);
			//load level progress
			int a=1;
			if(level_progress.is_open() && !level_progress.eof()) level_progress >> a;
			level_locked.push_back( !( a==0 || i_level_count==0 ) );
			//over
			i_level_count++;
		}
	}

	m_bLoaded=true;
	return true;
}

void Level::save_levels(const std::string& level_list_file){
	string level_list_new=ProcessFileName(level_list_file);

	ofstream level ( level_list_new.c_str() );

	if(!level){
		cerr<<"Error: Can't load level list "<<level_list_new<<endl;
		return;
	}
	
	TreeStorageNode obj;

	obj.attributes["name"].push_back(LevelPackName);

	for(int i=0;i<i_level_count;i++){
		TreeStorageNode* obj1=new TreeStorageNode;
		obj1->name="levelfile";
		obj1->value.push_back(level_files[i]);
		obj1->value.push_back(level_name[i]);
		obj.subNodes.push_back(obj1);
	}

	POASerializer objSerializer;
	objSerializer.WriteNode(&obj,level,false,true);
}

void Level::add_level(const string& level_file_name,const string& level_name,int level){
	if(level<0 || level>=i_level_count){
		level_files.push_back(level_file_name);
		Level::level_name.push_back(level_name);
		level_locked.push_back(i_level_count>0?true:false);
		i_level_count++;
	}else{
		level_files.insert(level_files.begin()+level,level_file_name);
		Level::level_name.insert(Level::level_name.begin()+level,level_name);
		level_locked.insert(level_locked.begin()+level,level>0?true:false);
		i_level_count++;
	}
}

void Level::save_level_progress()
{
	if(!m_bLoaded || m_sLevelProgressFile.empty()) return;

	ofstream level_progress ( ProcessFileName(m_sLevelProgressFile).c_str() );

	for ( int n = 0; n < i_level_count; n++ )
	{
		level_progress << (level_locked[n]?1:0) << "\n";
	}
}

const string& Level::get_level_name(int level)
{
	if(level<0) level=i_current_level;
	return level_name[level];
}

void Level::set_level_name(int lvl,const std::string& s){
	if(lvl>=0&&lvl<i_level_count) level_name[lvl]=s;
}

const string& Level::get_level_file(int level)
{
	if(level<0) level=i_current_level;
	return level_files[level];
}

void Level::next_level()
{
	i_current_level++;
}

bool Level::get_locked( int level )
{
	return level_locked[level];
}

void Level::set_level(int lvl)
{
	i_current_level = lvl;
}

void Level::set_locked(int lvl,bool bLocked)
{
	level_locked[lvl] = bLocked;
}

void Level::swap_level(int lvl1,int lvl2){
	if(lvl1>=0&&lvl1<i_level_count&&lvl2>=0&&lvl2<i_level_count){
		swap(level_files[lvl1],level_files[lvl2]);
		swap(level_name[lvl1],level_name[lvl2]);
		
		//Compile error under x86_64 Linux.
		//swap(level_locked[lvl1],level_locked[lvl2]);
		bool temp = level_locked[lvl1];
		level_locked[lvl1] = level_locked[lvl2];
		level_locked[lvl2] = temp;
	}
}

void Level::remove_level(int lvl){
	if(lvl>=0&&lvl<i_level_count){
		level_files.erase(level_files.begin()+lvl);
		level_name.erase(level_name.begin()+lvl);
		level_locked.erase(level_locked.begin()+lvl);
		i_level_count--;
	}
}