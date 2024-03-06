#include <math.h>
#include "helpers/helpers.h"
#include "3d_layer.h"
#include "gltf_loader.h"
#include "3d_format.h"


// EXPORTED FUNCTIONS
#define UPDATE_TYPE(...) void (*__VA_ARGS__)(App_memory*, Audio_playback*, u32, Int2)
#define RENDER_TYPE(...) void (*__VA_ARGS__)(App_memory*, LIST(Renderer_request,), Int2 )
#define INIT_TYPE(...) void (*__VA_ARGS__)(App_memory*, Init_data* )
#define CLOSE_TYPE(...) void (*__VA_ARGS__)(App_memory*)

// WIN FUNCTIONS
	// file io
#define READ_FILE_FUNCTION_TYPE(...) File_data (*__VA_ARGS__)(String, Memory_arena*)
#define WRITE_FILE_FUNCTION_TYPE(...) bool (*__VA_ARGS__)(String, void*, u32)
#define FILE_EXISTS_FUNCTION_TYPE(...) bool (*__VA_ARGS__)(char*)


struct Element_handle
{
	u32 index;
	u32 generation; // this value updates when the entity is deleted
};

b32 handle_is_valid(Element_handle handle, u32* generations)
{
	// generation at index 0 must be different than 0
	if(!handle.index && !handle.generation || ((s32)handle.index) < 0) return false;
	else return generations[handle.index] == handle.generation;
}

internal b32 
operator ==(Element_handle h1, Element_handle h2){
	return ((h1.index || h1.generation) && (h2.index || h2.generation)) && (h1.index == h2.index && h1.generation == h2.generation);
}
internal b32
operator !=(Element_handle h1, Element_handle h2){
	return (!(h1.index || h1.generation) || !(h2.index || h2.generation)) || (h1.index != h2.index || h1.generation != h2.generation);
}
internal b32 
compare_entity_handles(Element_handle h1, Element_handle h2){
	return h1 == h2;
}

struct Frame_animation
{
	u16 frames_count;
	u16 loop_beginning;	
	u16* frames_tex_uids;
};

struct Frame_animation_group
{
	u32 animations_count;
	Frame_animation* animations;
};

struct Frame_animation_player
{
	u16 animation_index; 
	b8 loop;
	b8 flip_h;

	f32 initial_time;
	f32 duration;


	f32 rotation_angle;
	V2 offset_pos;
	V2 offset_scale;

	//TODO: check if this is true
	// CURRENTLY THIS NEEDS TO BE DONE EVERY FRAME CUZ EVERYTIME I AM ZEROING THE ANIMATION_PLAYERS 
	void set_animation(
		u16 new_animation_index, 
		f32 new_initial_time, 
		f32 new_duration,
		b8 _loop,
		b8 _flip_h
	)
	{
		if(animation_index != new_animation_index || duration != new_duration)
		{
			animation_index = new_animation_index;
			initial_time = new_initial_time;
			duration = new_duration;
			loop = _loop;
		}
		flip_h = _flip_h;
		rotation_angle = 0;
		offset_pos = {0};
		offset_scale = {1,1};
	}
};

// scale must be 1,1,1 by default

// struct Tex_uid{
// 	u32 tex_info_uid;
// 	// b32 is_atlas;
// };

enum Input_keyboard_indices
{
	INPUT_CURSOR_PRIMARY,
	INPUT_CURSOR_SECONDARY,

	INPUT_TAB,
	INPUT_SPACE_BAR,
	INPUT_SHIFT,
	INPUT_CONTROL,
	INPUT_ALT,

	INPUT_Q,
	INPUT_W,
	INPUT_E,
	INPUT_R,
	INPUT_T,
	INPUT_Y,
	INPUT_U,
	INPUT_I,
	INPUT_O,
	INPUT_P,
	INPUT_A,
	INPUT_S,
	INPUT_D,
	INPUT_F,
	INPUT_G,
	INPUT_H,
	INPUT_J,
	INPUT_K,
	INPUT_L,
	INPUT_Z,
	INPUT_X,
	INPUT_C,
	INPUT_V,
	INPUT_B,
	INPUT_N,
	INPUT_M,

