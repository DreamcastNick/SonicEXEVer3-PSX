/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "xeno.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Xeno character structure
enum
{
	Xeno_ArcMain_Idle0,
	Xeno_ArcMain_Idle1,
	Xeno_ArcMain_Idle2,
	Xeno_ArcMain_Idle3,
	Xeno_ArcMain_Idle4,
	Xeno_ArcMain_Idle5,
	Xeno_ArcMain_Left0,
	Xeno_ArcMain_Left1,
	Xeno_ArcMain_Down0,
	Xeno_ArcMain_Down1,
	Xeno_ArcMain_Up0,
	Xeno_ArcMain_Up1,
	Xeno_ArcMain_Right0,
	Xeno_ArcMain_Right1,
	Xeno_ArcMain_Laugh0,
	Xeno_ArcMain_Laugh1,
	
	Xeno_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Xeno_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Xeno;

//Xeno character definitions
static const CharFrame char_xeno_frame[] = {
	{Xeno_ArcMain_Idle0, {  0,   0, 156, 235}, { 92, 233}}, //0 idle 1
	{Xeno_ArcMain_Idle1, {  0,   0, 156, 235}, { 92, 233}}, //1 idle 2
	{Xeno_ArcMain_Idle2, {  0,   0, 156, 235}, { 92, 233}}, //2 idle 3
	{Xeno_ArcMain_Idle3, {  0,   0, 156, 235}, { 92, 233}}, //3 idle 4	
	{Xeno_ArcMain_Idle4, {  0,   0, 156, 235}, { 92, 233}}, //4 idle 5
	{Xeno_ArcMain_Idle5, {  0,   0, 156, 235}, { 92, 233}}, //5 idle 6
	
	{Xeno_ArcMain_Left0, {  0,   0, 236, 213}, { 133, 210}}, //6 left 1
	{Xeno_ArcMain_Left1, {  0,   0, 236, 213}, { 133, 210}}, //7 left 2
	
	{Xeno_ArcMain_Down0, {  0,   0, 156, 214}, { 91, 210}}, //8 down 1
	{Xeno_ArcMain_Down1, {  0,   0, 156, 214}, { 91, 210}}, //9 down 2
	
	{Xeno_ArcMain_Up0, {   0,   0, 156, 255}, { 89, 252}}, //10 up 1
	{Xeno_ArcMain_Up1, {   0,   0, 156, 255}, { 89, 252}}, //11 up 2
	
	{Xeno_ArcMain_Right0, {  0,   0, 196, 219}, { 36, 218}}, //12 right 1
	{Xeno_ArcMain_Right1, {  0,   0, 196, 219}, { 36, 218}}, //13 right 2
	
	{Xeno_ArcMain_Laugh0, {  0,   0, 148, 181}, { 59, 178}}, //14 laugh 1
	{Xeno_ArcMain_Laugh1, {  0,   0, 148, 181}, { 59, 178}}, //15 laugh 2
};

static const Animation char_xeno_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5, ASCR_BACK, 1}},     		   //CharAnim_Idle
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},       		   		   		   //CharAnim_Left
	{2, (const u8[]){14, 15, ASCR_BACK, 1}},                       		   //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},  		     		   		   //CharAnim_Down
	{2, (const u8[]){14, 15, ASCR_BACK, 1}},                       		   //CharAnim_DownAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},					   		   //CharAnim_Up
	{2, (const u8[]){14, 15, ASCR_BACK, 1}},                      		   //CharAnim_UpAlt
	{2, (const u8[]){12, 13, ASCR_BACK, 1}}, 					     	   //CharAnim_Right
	{2, (const u8[]){14, 15, ASCR_BACK, 1}},                       		   //CharAnim_RightAlt
};

//Xeno character functions
void Char_Xeno_SetFrame(void *user, u8 frame)
{
	Char_Xeno *this = (Char_Xeno*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_xeno_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Xeno_Tick(Character *character)
{
	Char_Xeno *this = (Char_Xeno*)character;
	
	//Perform idle dance
	Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Xeno_SetFrame);

	if (stage.stage_id == StageId_1_3 && stage.song_step >= 1040 && stage.song_step <= 1296)
	Character_Draw(character, &this->tex, &char_xeno_frame[this->frame]);
	if (stage.stage_id == StageId_1_3 && stage.song_step >= 2320 && stage.song_step <= 2832)
	Character_Draw(character, &this->tex, &char_xeno_frame[this->frame]);
	if (stage.stage_id == StageId_1_3 && stage.song_step >= 4111 && stage.song_step <= 5250)
	Character_Draw(character, &this->tex, &char_xeno_frame[this->frame]);
}

void Char_Xeno_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Xeno_Free(Character *character)
{
	Char_Xeno *this = (Char_Xeno*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Xeno_New(fixed_t x, fixed_t y)
{
	//Allocate xeno object
	Char_Xeno *this = Mem_Alloc(sizeof(Char_Xeno));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Xeno_New] Failed to allocate xeno object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Xeno_Tick;
	this->character.set_anim = Char_Xeno_SetAnim;
	this->character.free = Char_Xeno_Free;
	
	Animatable_Init(&this->character.animatable, char_xeno_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character stage information
	this->character.health_i = 2;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\XENO.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Xeno_ArcMain_Idle0
		"idle1.tim", //Xeno_ArcMain_Idle1
		"idle2.tim", //Xeno_ArcMain_Idle2
		"idle3.tim", //Xeno_ArcMain_Idle3
		"idle4.tim", //Xeno_ArcMain_Idle4
		"idle5.tim", //Xeno_ArcMain_Idle5
		"left0.tim",  //Xeno_ArcMain_Left0
		"left1.tim",  //Xeno_ArcMain_Left1
		"down0.tim",  //Xeno_ArcMain_Down0
		"down1.tim",  //Xeno_ArcMain_Down1
		"up0.tim",    //Xeno_ArcMain_Up0
		"up1.tim",    //Xeno_ArcMain_Up1
		"right0.tim", //Xeno_ArcMain_Right0
		"right1.tim", //Xeno_ArcMain_Right1
		"laugh0.tim", //Xeno_ArcMain_Laugh0
		"laugh1.tim", //Xeno_ArcMain_Laugh1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
