/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "tails.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Tails character structure
enum
{
	Tails_ArcMain_Idle0,
	Tails_ArcMain_Idle1,
	Tails_ArcMain_Left,
	Tails_ArcMain_Down,
	Tails_ArcMain_Up,
	Tails_ArcMain_Right,
	
	Tails_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Tails_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Tails;

//Tails character definitions
static const CharFrame char_tails_frame[] = {
	{Tails_ArcMain_Idle0, {  0,   0, 64, 103}, { 53, 103}},  //0 idle 1
	{Tails_ArcMain_Idle0, { 64,   0, 64, 103}, { 52, 103}},  //1 idle 2
	{Tails_ArcMain_Idle0, {128,   0, 63, 103}, { 52, 103}},  //2 idle 3
	{Tails_ArcMain_Idle0, {191,   0, 65, 103}, { 54, 103}},  //3 idle 4
	{Tails_ArcMain_Idle0, {  0, 103, 65, 103}, { 54, 103}},  //4 idle 5
	{Tails_ArcMain_Idle0, { 65, 103, 65, 103}, { 54, 103}},  //5 idle 6
	{Tails_ArcMain_Idle0, {130, 103, 65, 103}, { 54, 103}},  //6 idle 7
	{Tails_ArcMain_Idle1, {  0,   0, 65, 103}, { 54, 103}},  //7 idle 8
	{Tails_ArcMain_Idle1, { 65,   0, 65, 103}, { 54, 103}},  //8 idle 9
	{Tails_ArcMain_Idle1, {130,   0, 65, 103}, { 53, 103}},  //9 idle 10
	
	{Tails_ArcMain_Left, {  0,   0, 87, 99}, { 87, 99}},    //10 left 1
	{Tails_ArcMain_Left, { 87,   0, 85, 100}, { 85, 99}},   //11 left 2
	{Tails_ArcMain_Left, {172,   0, 84, 100}, { 84, 100}},  //12 left 3
	{Tails_ArcMain_Left, {  0,  99, 86, 100}, { 86, 100}},  //13 left 4
	{Tails_ArcMain_Left, { 86, 100, 85, 100}, { 85, 100}},  //14 left 5
	{Tails_ArcMain_Left, {171, 100, 85, 100}, { 85, 100}},  //15 left 6

	{Tails_ArcMain_Down, {  0,   0, 74, 88}, { 74, 88}},  //16 down 1
	{Tails_ArcMain_Down, { 74,   0, 73, 89}, { 73, 89}},  //17 down 2
	{Tails_ArcMain_Down, {147,   0, 74, 90}, { 74, 90}},  //18 down 3
	{Tails_ArcMain_Down, {  0,  88, 74, 90}, { 74, 90}},  //19 down 4
	{Tails_ArcMain_Down, { 74,  89, 74, 90}, { 73, 90}},  //20 down 5
	{Tails_ArcMain_Down, {148,  90, 74, 90}, { 73, 90}},  //21 down 6
	
	{Tails_ArcMain_Up, {  0,   0, 76, 113}, { 57, 113}},  //22 up 1
	{Tails_ArcMain_Up, { 76,   0, 76, 112}, { 57, 112}},  //23 up 2
	{Tails_ArcMain_Up, {152,   0, 77, 111}, { 58, 111}},  //24 up 3
	{Tails_ArcMain_Up, {  0, 113, 78, 111}, { 59, 111}},  //25 up 4
	{Tails_ArcMain_Up, { 78, 112, 78, 110}, { 59, 110}},  //26 up 5
	{Tails_ArcMain_Up, {156, 111, 78, 110}, { 59, 110}},  //27 up 6
	
	{Tails_ArcMain_Right, {  0,   0, 83, 99}, { 70, 99}},  //28 right 1
	{Tails_ArcMain_Right, { 83,   0, 80, 99}, { 70, 99}},  //29 right 2
	{Tails_ArcMain_Right, {163,   0, 80, 97}, { 70, 97}},  //30 right 3
	{Tails_ArcMain_Right, {  0,  99, 81, 97}, { 70, 97}},  //31 right 4
	{Tails_ArcMain_Right, { 81,  99, 81, 97}, { 70, 97}},  //32 right 5
	{Tails_ArcMain_Right, {162,  97, 81, 97}, { 70, 97}},  //33 right 6
};

static const Animation char_tails_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, ASCR_BACK, 0}},   //CharAnim_Idle
	{2, (const u8[]){ 10, 11, 12, 13, 14, 15, ASCR_BACK, 0}},         //CharAnim_Left
	{2, (const u8[]){ 10, 11, 12, 13, 14, 15, ASCR_BACK, 0}},         //CharAnim_LeftAlt
	{2, (const u8[]){ 16, 17, 18, 19, 20, 21, ASCR_BACK, 0}},         //CharAnim_Down
	{2, (const u8[]){ 16, 17, 18, 19, 20, 21, ASCR_BACK, 0}},   	  //CharAnim_DownAlt
	{2, (const u8[]){ 22, 23, 24, 25, 26, 27, ASCR_BACK, 0}},         //CharAnim_Up
	{2, (const u8[]){ 22, 23, 24, 25, 26, 27, ASCR_BACK, 0}},   	  //CharAnim_UpAlt
	{2, (const u8[]){ 28, 29, 30, 31, 32, 33, ASCR_BACK, 0}},         //CharAnim_Right
	{2, (const u8[]){ 28, 29, 30, 31, 32, 33, ASCR_BACK, 0}},  		  //CharAnim_RightAlt
};

//Tails character functions
void Char_Tails_SetFrame(void *user, u8 frame)
{
	Char_Tails *this = (Char_Tails*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_tails_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Tails_Tick(Character *character)
{
	Char_Tails *this = (Char_Tails*)character;

	if (stage.stage_id == StageId_1_3 && stage.song_step >= 1040)
	this->character.health_i = 1;

	if (stage.stage_id == StageId_1_3 && stage.song_step >= 1296)
	this->character.health_i = 3;

	if (stage.stage_id == StageId_1_3 && stage.song_step >= 2320)
	this->character.health_i = 1;

	if (stage.stage_id == StageId_1_3 && stage.song_step >= 2832)
	this->character.health_i = 4;

	if (stage.stage_id == StageId_1_3 && stage.song_step >= 4112)
	this->character.health_i = 1;


	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Tails_SetFrame);

	if (stage.stage_id == StageId_1_3 && stage.song_step >= -100 && stage.song_step <= 1040)
	Character_Draw(character, &this->tex, &char_tails_frame[this->frame]);
}

void Char_Tails_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Tails_Free(Character *character)
{
	Char_Tails *this = (Char_Tails*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Tails_New(fixed_t x, fixed_t y)
{
	//Allocate tails object
	Char_Tails *this = Mem_Alloc(sizeof(Char_Tails));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Tails_New] Failed to allocate tails object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Tails_Tick;
	this->character.set_anim = Char_Tails_SetAnim;
	this->character.free = Char_Tails_Free;
	
	Animatable_Init(&this->character.animatable, char_tails_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 2;

	this->character.focus_x = FIXED_DEC(55,1);
	this->character.focus_y = FIXED_DEC(-75,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\TAILS.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Tails_ArcMain_Idle0
		"idle1.tim", //Tails_ArcMain_Idle1
		"left.tim",  //Tails_ArcMain_Left
		"down.tim",  //Tails_ArcMain_Down
		"up.tim",    //Tails_ArcMain_Up
		"right.tim", //Tails_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