	INPUT_1,
	INPUT_2,
	INPUT_3,
	INPUT_4,
	INPUT_5,
	INPUT_6,
	INPUT_7,
	INPUT_8,
	INPUT_9,
	INPUT_0,

	INPUT_F1,
	INPUT_F2,
	INPUT_F3,
	INPUT_F4,
	INPUT_F5,
	INPUT_F6,
	INPUT_F7,
	INPUT_F8,
	INPUT_F9,
	INPUT_F10,
	INPUT_F11,
	INPUT_F12,

	INPUT_BACKSPACE,
	INPUT_RETURN,
	INPUT_INSERT,
	INPUT_DEL,	
	INPUT_HOME,
	INPUT_END,
	INPUT_PGUP,
	INPUT_PGDOWN,
	
	INPUT_DOWN,
	INPUT_UP,
	INPUT_LEFT,
	INPUT_RIGHT,


	INPUT_COUNT,
};


struct User_input
{
	V2 cursor_pos;
	V2 cursor_speed;

	Int2 cursor_pixels_pos;
	s32 delta_wheel;

	s32 keys[INPUT_COUNT];
};

internal void
set_input(s32* holding_keys, u8* pressed_inputs, Input_keyboard_indices k, b32 is_pressed)
{
	holding_keys[k] = is_pressed;
	if(is_pressed)
	{
		pressed_inputs[k] = 1;
	}
}

struct FILE_IO_FUNCTIONS
{
	READ_FILE_FUNCTION_TYPE(read_file);
	WRITE_FILE_FUNCTION_TYPE(write_file);
	FILE_EXISTS_FUNCTION_TYPE(file_exists);
};

struct Date
{
	u16 year;
	u16 month;
	u16 monthday;
	u16 weekday;
};

struct Datetime
{
	union
	{
		struct{
			u16 year;
			u16 month;
			u16 monthday;
			u16 weekday;
		};
		Date date;
	};
	u16 hour;
	u16 min;
	u16 sec;
	u16 ms;
};

internal b32 
compare_dates(Date d1, Date d2)
{
	return 
		d1.year == d2.year &&
		d1.month == d2.month &&
		d1.monthday == d2.monthday
	;	
}

internal b32 operator==(Date d1, Date d2)
{
	return compare_dates(d1, d2);
}

#define GET_CURRENT_DATE_FUNCTION_TYPE(...) Datetime (*__VA_ARGS__)()
#define OFFSET_DATE_BY_DAYS_FUNCTION_TYPE(...) Datetime (*__VA_ARGS__)(Datetime*, s32)

struct WIN_TIME_FUNCTIONS
{
	GET_CURRENT_DATE_FUNCTION_TYPE(get_current_date);
	OFFSET_DATE_BY_DAYS_FUNCTION_TYPE(offset_date_by_days);
};

struct App_memory
{
	FILE_IO_FUNCTIONS file_io;
	WIN_TIME_FUNCTIONS win_time;

	u64 win_time_ns;

	b32 close_app;

	b32 is_initialized;
	b32 renderer_needs_resizing; // this is just for when i want to change the rendering resolution

	Memory_arena* permanent_arena;
	Memory_arena* temp_arena;

	f32 keyboard_repeat_delay;
	f32 keyboard_repeat_cooldown;

	RNG rng;

	Color bg_color;

	Int2 screen_size; //THIS IS THE SIZE OF THE SCREEN AS THE NAME SAYS YOU DUMB FU**ER
	f32 fov;
	b32 enforce_aspect_ratio;
	f32 aspect_ratio;
	f32 depth_effect; // this is redundant with the fov

	LIST(Tex_info, tex_infos);
	LIST(Font, fonts_list);

	Camera camera;
	
