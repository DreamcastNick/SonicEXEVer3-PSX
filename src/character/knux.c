/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "knux.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Knux character structure
enum
{
	Knux_ArcMain_Idle0,
	Knux_ArcMain_Idle1,
	Knux_ArcMain_Left,
	Knux_ArcMain_Down,
	Knux_ArcMain_Up,
	Knux_ArcMain_Right,
	
	Knux_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Knux_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Knux;

//Knux character definitions
static const CharFrame char_knux_frame[] = {
	{Knux_ArcMain_Idle0, {  4,   4,  74, 118}, { 62, 113}},  //0 idle 1
	{Knux_ArcMain_Idle0, { 86,   4,  74, 118}, { 62, 113}},  //1 idle 2
	{Knux_ArcMain_Idle0, {  4, 130,  74, 118}, { 62, 113}},  //2 idle 3
	{Knux_ArcMain_Idle0, { 86, 130,  74, 118}, { 62, 113}},  //3 idle 4
	{Knux_ArcMain_Idle1, {  4,   4,  74, 118}, { 62, 113}},  //4 idle 5
	{Knux_ArcMain_Idle1, { 86,   4,  74, 118}, { 62, 113}},  //5 idle 6
	{Knux_ArcMain_Idle1, {  4, 130,  74, 118}, { 62, 113}},  //6 idle 7
	{Knux_ArcMain_Idle1, { 86, 130,  74, 118}, { 62, 113}},  //7 idle 8
	
	{Knux_ArcMain_Left, {  5,   4, 101, 103}, { 96, 98}}, //8 left 1
	{Knux_ArcMain_Left, {114,   4, 101, 103}, { 96, 98}}, //9 left 2
	{Knux_ArcMain_Left, {  5, 115, 101, 103}, { 96, 98}}, //10 left 3
	{Knux_ArcMain_Left, {114, 115, 101, 103}, { 96, 98}}, //11 left 4
	
	{Knux_ArcMain_Down, {  4,   4, 100,  94}, { 78, 89}}, //12 down 1
	{Knux_ArcMain_Down, {112,   4, 100,  94}, { 78, 89}}, //13 down 2
	{Knux_ArcMain_Down, {  4, 106, 100,  94}, { 78, 89}}, //14 down 3
	{Knux_ArcMain_Down, {112, 106, 100,  94}, { 78, 89}}, //15 down 4
	
	{Knux_ArcMain_Up, {  7,   4,  87, 131}, { 70, 126}}, //16 up 1
	{Knux_ArcMain_Up, {102,   4,  87, 131}, { 70, 126}}, //17 up 2
	
	{Knux_ArcMain_Right, {  5,   4, 101, 102}, { 52, 96}}, //18 right 1
	{Knux_ArcMain_Right, {114,   4, 101, 102}, { 52, 96}}, //19 right 2
	{Knux_ArcMain_Right, {  5, 114, 101, 102}, { 52, 96}}, //20 right 3
	{Knux_ArcMain_Right, {114, 114, 101, 102}, { 52, 96}}, //21 right 4
};

static const Animation char_knux_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5,  6,  7, ASCR_BACK, 1}},     		   		   			//CharAnim_Idle
	{2, (const u8[]){18, 19, 20, 21, ASCR_BACK, 1}},       		   		    	   					//CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                     	   							//CharAnim_LeftAlt
	{2, (const u8[]){12, 13, 14, 15, ASCR_BACK, 1}},  		     		   		   					//CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   	          	       							//CharAnim_DownAlt
	{2, (const u8[]){16, 17, ASCR_BACK, 1}},					  				  			  		//CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},             			            				//CharAnim_UpAlt
	{2, (const u8[]){ 8,  9, 10, 11, ASCR_BACK, 1}}, 					   		   					//CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                         							//CharAnim_RightAlt
};

//Knux character functions
void Char_Knux_SetFrame(void *user, u8 frame)
{
	Char_Knux *this = (Char_Knux*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_knux_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Knux_Tick(Character *character)
{
	Char_Knux *this = (Char_Knux*)character;
	
	//Perform idle dance
	Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Knux_SetFrame);

	if (stage.stage_id == StageId_1_3 && stage.song_step >= 1296 && stage.song_step <= 2320)
	Character_Draw(character, &this->tex, &char_knux_frame[this->frame]);
}

void Char_Knux_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Knux_Free(Character *character)
{
	Char_Knux *this = (Char_Knux*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Knux_New(fixed_t x, fixed_t y)
{
	//Allocate knux object
	Char_Knux *this = Mem_Alloc(sizeof(Char_Knux));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Knux_New] Failed to allocate knux object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Knux_Tick;
	this->character.set_anim = Char_Knux_SetAnim;
	this->character.free = Char_Knux_Free;
	
	Animatable_Init(&this->character.animatable, char_knux_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character stage information
	this->character.health_i = 2;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\KNUX.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Knux_ArcMain_Idle0
		"idle1.tim", //Knux_ArcMain_Idle1
		"left.tim",   //Knux_ArcMain_Left
		"down.tim",   //Knux_ArcMain_Down
		"up.tim",     //Knux_ArcMain_Up
		"right.tim",  //Knux_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
