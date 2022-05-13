/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "week8.h"

#include "../mem.h"
#include "../archive.h"

//Week 8 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	Gfx_Tex tex_back0; //Background
	Gfx_Tex tex_back1; //JUMPSCARE
	Gfx_Tex tex_back2; //Black BG
} Back_Week8;

//Week 8 background functions
void Back_Week8_DrawBG(StageBack *back)
{
	Back_Week8 *this = (Back_Week8*)back;
	
	fixed_t fx, fy;
	fx = stage.camera.x;
	fy = stage.camera.y;

	//Draw background
	RECT back_src = {0, 0, 256, 256};
	RECT_FIXED back_dst = {
		FIXED_DEC(-275,1) - fx,
		FIXED_DEC(-200,1) - fy,
		FIXED_DEC(483,1),
		FIXED_DEC(367,1)
	};
	Stage_DrawTex(&this->tex_back0, &back_src, &back_dst, stage.camera.bzoom);

	//Draw background 3
	RECT backb_src = {0, 0, 256, 256};
	RECT_FIXED backb_dst = {
		FIXED_DEC(-275,1) - fx,
		FIXED_DEC(-200,1) - fy,
		FIXED_DEC(483,1),
		FIXED_DEC(367,1)
	};
	if (stage.stage_id == StageId_1_3 && stage.song_step >= 1040 && stage.song_step <= 1296)
	Stage_DrawTex(&this->tex_back2, &backb_src, &backb_dst, stage.camera.bzoom);
	if (stage.stage_id == StageId_1_3 && stage.song_step >= 2320 && stage.song_step <= 2832)
	Stage_DrawTex(&this->tex_back2, &backb_src, &backb_dst, stage.camera.bzoom);
	if (stage.stage_id == StageId_1_3 && stage.song_step >= 4111 && stage.song_step <= 5250)
	Stage_DrawTex(&this->tex_back2, &backb_src, &backb_dst, stage.camera.bzoom);

	//Draw background 2
	RECT window_src = {0, 0, 256, 256};
	RECT_FIXED window_dst = {
		FIXED_DEC(-275,1) - fx,
		FIXED_DEC(-200,1) - fy,
		FIXED_DEC(483,1),
		FIXED_DEC(367,1)
	};
	Stage_DrawTex(&this->tex_back1, &window_src, &window_dst, stage.camera.bzoom);
}

void Back_Week8_Free(StageBack *back)
{
	Back_Week8 *this = (Back_Week8*)back;
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Week8_New(void)
{
	//Allocate background structure
	Back_Week8 *this = (Back_Week8*)Mem_Alloc(sizeof(Back_Week8));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Week8_DrawBG;
	this->back.free = Back_Week8_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\WEEK8\\BACK.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Mem_Free(arc_back);
	
	return (StageBack*)this;
}