	struct
	{// this is meant to be passed to the shader 
		b32 perspective_on;
		f32 time_s;
		f32 depth_writing;
	};

	User_input* input;
	User_input* holding_inputs;
	u8 input_chars_buffer[64];
	u32 input_chars_buffer_current_size;


	b32 is_window_in_focus;
	b32 lock_mouse;

	f32 update_hz;

	f32 old_time_s;
	f32 fixed_dt;
		
	void* app;
};



internal u16
get_next_available_index(u8* array, u32 arraylen, u16* last_used_index)
{
	u16 last_index = ((*last_used_index))%arraylen;

	ASSERT(arraylen < 0xffff);
	UNTIL(i, arraylen)
	{
		u16 index = (last_index+i)%(u16)arraylen;
		if(!array[index])
		{
			array[index] = 1;
			*last_used_index = index;
			return index;
		}
	}
	ASSERT(false);
	return false;
}


// this MUST match the register buffer indices in the shader code
enum Renderer_variable_register_index : u16
{
	// VERTEX SHADER REGISTER INDICES
	OBJECT_DATA_REGISTER_INDEX = 0,
	PROJECTION_VIEW_REGISTER_INDEX,
	CAMERA_POS_REGISTER_INDEX,
	BONE_TRANSFORMS_REGISTER_INDEX,
	

	// PIXEL SHADER REGISTER_INDICES
	SCREEN_DATA_REGISTER_INDEX = 14,
	TIME_REGISTER_INDEX,
};

//@@bitwise_enum@@
enum RENDERER_REQUEST_TYPE_FLAGS
{
	REQUEST_FLAG_RENDER_OBJECT = 0b1,
	REQUEST_FLAG_RENDER_INSTANCES = 0b10,
	REQUEST_FLAG_POSTPROCESSING = 0b100,

	REQUEST_FLAG_CHANGE_VIEWPORT_SIZE = 0b1000,
	REQUEST_FLAG_MODIFY_RENDERER_VARIABLE = 0b10000,
	REQUEST_FLAG_RESIZE_DEPTH_STENCIL_VIEW = 0b100000,
	REQUEST_FLAG_MODIFY_DYNAMIC_MESH = 0b1000000,
	REQUEST_FLAG_SET_VS = 0b10000000,
	REQUEST_FLAG_SET_PS = 0b100000000,
	REQUEST_FLAG_RESIZE_TARGET_VIEW = 0b1000000000,
	REQUEST_FLAG_SET_BLEND_STATE = 0b10000000000,
	REQUEST_FLAG_SET_RENDER_TARGET_AND_DEPTH_STENCIL = 0b100000000000,
	REQUEST_FLAG_SET_SHADER_RESOURCE_FROM_RENDER_TARGET = 0b1000000000000,
	REQUEST_FLAG_SET_RASTERIZER_STATE = 0b10000000000000,
};

struct Renderer_request{
	u32 type_flags;
	
	union
	{
		Object3d object3d;
		struct{
			OBJECT3D_STRUCTURE
			b8 flip_h;
		};

		struct {
			u16 mesh_uid;
			u16 instances_count;
			Instance_data* instances;
			u16 dynamic_instances_mesh;
			b8 flip_h;
		}instancing_data;
	};
	struct
	{
		struct
		{
			u16 uid;
			u16 size;
			void* new_data;
		}renderer_variable;
		
		struct
		{
			u16 mesh_uid;
			Vertex* vertices;
			u16 vertex_count;
			u16* indices;
			u16 indices_count;
		}modified_mesh;

		struct{
			u16 fill_mode;
			u16 cull_mode;
		}rasterizer_state;

		Int2 new_viewport_size;

		u16 vshader_uid;
		u16 pshader_uid;
		u16 blend_state_uid;
		u16 resize_depth_stencil_view_uid;
		u16 resize_rtv_uid;
		u16 set_depth_stencil_uid;	

		struct 
		{
			u16 target_index;
			u16 render_target_uid;
		}set_shader_resource;
		

