/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfalt.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

int secret2;

//Boyfriend skull fragments
static SkullFragment char_bfalt_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend player types
enum
{
	BFAlt_ArcMain_BFAlt0,
	BFAlt_ArcMain_BFAlt1,
	BFAlt_ArcMain_BFAlt2,
	BFAlt_ArcMain_BFAlt3,
	BFAlt_ArcMain_BFAlt4,
	BFAlt_ArcMain_BFAlt5,
	BFAlt_ArcMain_BFAlt6,
	
	BFAlt_ArcMain_Max,
};

enum
{
	BFAlt_ArcDead_Dead1, //Mic Drop
	BFAlt_ArcDead_Dead2, //Twitch
	BFAlt_ArcDead_Retry, //Retry prompt
	
	BFAlt_ArcDead_Max,
};

#define BFAlt_Arc_Max BFAlt_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BFAlt_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bfalt_skull)];
	u8 skull_scale;
} Char_BFAlt;

//Boyfriend player definitions
static const CharFrame char_bfalt_frame[] = {
	{BFAlt_ArcMain_BFAlt0, {  5,   3, 110,  93}, { 53,  92}}, //0 idle 1
	{BFAlt_ArcMain_BFAlt0, {121,   3, 110,  93}, { 53,  92}}, //1 idle 2
	{BFAlt_ArcMain_BFAlt0, {  5, 102, 110,  93}, { 53,  92}}, //2 idle 3
	{BFAlt_ArcMain_BFAlt0, {121, 102, 110,  93}, { 53,  92}}, //3 idle 4
	
	{BFAlt_ArcMain_BFAlt1, { 10, 100,  95,  91}, { 34,  89}}, //4 left 1
	{BFAlt_ArcMain_BFAlt1, {111, 100,  95,  91}, { 34,  89}}, //5 left 2
	
	{BFAlt_ArcMain_BFAlt1, { 10,   3,  95,  91}, { 47,  89}}, //6 down 1
	{BFAlt_ArcMain_BFAlt1, {111,   3,  95,  91}, { 47,  89}}, //7 down 2
	
	{BFAlt_ArcMain_BFAlt2, {  7, 113, 108, 104}, { 57, 105}}, //8 up 1
	{BFAlt_ArcMain_BFAlt2, {121, 113, 108, 104}, { 57, 105}}, //9 up 2
	
	{BFAlt_ArcMain_BFAlt2, {  7,   3, 108, 104}, { 61, 106}}, //10 right 1
	{BFAlt_ArcMain_BFAlt2, {121,   3, 108, 104}, { 61, 106}}, //11 right 2
	
	{BFAlt_ArcMain_BFAlt3, {  9, 116,  96, 107}, { 38, 110}}, //12 left miss 1
	{BFAlt_ArcMain_BFAlt3, {111, 116,  96, 107}, { 38, 110}}, //13 left miss 2
	
	{BFAlt_ArcMain_BFAlt3, {  9,   3,  96, 107}, { 44, 109}}, //14 down miss 1
	{BFAlt_ArcMain_BFAlt3, {111,   3,  96, 107}, { 44, 109}}, //15 down miss 2
	
	{BFAlt_ArcMain_BFAlt4, {  6, 127, 103, 118}, { 61, 123}}, //16 up miss 1
	{BFAlt_ArcMain_BFAlt4, {115, 127, 103, 118}, { 61, 123}}, //17 up miss 2
	
	{BFAlt_ArcMain_BFAlt4, {  6,   3, 103, 118}, { 54, 123}}, //18 right miss 1
	{BFAlt_ArcMain_BFAlt4, {115,   3, 103, 118}, { 54, 123}}, //19 right miss 2
};

static const Animation char_bfalt_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){ 4, 12, 12, 13, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 6, 14, 14, 15, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 8, 16, 16, 17, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){10, 18, 18, 19, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{2, (const u8[]){ 0,  1,  2,  3,  3, ASCR_BACK, 1}}, //PlayerAnim_Peace
	{2, (const u8[]){ 0,  1,  2,  3,  3, ASCR_BACK, 1}}, //PlayerAnim_Sweat
	
	{5, (const u8[]){ 0,  1,  2,  3,  3, ASCR_CHGANI, PlayerAnim_Dead1}}, 					  //PlayerAnim_Dead0
	{5, (const u8[]){ 0,  1,  2,  3,  3, ASCR_REPEAT}},                                       //PlayerAnim_Dead1
	{3, (const u8[]){ 0,  1,  2,  3,  3, ASCR_CHGANI, PlayerAnim_Dead3}}, 					  //PlayerAnim_Dead2
	{3, (const u8[]){ 0,  1,  2,  3,  3, ASCR_REPEAT}},                                       //PlayerAnim_Dead3
	{3, (const u8[]){ 0,  1,  2,  3,  3, ASCR_CHGANI, PlayerAnim_Dead3}},		              //PlayerAnim_Dead4
	{3, (const u8[]){ 0,  1,  2,  3,  3, ASCR_CHGANI, PlayerAnim_Dead3}},		              //PlayerAnim_Dead5
	
	{10, (const u8[]){ 0,  1,  2,  3,  3, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){ 0,  1,  2,  3,  3, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend Sizer
void BFAlt_Draw(Character *this, Gfx_Tex *tex, const CharFrame *cframe)
{
	//Draw character
	fixed_t x = this->x - stage.camera.x - FIXED_DEC(cframe->off[0],1);
	fixed_t y = this->y - stage.camera.y - FIXED_DEC(cframe->off[1],1);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {x, y, (src.w + 30) << FIXED_SHIFT, (src.h + 30) << FIXED_SHIFT};
	Stage_DrawTex(tex, &src, &dst, stage.camera.bzoom);
}

//Boyfriend player functions
void Char_BFAlt_SetFrame(void *user, u8 frame)
{
	Char_BFAlt *this = (Char_BFAlt*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfalt_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFAlt_Tick(Character *character)
{
	Char_BFAlt *this = (Char_BFAlt*)character;
	
	//Secret icon
	if (secret2)
		this->character.health_i = 9;
	else
		this->character.health_i = 0;

	if (pad_state.press & PAD_SELECT)
		secret2 ++;
	
	if (secret2 == 2)
		secret2 = 0;
	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFAlt, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(56,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_BFAlt_SetFrame);
	BFAlt_Draw(character, &this->tex, &char_bfalt_frame[this->frame]);
}

void Char_BFAlt_SetAnim(Character *character, u8 anim)
{
	Char_BFAlt *this = (Char_BFAlt*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;
			
			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //BFAlt_ArcDead_Dead1
				"dead2.tim", //BFAlt_ArcDead_Dead2
				"retry.tim", //BFAlt_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BFAlt_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BFAlt_Free(Character *character)
{
	Char_BFAlt *this = (Char_BFAlt*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_BFAlt_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BFAlt *this = Mem_Alloc(sizeof(Char_BFAlt));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFAlt_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFAlt_Tick;
	this->character.set_anim = Char_BFAlt_SetAnim;
	this->character.free = Char_BFAlt_Free;
	
	Animatable_Init(&this->character.animatable, char_bfalt_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFALT.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"bfalt0.tim",   //BFAlt_ArcMain_BFAlt0
		"bfalt1.tim",   //BFAlt_ArcMain_BFAlt1
		"bfalt2.tim",   //BFAlt_ArcMain_BFAlt2
		"bfalt3.tim",   //BFAlt_ArcMain_BFAlt3
		"bfalt4.tim",   //BFAlt_ArcMain_BFAlt4
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_bfalt_skull, sizeof(char_bfalt_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BFAlt, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
