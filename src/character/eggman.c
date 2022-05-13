/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "eggman.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Eggman character structure
enum
{
	Eggman_ArcMain_Idle0,
	Eggman_ArcMain_Idle1,
	Eggman_ArcMain_Idle2,
	Eggman_ArcMain_Idle3,
	Eggman_ArcMain_Idle4,
	Eggman_ArcMain_Idle5,
	Eggman_ArcMain_Left0,
	Eggman_ArcMain_Left1,
	Eggman_ArcMain_Left2,
	Eggman_ArcMain_Left3,
	Eggman_ArcMain_Down0,
	Eggman_ArcMain_Down1,
	Eggman_ArcMain_Down2,
	Eggman_ArcMain_Down3,
	Eggman_ArcMain_Up0,
	Eggman_ArcMain_Up1,
	Eggman_ArcMain_Up2,
	Eggman_ArcMain_Up3,
	Eggman_ArcMain_Right0,
	Eggman_ArcMain_Right1,
	Eggman_ArcMain_Right2,
	Eggman_ArcMain_Right3,
	Eggman_ArcMain_Laugh0,
	Eggman_ArcMain_Laugh1,
	Eggman_ArcMain_Laugh2,
	Eggman_ArcMain_Laugh3,
	Eggman_ArcMain_Laugh4,
	Eggman_ArcMain_Laugh5,
	Eggman_ArcMain_Laugh6,
	
	Eggman_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Eggman_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Eggman;

//Eggman character definitions
static const CharFrame char_eggman_frame[] = {
	{Eggman_ArcMain_Idle0, {  0,   0, 128, 146}, { 72, 143}}, //0 idle 1
	{Eggman_ArcMain_Idle1, {  0,   0, 128, 146}, { 72, 143}}, //1 idle 2
	{Eggman_ArcMain_Idle2, {  0,   0, 128, 146}, { 72, 143}}, //2 idle 3
	{Eggman_ArcMain_Idle3, {  0,   0, 128, 146}, { 72, 143}}, //3 idle 4	
	{Eggman_ArcMain_Idle4, {  0,   0, 128, 146}, { 72, 143}}, //4 idle 5
	{Eggman_ArcMain_Idle5, {  0,   0, 128, 146}, { 72, 143}}, //5 idle 6
	
	{Eggman_ArcMain_Left0, {  0,   0, 172, 170}, { 133, 167}}, //6 left 1
	{Eggman_ArcMain_Left1, {  0,   0, 172, 170}, { 133, 167}}, //7 left 2
	{Eggman_ArcMain_Left2, {  0,   0, 172, 170}, { 133, 167}}, //8 left 3
	{Eggman_ArcMain_Left3, {  0,   0, 172, 170}, { 133, 167}}, //9 left 4
	
	{Eggman_ArcMain_Down0, {  0,   0, 156, 121}, { 81, 119}}, //10 down 1
	{Eggman_ArcMain_Down1, {  0,   0, 156, 121}, { 81, 119}}, //11 down 2
	{Eggman_ArcMain_Down2, {  0,   0, 156, 121}, { 81, 119}}, //12 down 3
	{Eggman_ArcMain_Down3, {  0,   0, 156, 121}, { 81, 119}}, //13 down 4
	
	{Eggman_ArcMain_Up0, {   0,   0, 196, 205}, { 107, 201}}, //14 up 1
	{Eggman_ArcMain_Up1, {   0,   0, 196, 205}, { 107, 201}}, //15 up 2
	{Eggman_ArcMain_Up2, {   0,   0, 196, 205}, { 107, 201}}, //16 up 3
	{Eggman_ArcMain_Up3, {   0,   0, 196, 205}, { 107, 201}}, //17 up 4
	
	{Eggman_ArcMain_Right0, {  0,   0, 180, 191}, { 107, 188}}, //18 right 1
	{Eggman_ArcMain_Right1, {  0,   0, 180, 191}, { 107, 188}}, //19 right 2
	{Eggman_ArcMain_Right2, {  0,   0, 180, 191}, { 107, 188}}, //20 right 3
	{Eggman_ArcMain_Right3, {  0,   0, 180, 191}, { 107, 188}}, //21 right 4
	
	{Eggman_ArcMain_Laugh0, {  0,   0, 136, 198}, { 75, 194}}, //22 laugh 1
	{Eggman_ArcMain_Laugh1, {  0,   0, 136, 198}, { 75, 194}}, //23 laugh 2
	{Eggman_ArcMain_Laugh2, {  0,   0, 136, 198}, { 75, 194}}, //24 laugh 3
	{Eggman_ArcMain_Laugh3, {  0,   0, 136, 198}, { 75, 194}}, //25 laugh 4	
	{Eggman_ArcMain_Laugh4, {  0,   0, 136, 198}, { 75, 194}}, //26 laugh 5
	{Eggman_ArcMain_Laugh5, {  0,   0, 136, 198}, { 75, 194}}, //27 laugh 6
	{Eggman_ArcMain_Laugh6, {  0,   0, 136, 198}, { 75, 194}}, //28 laugh 7
};

static const Animation char_eggman_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5, ASCR_BACK, 1}},     		   //CharAnim_Idle
	{2, (const u8[]){ 6,  7,  8,  9, ASCR_BACK, 1}},       		   		   //CharAnim_Left
	{2, (const u8[]){25, 26, 27, 28, ASCR_BACK, 1}},                       //CharAnim_LeftAlt
	{2, (const u8[]){10, 11, 12, 13, ASCR_BACK, 1}},  		     		   //CharAnim_Down
	{2, (const u8[]){22, 23, 24, 25, ASCR_BACK, 1}},                       //CharAnim_DownAlt
	{2, (const u8[]){14, 15, 16, 17, ASCR_BACK, 1}},					   //CharAnim_Up
	{2, (const u8[]){25, 26, 27, 28, ASCR_BACK, 1}},                       //CharAnim_UpAlt
	{2, (const u8[]){18, 19, 20, 21, ASCR_BACK, 1}}, 					   //CharAnim_Right
	{2, (const u8[]){22, 23, 24, 25, ASCR_BACK, 1}},                       //CharAnim_RightAlt
};

