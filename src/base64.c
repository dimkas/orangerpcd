/*
cencode. - c source to a base64 encoding algorithm implementation

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64
*/

#include "base64.h"
#include <string.h>

typedef enum
{
	step_A, step_B, step_C
} base64_encodestep;

typedef struct
{
	base64_encodestep step;
	char result;
	int stepcount;
} base64_encodestate;

typedef enum
{
	step_a, step_b, step_c, step_d
} base64_decodestep;

typedef struct
{
	base64_decodestep step;
	char plainchar;
} base64_decodestate;

const int CHARS_PER_LINE = 72;

static void base64_init_encodestate(base64_encodestate* state_in){
	state_in->step = step_A;
	state_in->result = 0;
	state_in->stepcount = 0;
}

static char base64_encode_value(char value_in){
	static const char* encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	if (value_in > 63) return '=';
	return encoding[(int)value_in];
}

static int base64_encode_block(const char* plaintext_in, char* code_out, int size_out, base64_encodestate* state_in){
	const char* plainchar = plaintext_in;
	const char* const plaintextend = plaintext_in + strlen(plaintext_in);
	char* codechar = code_out;
	char result;
	char fragment;
	
	result = state_in->result;
	
	switch (state_in->step)
	{
		while (1)
		{
	case step_A:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_A;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result = (fragment & 0x0fc) >> 2;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x003) << 4;
			__attribute__((fallthrough));
	case step_B:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_B;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result |= (fragment & 0x0f0) >> 4;
			*codechar++ = base64_encode_value(result);
			result = (fragment & 0x00f) << 2;
			__attribute__((fallthrough));
	case step_C:
			if (plainchar == plaintextend)
			{
				state_in->result = result;
				state_in->step = step_C;
				return codechar - code_out;
			}
			fragment = *plainchar++;
			result |= (fragment & 0x0c0) >> 6;
			*codechar++ = base64_encode_value(result);
			result  = (fragment & 0x03f) >> 0;
			*codechar++ = base64_encode_value(result);
			
			++(state_in->stepcount);
			/*if (state_in->stepcount == CHARS_PER_LINE/4)
			{
				*codechar++ = '\n';
				state_in->stepcount = 0;
			}
			*/
		}
	}
	/* control should not reach here */
	return codechar - code_out;
}

static int base64_encode_blockend(char* code_out, base64_encodestate* state_in){
	char* codechar = code_out;
	
	switch (state_in->step)
	{
	case step_B:
		*codechar++ = base64_encode_value(state_in->result);
		*codechar++ = '=';
		*codechar++ = '=';
		break;
	case step_C:
		*codechar++ = base64_encode_value(state_in->result);
		*codechar++ = '=';
		break;
	case step_A:
		break;
	}
	//*codechar++ = '\n';
	
	return codechar - code_out;
}

static int base64_decode_value(int value_in){
	static const char decoding[] = {62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51};
	static const char decoding_size = sizeof(decoding);
	value_in -= 43;
	if (value_in < 0 || value_in > decoding_size) return -1;
	return decoding[value_in];
}

static void base64_init_decodestate(base64_decodestate* state_in){
	state_in->step = step_a;
	state_in->plainchar = 0;
}

static int base64_decode_block(const char* code_in, const int length_in, char* plaintext_out, size_t size_out, base64_decodestate* state_in){
	const char* codechar = code_in;
	char* plainchar = plaintext_out;
	int fragment;
	
	*plainchar = state_in->plainchar;
	
	switch (state_in->step)
	{
		while (1)
		{
	case step_a:
			do {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_a;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar    = (fragment & 0x03f) << 2;
			if(plainchar >= (plaintext_out + size_out - 1)) break;
			__attribute__((fallthrough));
	case step_b:
			do {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_b;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++ |= (fragment & 0x030) >> 4;
			*plainchar    = (fragment & 0x00f) << 4;
			if(plainchar >= (plaintext_out + size_out - 1)) break;
			__attribute__((fallthrough));
	case step_c:
			do {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_c;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++ |= (fragment & 0x03c) >> 2;
			*plainchar    = (fragment & 0x003) << 6;
			if(plainchar >= (plaintext_out + size_out - 1)) break;
			__attribute__((fallthrough));
	case step_d:
			do {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_d;
					state_in->plainchar = *plainchar;
					return plainchar - plaintext_out;
				}
				fragment = (char)base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++   |= (fragment & 0x03f);
			if(plainchar >= (plaintext_out + size_out - 1)) break;
		}
	}
	/* control should not reach here */
	size_t s = plainchar - plaintext_out;
	plaintext_out[s] = 0; 
	return s; 
}

int base64_encode(const char* text_in, char *code_out, size_t length_out){
	base64_encodestate st; 
	base64_init_encodestate(&st); 	
	int n = base64_encode_block(text_in, code_out, length_out, &st); 
	n += base64_encode_blockend(code_out + n, &st); 
	code_out[n] = 0; 
	return n; 
}

int base64_decode(const char* code_in, char* plaintext_out, size_t size_out){
	base64_decodestate st; 
	base64_init_decodestate(&st); 	
	return base64_decode_block(code_in, strlen(code_in), plaintext_out, size_out, &st); 
}
