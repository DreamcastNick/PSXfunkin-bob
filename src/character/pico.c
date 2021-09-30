/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "pico.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Pico character structure
enum
{
	Pico_ArcMain_Idle,
	Pico_ArcMain_Hit0,
	Pico_ArcMain_Hit1,
	
	Pico_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Pico_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Pico;

//Pico character definitions
static const CharFrame char_pico_frame[] = {
	{Pico_ArcMain_Idle, {  0,  22, 106,  94}, { 62, 94}}, //0 idle 1
	{Pico_ArcMain_Idle, {111,  22, 108,  94}, { 62, 94}}, //1 idle 2
	{Pico_ArcMain_Idle, {  0, 146, 105,  94}, { 62, 94}}, //2 idle 3
	{Pico_ArcMain_Idle, {113, 146, 105,  94}, { 62, 94}}, //3 idle 4
	
	{Pico_ArcMain_Hit0, {  0, 124, 100,  95}, { 62, 94}}, //4 left 1
	{Pico_ArcMain_Hit0, {125, 124,  98,  95}, { 62, 94}}, //5 left 2
	
	{Pico_ArcMain_Hit1, {  0, 154,  82,  89}, { 64,  89}}, //6 down 1
	{Pico_ArcMain_Hit1, {114, 154,  82,  89}, { 64,  89}}, //7 down 2
	
	{Pico_ArcMain_Hit1, {  0,  13,  99, 112}, { 64, 112}}, //8 up 1
	{Pico_ArcMain_Hit1, {110,  11, 100, 112}, { 64, 112}}, //9 up 2
	
	{Pico_ArcMain_Hit0, {  0,  19,  77, 101}, { 62, 95}}, //10 right 1
	{Pico_ArcMain_Hit0, {116,  17,  77, 102}, { 62, 95}}, //11 right 2
};

static const Animation char_pico_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  2,  1, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Pico character functions
void Char_Pico_SetFrame(void *user, u8 frame)
{
	Char_Pico *this = (Char_Pico*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_pico_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Pico_Tick(Character *character)
{
	Char_Pico *this = (Char_Pico*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Pico_SetFrame);
	Character_Draw(character, &this->tex, &char_pico_frame[this->frame]);
}

void Char_Pico_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Pico_Free(Character *character)
{
	Char_Pico *this = (Char_Pico*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Pico_New(fixed_t x, fixed_t y)
{
	//Allocate pico object
	Char_Pico *this = Mem_Alloc(sizeof(Char_Pico));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Pico_New] Failed to allocate pico object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Pico_Tick;
	this->character.set_anim = Char_Pico_SetAnim;
	this->character.free = Char_Pico_Free;
	
	Animatable_Init(&this->character.animatable, char_pico_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 3;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\PICO.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim", //Pico_ArcMain_Idle0
		"hit0.tim", //Pico_ArcMain_Hit0
		"hit1.tim", //Pico_ArcMain_Hit1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
