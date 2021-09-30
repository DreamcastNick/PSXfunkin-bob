/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "happybob.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"
#include "../timer.h"

//HappyBob character structure
enum
{
	HappyBob_ArcMain_Idle0,
	HappyBob_ArcMain_Idle1,
	HappyBob_ArcMain_Idle2,
	HappyBob_ArcMain_Idle3,
	
	HappyBob_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[HappyBob_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Hair texture
	Gfx_Tex tex_hair;
} Char_HappyBob;

//HappyBob character definitions
static const CharFrame char_happybob_frame[] = {
	{HappyBob_ArcMain_Idle0, {  6,   4,  74, 116}, { 42, 85}}, //0 idle 1
	{HappyBob_ArcMain_Idle0, { 92,   4,  78, 116}, { 46, 85}}, //1 idle 2
	{HappyBob_ArcMain_Idle0, {177,   4,  78, 116}, { 46, 85}}, //2 idle 3
	{HappyBob_ArcMain_Idle0, {  5, 132,  75, 118}, { 43, 85}}, //3 idle 4
	{HappyBob_ArcMain_Idle0, { 94, 132,  75, 118}, { 43, 85}}, //4 idle 5
	{HappyBob_ArcMain_Idle0, {179, 135,  75, 116}, { 43, 85}}, //5 idle 6
	{HappyBob_ArcMain_Idle1, {  6,   4,  75, 116}, { 43, 85}}, //6 idle 7
	{HappyBob_ArcMain_Idle1, { 92,   4,  75, 116}, { 43, 85}}, //7 idle 8
	{HappyBob_ArcMain_Idle1, {179,   4,  75, 116}, { 43, 85}}, //8 idle 9
	{HappyBob_ArcMain_Idle1, {  5, 133,  75, 116}, { 43, 85}}, //9 idle 10
	{HappyBob_ArcMain_Idle1, { 94, 134,  75, 116}, { 43, 85}}, //10 idle 11
	{HappyBob_ArcMain_Idle1, {178, 135,  75, 116}, { 43, 85}}, //11 idle 12
	{HappyBob_ArcMain_Idle2, {  6,   4,  75, 116}, { 43, 85}}, //12 idle 13
	{HappyBob_ArcMain_Idle2, { 92,   4,  75, 116}, { 43, 85}}, //13 idle 14
	{HappyBob_ArcMain_Idle2, {179,   4,  75, 116}, { 43, 85}}, //14 idle 15
	{HappyBob_ArcMain_Idle2, {  5, 133,  75, 116}, { 43, 85}}, //15 idle 16
	{HappyBob_ArcMain_Idle2, { 94, 134,  75, 116}, { 43, 85}}, //16 idle 17
	{HappyBob_ArcMain_Idle3, {178, 135,  75, 116}, { 43, 85}}, //17 idle 18
	{HappyBob_ArcMain_Idle3, {  6,   4,  75, 116}, { 43, 85}}, //18 idle 19
	{HappyBob_ArcMain_Idle3, { 92,   4,  75, 116}, { 43, 85}}, //19 idle 20
	{HappyBob_ArcMain_Idle3, {179,   4,  75, 116}, { 43, 85}}, //20 idle 21
	{HappyBob_ArcMain_Idle3, {  5, 133,  75, 116}, { 43, 85}}, //21 idle 22
	{HappyBob_ArcMain_Idle3, { 94, 134,  75, 116}, { 43, 85}}, //22 idle 23
	{HappyBob_ArcMain_Idle3, {178, 135,  75, 116}, { 43, 85}}, //23 idle 24
};

static const Animation char_happybob_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23, ASCR_REPEAT, 1}}, //CharAnim_Idle
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_LeftAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_UpAlt
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},        //CharAnim_RightAlt
};

//HappyBob character functions
void Char_HappyBob_SetFrame(void *user, u8 frame)
{
	Char_HappyBob *this = (Char_HappyBob*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_happybob_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_HappyBob_Tick(Character *character)
{
	Char_HappyBob *this = (Char_HappyBob*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_HappyBob_SetFrame);
	Character_Draw(character, &this->tex, &char_happybob_frame[this->frame]);
}

void Char_HappyBob_SetAnim(Character *character, u8 anim)
{
	if (anim != CharAnim_Idle)
    	return;
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_HappyBob_Free(Character *character)
{
	Char_HappyBob *this = (Char_HappyBob*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_HappyBob_New(fixed_t x, fixed_t y)
{
	//Allocate happybob object
	Char_HappyBob *this = Mem_Alloc(sizeof(Char_HappyBob));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_HappyBob_New] Failed to allocate happybob object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_HappyBob_Tick;
	this->character.set_anim = Char_HappyBob_SetAnim;
	this->character.free = Char_HappyBob_Free;
	
	Animatable_Init(&this->character.animatable, char_happybob_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 4;
	
	this->character.focus_x = FIXED_DEC(50,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\HAPPYBOB.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //HappyBob_ArcMain_Idle0
		"idle1.tim", //HappyBob_ArcMain_Idle1
		"idle2.tim", //HappyBob_ArcMain_Idle2
		"idle3.tim", //HappyBob_ArcMain_Idle3
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
