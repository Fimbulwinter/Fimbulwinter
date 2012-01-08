#pragma once

/// Produces the hexadecimal representation of the given input.
/// The output buffer must be at least count*2+1 in size.
/// Returns true on success, false on failure.
extern bool bin2hex(char* output, unsigned char* input, size_t count);

extern int remove_control_chars(char* str);

template<typename T, int size>
int GetArrayLength(T(&)[size]){ return size; }