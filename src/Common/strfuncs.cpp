#include <ctype.h>

/// Produces the hexadecimal representation of the given input.
/// The output buffer must be at least count*2+1 in size.
/// Returns true on success, false on failure.
///
/// @param output Output string
/// @param input Binary input buffer
/// @param count Number of bytes to convert
bool bin2hex(char* output, unsigned char* input, size_t count) // [eAthena]
{
	char toHex[] = "0123456789abcdef";
	size_t i;

	for( i = 0; i < count; ++i )
	{
		*output++ = toHex[(*input & 0xF0) >> 4];
		*output++ = toHex[(*input & 0x0F) >> 0];
		++input;
	}
	*output = '\0';
	return true;
}

int remove_control_chars(char* str)
{
	int i;
	int change = 0;

	for(i = 0; str[i]; i++) {
		if (iscntrl(str[i])) {
			str[i] = '_';
			change = 1;
		}
	}

	return change;
}

