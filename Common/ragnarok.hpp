#pragma once

#define NAME_LENGTH (23 + 1)

enum {
	SEX_FEMALE = 0,
	SEX_MALE,
	SEX_SERVER
};

#define sex_num2str(num) ( (num ==  SEX_FEMALE  ) ? 'F' : (num ==  SEX_MALE  ) ? 'M' : 'S' )
#define sex_str2num(str) ( (str == 'F' ) ?  SEX_FEMALE  : (str == 'M' ) ?  SEX_MALE  :  SEX_SERVER  )