		u16* set_rtv_uids;
		u16 set_rtv_count;
	};
};

struct Sound_playback_request
{
	u16 sound_uid;
};


// REQUESTS STRUCTS AND FUNCTIONS BOILERPLATE
// TODO: UNIFY ALL THIS FUNCTIONS AND STRUCTS TO BE JUST ONE THING
enum Asset_request_type{
	FORGOR_TO_SET_ASSET_TYPE = 0,
	TEX_FROM_FILE_REQUEST,
	FONT_FROM_FILE_REQUEST,
	VERTEX_SHADER_FROM_FILE_REQUEST,
	PIXEL_SHADER_FROM_FILE_REQUEST,
	MESH_FROM_FILE_REQUEST,
	CREATE_BLEND_STATE_REQUEST,
	CREATE_DEPTH_STENCIL_REQUEST,
	CREATE_RENDER_TARGET_REQUEST,
	SOUND_FROM_FILE_REQUEST,

	TEX_FROM_SURFACE_REQUEST,
	MESH_FROM_PRIMITIVES_REQUEST,
	CREATE_CONSTANT_BUFFER_REQUEST,
	CREATE_DYNAMIC_MESH,
};
enum IE_CLASS
{
	IE_CLASS_PER_VERTEX,
	IE_CLASS_PER_INDEX,
};

struct Input_element_desc
{
	ARRAY(char*, names);
	ARRAY(IE_FORMATS, formats);
	u32 next_slot_beginning_index; // this is for instancing
};

struct Asset_request{
	//TODO: clearly create each struct for each type of request 
	// cuz right now i can't tell which properties each request uses
	Asset_request_type type;
	union{
		u16* p_uid;
		u32 sound_uid;
		u16* font_uid;
	};
	union{
		struct { // this is for vertex shader
			String filename;
			Input_element_desc ied; // input_element_desc
			f32 font_lines_height;
		};
		struct{
			Renderer_variable_register_index register_index;
			u16 size;
		}constant_buffer;

		Mesh_primitive mesh_primitives; 
		
		Surface tex_surface;
		
		//TODO: THIS ARE NOT ASSETS BUT I DON'T KNOW WHERE TO PUT'EM
		b32 enable_alpha_blending;

		b32 enable_depth;
	};
};

struct Init_data
{
	// this is sent from the platform layer to the init function
	File_data meshes_serialization;
	File_data textures_serialization;
	File_data sounds_serialization;
 
	// this is the result from the init function to the platform layer
	LIST(Asset_request, asset_requests);
};


struct String_index_pair{
	String str;
	union{
		u16** index_pp;
		u16* index_p;
	};
};

enum Asset_serialization_parse_state
{
	ASPS_NO_STATE,
	ASPS_READING_IDENTIFIERS,
	ASPS_READING_WORD,
	ASPS_READING_FILENAME,
};

