/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bobt.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//BobT character structure
enum
{
	BobT_ArcMain_Idle0,
	BobT_ArcMain_Idle1,
	BobT_ArcMain_Idle2,
	BobT_ArcMain_Left,
	BobT_ArcMain_Down,
	BobT_ArcMain_Up,
	BobT_ArcMain_Right,
	
	BobT_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[BobT_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_BobT;

//BobT character definitions
static const CharFrame char_bobt_frame[] = {
	{BobT_ArcMain_Idle0, {  0,  57, 107,  84}, { 62, 61}}, //0 idle 1
	{BobT_ArcMain_Idle0, {118,  55, 108,  86}, { 62, 61}}, //1 idle 2
	{BobT_ArcMain_Idle1, {  1,  56, 107,  85}, { 62, 61}}, //2 idle 3
	{BobT_ArcMain_Idle1, {120,  57, 108,  84}, { 62, 61}}, //3 idle 4
	{BobT_ArcMain_Idle2, {  1,  57, 108,  84}, { 62, 61}}, //4 idle 5
	{BobT_ArcMain_Idle2, {120,  55, 106,  86}, { 62, 61}}, //5 idle 6
	
	{BobT_ArcMain_Left, {  1,  57,  92,  84}, {62, 61}}, //6 left 1
	{BobT_ArcMain_Left, {119,  57,  93,  82}, {62, 61}}, //7 left 2
	
	{BobT_ArcMain_Down, {  2,  57,  90,  84}, { 62,  61}}, //8 down 1
	{BobT_ArcMain_Down, {115,  58,  89,  83}, { 62,  61}}, //9 down 2
	
	{BobT_ArcMain_Up, {  1,  49, 101,  91}, { 62, 61}}, //10 up 1
	{BobT_ArcMain_Up, {117,  48, 104,  92}, { 62, 61}}, //11 up 2
	
	{BobT_ArcMain_Right, {  1,  57, 104,  83}, { 62, 61}}, //12 right 1
	{BobT_ArcMain_Right, {115,  56, 107,  84}, { 62, 61}}, //13 right 2
};

static const Animation char_bobt_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4,  5,  4,  3,  2,  1, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},          //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},    //CharAnim_LeftAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},          //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},    //CharAnim_DownAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},          //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},    //CharAnim_UpAlt
	{2, (const u8[]){12, 13, ASCR_BACK, 1}},          //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},    //CharAnim_RightAlt
};

//BobT character functions
void Char_BobT_SetFrame(void *user, u8 frame)
{
	Char_BobT *this = (Char_BobT*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bobt_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BobT_Tick(Character *character)
{
	Char_BobT *this = (Char_BobT*)character;
	
	//Perform bobty dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
	{
		Character_CheckEndSing(character);
		
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			if ((Animatable_Ended(&character->animatable) || character->animatable.anim == CharAnim_LeftAlt || character->animatable.anim == CharAnim_RightAlt) &&
				(character->animatable.anim != CharAnim_Left &&
				 character->animatable.anim != CharAnim_Down &&
				 character->animatable.anim != CharAnim_Up &&
				 character->animatable.anim != CharAnim_Right) &&
				(stage.song_step & 0x3) == 0)
				character->set_anim(character, CharAnim_Idle);
		}
	}
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_BobT_SetFrame);
	Character_Draw(character, &this->tex, &char_bobt_frame[this->frame]);
}

void Char_BobT_SetAnim(Character *character, u8 anim)
{
	//Set animation
	if (anim == CharAnim_Idle)
	{
		if (character->animatable.anim == CharAnim_LeftAlt)
			anim = CharAnim_RightAlt;
		else
			anim = CharAnim_LeftAlt;
		character->sing_end = FIXED_DEC(0x7FFF,1);
	}
	else
	{
		Character_CheckStartSing(character);
	}
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_BobT_Free(Character *character)
{
	Char_BobT *this = (Char_BobT*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BobT_New(fixed_t x, fixed_t y)
{
	//Allocate bobt object
	Char_BobT *this = Mem_Alloc(sizeof(Char_BobT));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BobT_New] Failed to allocate bobt object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BobT_Tick;
	this->character.set_anim = Char_BobT_SetAnim;
	this->character.free = Char_BobT_Free;
	
	Animatable_Init(&this->character.animatable, char_bobt_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 8;
	
	this->character.focus_x =  FIXED_DEC(50,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BOBT.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //BobT_ArcMain_Idle0
		"idle1.tim", //BobT_ArcMain_Idle1
		"idle2.tim", //BobT_ArcMain_Idle2
		"left.tim",  //BobT_ArcMain_Left
		"down.tim",  //BobT_ArcMain_Down
		"up.tim",    //BobT_ArcMain_Up
		"right.tim", //BobT_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
