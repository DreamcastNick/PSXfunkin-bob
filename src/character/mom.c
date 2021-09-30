/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "mom.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"
#include "../timer.h"

//Mom character structure
enum
{
	Mom_ArcMain_Idle0,
	Mom_ArcMain_Idle1,
	Mom_ArcMain_Idle2,
	
	Mom_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Mom_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Hair texture
	Gfx_Tex tex_hair;
} Char_Mom;

//Mom character definitions
static const CharFrame char_mom_frame[] = {
	{Mom_ArcMain_Idle0, {  6,   4,  74, 116}, { 42, 85}}, //0 idle 1
	{Mom_ArcMain_Idle0, { 92,   4,  78, 116}, { 46, 85}}, //1 idle 2
	{Mom_ArcMain_Idle0, {177,   4,  78, 116}, { 46, 85}}, //2 idle 3
	{Mom_ArcMain_Idle0, {  5, 132,  75, 118}, { 43, 85}}, //3 idle 4
	{Mom_ArcMain_Idle0, { 94, 132,  75, 118}, { 43, 85}}, //4 idle 5
	{Mom_ArcMain_Idle0, {179, 135,  75, 116}, { 43, 85}}, //5 idle 6
	{Mom_ArcMain_Idle1, {  6,   4,  75, 116}, { 43, 85}}, //6 idle 7
	{Mom_ArcMain_Idle1, { 92,   4,  75, 116}, { 43, 85}}, //7 idle 8
	{Mom_ArcMain_Idle1, {179,   4,  75, 116}, { 43, 85}}, //8 idle 9
	{Mom_ArcMain_Idle1, {  5, 133,  75, 116}, { 43, 85}}, //9 idle 10
	{Mom_ArcMain_Idle1, { 94, 134,  75, 116}, { 43, 85}}, //10 idle 11
	{Mom_ArcMain_Idle1, {178, 135,  75, 116}, { 43, 85}}, //11 idle 12
	{Mom_ArcMain_Idle2, {  6,   4,  75, 116}, { 43, 85}}, //12 idle 13
	{Mom_ArcMain_Idle2, { 92,   4,  75, 116}, { 43, 85}}, //13 idle 14
	{Mom_ArcMain_Idle2, {179,   4,  75, 116}, { 43, 85}}, //14 idle 15
	{Mom_ArcMain_Idle2, {  5, 133,  75, 116}, { 43, 85}}, //15 idle 16
	{Mom_ArcMain_Idle2, { 94, 134,  75, 116}, { 43, 85}}, //16 idle 17
	{Mom_ArcMain_Idle2, {178, 135,  75, 116}, { 43, 85}}, //17 idle 18
};

static const Animation char_mom_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,  11,  12,  13,  14,  15,  16,  17, ASCR_REPEAT, 1}}, //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_LeftAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_UpAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_RightAlt
};

//Mom character functions
void Char_Mom_SetFrame(void *user, u8 frame)
{
	Char_Mom *this = (Char_Mom*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_mom_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Mom_Tick(Character *character)
{
	Char_Mom *this = (Char_Mom*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Mom_SetFrame);
	Character_Draw(character, &this->tex, &char_mom_frame[this->frame]);
}

void Char_Mom_SetAnim(Character *character, u8 anim)
{
	if (anim != CharAnim_Idle)
    	return;
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Mom_Free(Character *character)
{
	Char_Mom *this = (Char_Mom*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Mom_New(fixed_t x, fixed_t y)
{
	//Allocate mom object
	Char_Mom *this = Mem_Alloc(sizeof(Char_Mom));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Mom_New] Failed to allocate mom object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Mom_Tick;
	this->character.set_anim = Char_Mom_SetAnim;
	this->character.free = Char_Mom_Free;
	
	Animatable_Init(&this->character.animatable, char_mom_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 4;
	
	this->character.focus_x = FIXED_DEC(50,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\MOM.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Mom_ArcMain_Idle0
		"idle1.tim", //Mom_ArcMain_Idle1
		"idle2.tim", //Mom_ArcMain_Idle2
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
