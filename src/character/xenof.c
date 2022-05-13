/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "xenof.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//XenoF character structure
enum
{
	XenoF_ArcMain_Idle0,
	XenoF_ArcMain_Idle1,
	XenoF_ArcMain_Idle2,
	XenoF_ArcMain_Idle3,
	XenoF_ArcMain_Idle4,
	XenoF_ArcMain_Idle5,
	XenoF_ArcMain_Left0,
	XenoF_ArcMain_Left1,
	XenoF_ArcMain_Down0,
	XenoF_ArcMain_Down1,
	XenoF_ArcMain_Up0,
	XenoF_ArcMain_Up1,
	XenoF_ArcMain_Right0,
	XenoF_ArcMain_Right1,
	XenoF_ArcMain_Laugh0,
	XenoF_ArcMain_Laugh1,
	
	XenoF_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[XenoF_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_XenoF;

//XenoF character definitions
static const CharFrame char_xenof_frame[] = {
	{XenoF_ArcMain_Idle0, {  0,   0, 156, 235}, { 92, 233}}, //0 idle 1
	{XenoF_ArcMain_Idle1, {  0,   0, 156, 235}, { 92, 233}}, //1 idle 2
	{XenoF_ArcMain_Idle2, {  0,   0, 156, 235}, { 92, 233}}, //2 idle 3
	{XenoF_ArcMain_Idle3, {  0,   0, 156, 235}, { 92, 233}}, //3 idle 4	
	{XenoF_ArcMain_Idle4, {  0,   0, 156, 235}, { 92, 233}}, //4 idle 5
	{XenoF_ArcMain_Idle5, {  0,   0, 156, 235}, { 92, 233}}, //5 idle 6
	
	{XenoF_ArcMain_Left0, {  0,   0, 236, 213}, { 133, 210}}, //6 left 1
	{XenoF_ArcMain_Left1, {  0,   0, 236, 213}, { 133, 210}}, //7 left 2
	
	{XenoF_ArcMain_Down0, {  0,   0, 156, 214}, { 91, 210}}, //8 down 1
	{XenoF_ArcMain_Down1, {  0,   0, 156, 214}, { 91, 210}}, //9 down 2
	
	{XenoF_ArcMain_Up0, {   0,   0, 156, 255}, { 89, 252}}, //10 up 1
	{XenoF_ArcMain_Up1, {   0,   0, 156, 255}, { 89, 252}}, //11 up 2
	
	{XenoF_ArcMain_Right0, {  0,   0, 196, 219}, { 36, 218}}, //12 right 1
	{XenoF_ArcMain_Right1, {  0,   0, 196, 219}, { 36, 218}}, //13 right 2
	
	{XenoF_ArcMain_Laugh0, {  0,   0, 148, 181}, { 59, 178}}, //14 laugh 1
	{XenoF_ArcMain_Laugh1, {  0,   0, 148, 181}, { 59, 178}}, //15 laugh 2
};

static const Animation char_xenof_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5, ASCR_BACK, 1}},     		   //CharAnim_Idle
	{2, (const u8[]){12, 13, ASCR_BACK, 1}},       		   		   		   //CharAnim_Left
	{2, (const u8[]){14, 15, ASCR_BACK, 1}},                       		   //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},  		     		   		   //CharAnim_Down
	{2, (const u8[]){14, 15, ASCR_BACK, 1}},                       		   //CharAnim_DownAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},					   		   //CharAnim_Up
	{2, (const u8[]){14, 15, ASCR_BACK, 1}},                      		   //CharAnim_UpAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}}, 					     	   //CharAnim_Right
	{2, (const u8[]){14, 15, ASCR_BACK, 1}},                       		   //CharAnim_RightAlt
};

//XenoF character functions
void Char_XenoF_SetFrame(void *user, u8 frame)
{
	Char_XenoF *this = (Char_XenoF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_xenof_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_XenoF_Tick(Character *character)
{
	Char_XenoF *this = (Char_XenoF*)character;
	
	//Perform idle dance
	Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_XenoF_SetFrame);
	Character_DrawFlipped(character, &this->tex, &char_xenof_frame[this->frame]);
}

void Char_XenoF_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_XenoF_Free(Character *character)
{
	Char_XenoF *this = (Char_XenoF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_XenoF_New(fixed_t x, fixed_t y)
{
	//Allocate xenof object
	Char_XenoF *this = Mem_Alloc(sizeof(Char_XenoF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_XenoF_New] Failed to allocate xenof object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_XenoF_Tick;
	this->character.set_anim = Char_XenoF_SetAnim;
	this->character.free = Char_XenoF_Free;
	
	Animatable_Init(&this->character.animatable, char_xenof_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character stage information
	this->character.health_i = 2;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\XENOF.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //XenoF_ArcMain_Idle0
		"idle1.tim", //XenoF_ArcMain_Idle1
		"idle2.tim", //XenoF_ArcMain_Idle2
		"idle3.tim", //XenoF_ArcMain_Idle3
		"idle4.tim", //XenoF_ArcMain_Idle4
		"idle5.tim", //XenoF_ArcMain_Idle5
		"left0.tim",  //XenoF_ArcMain_Left0
		"left1.tim",  //XenoF_ArcMain_Left1
		"down0.tim",  //XenoF_ArcMain_Down0
		"down1.tim",  //XenoF_ArcMain_Down1
		"up0.tim",    //XenoF_ArcMain_Up0
		"up1.tim",    //XenoF_ArcMain_Up1
		"right0.tim", //XenoF_ArcMain_Right0
		"right1.tim", //XenoF_ArcMain_Right1
		"laugh0.tim", //XenoF_ArcMain_Laugh0
		"laugh1.tim", //XenoF_ArcMain_Laugh1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