//Eggman character functions
void Char_Eggman_SetFrame(void *user, u8 frame)
{
	Char_Eggman *this = (Char_Eggman*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_eggman_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Eggman_Tick(Character *character)
{
	Char_Eggman *this = (Char_Eggman*)character;
	
	//Perform idle dance
	Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Eggman_SetFrame);

	if (stage.stage_id == StageId_1_3 && stage.song_step >= 2832 && stage.song_step <= 4111)
	Character_Draw(character, &this->tex, &char_eggman_frame[this->frame]);
}

void Char_Eggman_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Eggman_Free(Character *character)
{
	Char_Eggman *this = (Char_Eggman*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Eggman_New(fixed_t x, fixed_t y)
{
	//Allocate eggman object
	Char_Eggman *this = Mem_Alloc(sizeof(Char_Eggman));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Eggman_New] Failed to allocate eggman object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Eggman_Tick;
	this->character.set_anim = Char_Eggman_SetAnim;
	this->character.free = Char_Eggman_Free;
	
	Animatable_Init(&this->character.animatable, char_eggman_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character stage information
	this->character.health_i = 2;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\EGGMAN.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Eggman_ArcMain_Idle0
		"idle1.tim", //Eggman_ArcMain_Idle1
		"idle2.tim", //Eggman_ArcMain_Idle2
		"idle3.tim", //Eggman_ArcMain_Idle3
		"idle4.tim", //Eggman_ArcMain_Idle4
		"idle5.tim", //Eggman_ArcMain_Idle5
		"left0.tim",  //Eggman_ArcMain_Left0
		"left1.tim",  //Eggman_ArcMain_Left1
		"left2.tim",  //Eggman_ArcMain_Left2
		"left3.tim",  //Eggman_ArcMain_Left3
		"down0.tim",  //Eggman_ArcMain_Down0
		"down1.tim",  //Eggman_ArcMain_Down1
		"down2.tim",  //Eggman_ArcMain_Down2
		"down3.tim",  //Eggman_ArcMain_Down3
		"up0.tim",    //Eggman_ArcMain_Up0
		"up1.tim",    //Eggman_ArcMain_Up1
		"up2.tim",    //Eggman_ArcMain_Up2
		"up3.tim",    //Eggman_ArcMain_Up3
		"right0.tim", //Eggman_ArcMain_Right0
		"right1.tim", //Eggman_ArcMain_Right1
		"right2.tim", //Eggman_ArcMain_Right2
		"right3.tim", //Eggman_ArcMain_Right3
		"laugh0.tim", //Eggman_ArcMain_Laugh0
		"laugh1.tim", //Eggman_ArcMain_Laugh1
		"laugh2.tim", //Eggman_ArcMain_Laugh2
		"laugh3.tim", //Eggman_ArcMain_Laugh3
		"laugh4.tim", //Eggman_ArcMain_Laugh4
		"laugh5.tim", //Eggman_ArcMain_Laugh5
		"laugh6.tim", //Eggman_ArcMain_Laugh6
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
