#include "shader_header.h"

VS_OUTPUT_DEFAULT vs(VS_INPUT_DEFAULT input){
	VS_OUTPUT_DEFAULT result;

	result.pixel_pos = mul(object_transform , float4(input.vertex_pos, 1));
	result.color = object_color;
	result.texcoord.x = object_texrect.x + (input.texcoord.x * object_texrect.z);
	result.texcoord.y = object_texrect.y + (input.texcoord.y * object_texrect.w);

	
	result.normal = 0;
	result.vertex_world_pos = 0;
	result.camera_world_pos = 0;

	return result;
}