#include "definitions.h"


internal void
concat_char_strings(char* s1, char* s2, char* output)
{
	while(*s1)
	{
		*output = *s1;
		output++;
		s1++;
	}
	while(*s2)
	{
		*output = *s2;
		output++;
		s2++;
	}
}

internal b8
is_alphanumeric(u8 code)
{
   return 
        ('a' <= code && code <= 'z' )    ||
        ('A' <= code && code <= 'Z')    ||
        ('0' <= code && code <= '9')      ||
        ('_' == code)
        ;
}

struct String
{
	char* text;
	u32 length;
};

internal String 
string(char* text)
{
	String result = {0};
	result.text = text;
	while(*text)
	{
		result.length++;
		text++;
	}
	return result;
}

internal b32 
compare_chars(char* s1, char* s2)
{
	while(*s1 && *s2)
	{
		if(*s1 != *s2){
			return false;
		}
		s1++;
		s2++;
	}
	return true;	
}

internal bool
compare_strings(String s1, String s2)
{
	if(s1.length != s2.length){
		return false;
	}
	for(u32 i=0 ; s1.text[i] && s2.text[i]; i++)
	{
		if(s1.text[i] != s2.text[i])
			return false;
	}
	return true;
}

internal bool
compare_strings(String s1, char* s2)
{
	for(u32 i=0 ; s1.text[i] && s2[i]; i++)
	{
		if(s1.text[i] != s2[i])
			return false;
	}
	return true;
}

// return value is -1 if substr is not a sub string of str, returns the pos it found the substring otherwise
internal s32
find_substring(String str, String substr){
	UNTIL(i, str.length-substr.length){
		if(compare_strings(substr, {str.text+i, substr.length})){
			return i;
		}
	}
	return -1;
}

internal char
char_to_number(char c)
{
    return c - 48;
}

internal s32
string_to_int(String s)
{
	ASSERT(s.length);
	s32 sign = 1;
	s32 bigger_digit_pos = 0;
	if(*s.text == '-')
	{
		bigger_digit_pos = 1;
		sign = -1;
	}
	s32 result = 0;
	s32 power_of_10 = 1;
	for(s32 i = s.length-1; bigger_digit_pos <= i ; i--)
	{
		char digit = char_to_number(s.text[i]);
		result += digit*power_of_10;
		power_of_10 *= 10;
	}
	return result * sign;
}

internal bool
string_to_bool(String s)
{
	if(!s.text || compare_strings(s, "false") || compare_strings(s, "False"))
		return false;
	else
	{
		if( compare_strings(s, "true") ||
			compare_strings(s, "True") )
			return true;
		else
			return false;
	}
}

internal String
bool_to_string(b32 b)
{
	if(b)
		return string("true");
	else
		return string("false");
}


internal String
u32_to_string(u32 n, Memory_arena* arena)
{
	u32 i=0;
	String result = {0};
	result.text = (char*)arena_push_size(arena, 0);
	if(!n) // if number is 0
	{
		*(char*)arena_push_size(arena, 1) = '0';
		arena_push_size(arena, 1); // 0 ending string
		result.length = 1;
		return result;
	}
	u8 digits = 0;
	s32 temp = n;
	while(temp)
	{
		temp = temp/10;
		i++;
		digits++;
	}
	arena_push_size(arena, digits);
	result.length= i;
	for(;digits; digits--)
	{
		result.text[i-1] = 48 + (n%10);
		n = n/10;
		i--;
	}
	*arena_push_size(arena, 1) = 0; // 0 ending string
	return result;
}

// THIS NEEDS THE MEMORY ARENA
internal String 
s32_to_string(s32 n, Memory_arena* arena)
{
	u32 i=0;
	String result = {0};
	result.text = (char*)arena_push_size(arena, 0);
	if(n < 0)
	{ 
		arena_push_data(arena, "-", 1);
		n = -(n);
		i++;
	}
	if(!n) // if number is 0
	{
		*(char*)arena_push_size(arena, 1) = '0';
		arena_push_size(arena, 1); // 0 ending string
		result.length = 1;
		return result;
	}
	u8 digits = 0;
	s32 temp = n;
	while(temp)
	{
		temp = temp/10;
		i++;
		digits++;
	}
	arena_push_size(arena, digits);
	result.length= i;
	for(;digits; digits--)
	{
		result.text[i-1] = 48 + (n%10);
		n = n/10;
		i--;
	}
	arena_push_data(arena, "\0", 1);
	return result;
}

// THIS NEEDS THE MEMORY ARENA
internal String
concat_strings(String s1, String s2, Memory_arena* arena)
{
	String result = {0};
	result.length = s1.length + s2.length;
	result.text = (char*)arena_push_size(arena, result.length);
	copy_mem(s1.text, result.text, s1.length);
	copy_mem(s2.text, &result.text[s1.length], s2.length);

	arena_push_size(arena, 1); // 0 ending string
	return result;
}