#ifndef __UTILS_H__
#define __UTILS_H__

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

char*
stralloc (size_t len)
{
	char *ret = malloc(len * sizeof(char));
	ret[0] = '\0';
	return ret;
}

int8_t
str_eq (const char *a, const char *b)
{
	while (*a && *b)
		if (*a != *b) return (*a) - (*b);
	return (*a) ? 1 : (*b) : -1 : 0;
}

#define APPEND(buff, str) strcpy((buff) + strlen(buff), (str))

#define VARG_CAT_(x, y) x ## y
#define VARG_ARGN_(a1, a2, a3, a4, a5, N, ...) N
#define VARG_ARGN(...) VARG_ARGN_(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define VARG_CAT(x, y) VARG_CAT_(x, y)
#define VARG_SET(func, ...) VARG_CAT(func, VARG_ARGN(__VA_ARGS__))(__VA_ARGS__)

#endif //__UTILS_H__