internal void
parse_assets_serialization_file(
	App_memory* memory, File_data file,
	ARRAY(String_index_pair, string_index_pairs),
	LIST(String_index_pair, result_pairs)
)
{
	Asset_serialization_parse_state state = ASPS_NO_STATE;

	b32 already_saved_uid = 0;
	u16* current_asset_index = 0;
	String temp_string = {0};

	UNTIL(i, file.size)
	{
		char current_char = file.text[i];
		if(state == ASPS_READING_FILENAME)
		{
			
			if(current_char == ' ' || current_char == '\r' || current_char == '\n' || !current_char || i == file.size-1)
			{
				state = ASPS_NO_STATE;
				if(i==file.size-1)
				{
					temp_string.length++;
				}
				String_index_pair* new_pair;

				PUSH_BACK(result_pairs, memory->temp_arena, new_pair);
				new_pair->str = temp_string;
				new_pair->index_p = current_asset_index;

				already_saved_uid = 0;
				temp_string = {0};
				current_asset_index = 0;
				
			}
			else
			{
				temp_string.length++;
			}
		}
		if(is_alphanumeric(current_char))
		{
			if(state == ASPS_NO_STATE)
			{
				if(already_saved_uid)
				{
					state = ASPS_READING_FILENAME;
					temp_string.text = file.text+i;
					temp_string.length++;
				}
			}
			if(state == ASPS_READING_IDENTIFIERS)
			{
				state = ASPS_READING_WORD;
				temp_string.text = file.text+i;
			}
			if(state == ASPS_READING_WORD)
			{
				temp_string.length++;
			}
		}
		else
		{
			if(state == ASPS_NO_STATE)
			{
				if(current_char == '[')
				{
					state = ASPS_READING_IDENTIFIERS;
				}
			}
			else if(state == ASPS_READING_WORD)
			{
				UNTIL(pair_index, ARRAYLEN(string_index_pairs))
				{
					if(compare_strings(temp_string, string_index_pairs[pair_index].str))
					{
						if(!already_saved_uid)
						{
							already_saved_uid = 1;
							// SAVE AN INDEX FOR A NEW ASSET
							current_asset_index = ARENA_PUSH_STRUCT(memory->permanent_arena, u16);
						}
						*string_index_pairs[pair_index].index_pp = current_asset_index;
					}
				}
				temp_string = {0};
				if(current_char == ',')
				{
					state = ASPS_READING_IDENTIFIERS;
				}
				else if(current_char == ']')
				{
					state = ASPS_NO_STATE;
				}
			}
		}
	}
}

internal void
push_sound(Audio_playback* playback_list, u32 sound_uid, u32 sample_t)
{
	Audio_playback* playback_request = find_next_available_playback(playback_list);
	playback_request->sound_uid = sound_uid;
	playback_request->initial_sample_t = sample_t;
}


internal void
push_asset_sound_request(Asset_request* request, Memory_arena* arena, String filename, u32 sound_uid)
{
	request->type = SOUND_FROM_FILE_REQUEST;
	request->filename.length  = filename.length;
	request->filename.text = (char*)arena_push_data(arena, filename.text, filename.length);

	request->sound_uid = sound_uid;
}

// OLD PRINTO_SCREEN FUNCTION
#if 0
internal void
printo_screen(App_memory* memory,Int2 screen_size, LIST(Renderer_request,render_list),String text, V2 pos, Color color){
	f32 line_height = 18;
	Renderer_request* request = 0;
	f32 xpos = pos.x;
	UNTIL(c, text.length){
		char current_char = text.text[c];
		char char_index = CHAR_TO_INDEX(current_char);

		if(current_char == ' ')
			xpos += 16.0f/screen_size.x;
		else{			
			PUSH_BACK(render_list, memory->temp_arena, request);
			request->type_flags = REQUEST_FLAG_RENDER_IMAGE_TO_SCREEN;
			Object3d* object = &request->object3d;
			object->mesh_uid = memory->meshes.plane_mesh_uid;
			object->texinfo_uid = memory->font_tex_infos_uids[char_index];

			Tex_info* tex_info; LIST_GET(memory->tex_infos, object->texinfo_uid, tex_info);

			V2 normalized_scale = normalize_texture_size(screen_size, {tex_info->w, tex_info->h});
			object->scale = {normalized_scale.x, normalized_scale.y, 1};
			
			object->pos.x = xpos+((f32)(tex_info->xoffset)/screen_size.x);
			object->pos.y = pos.y-((2.0f*(line_height+tex_info->yoffset))/screen_size.y);

			object->rotation = {0,0,0};
			object->color = color;

			request->object3d.texinfo_uid = memory->font_tex_infos_uids[char_index];
			xpos += 16.0f / screen_size.x;
		}
	}
}
#endif

#define PUSH_BACK_RENDER_REQUEST(render_list) \
   ASSERT(!request || request->type_flags != REQUEST_FLAG_RENDER_OBJECT || (request->color.a && request->scale.x && request->scale.y && request->scale.z));\
   PUSH_BACK(render_list, memory->temp_arena, request)
