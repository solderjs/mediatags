#ifndef JSONDEFS_H
#define JSONDEFS_H

/*
    Defines all of the types of functions and various other definitions
    that are used in C applications, this is very useful if dynamically loading
    the library instead of linking.
*/

#include "../JSONOptions.h"

#define JSON_NULL '\0'
#define JSON_STRING '\1'
#define JSON_NUMBER '\2'
#define JSON_BOOL '\3'
#define JSON_ARRAY '\4'
#define JSON_NODE '\5'

#ifdef __cplusplus
    #ifdef JSON_STRING_HEADER
	   #include JSON_STRING_HEADER
    #else
	   #include <string>
    #endif
#endif

#ifdef JSON_UNICODE
    #ifdef JSON_ISO_STRICT
	   #error, You can not use unicode under ISO Strict C++
    #endif
    #define json_char wchar_t
    #define json_uchar wchar_t
    #ifdef __cplusplus
	   #include <cwchar>  //need wide characters
	   #ifndef JSON_STRING_HEADER
		  typedef std::wstring json_string;
	   #endif
    #else
	   #include <wchar.h>  //need wide characters
    #endif
    #define JSON_TEXT(s) L ## s
    #define json_strlen wcslen
    #define json_strcmp wcscmp
#else
    #define json_char char
    #define json_uchar unsigned char
    #ifdef __cplusplus
	   #ifndef JSON_STRING_HEADER
		  typedef std::string json_string;
	   #endif
    #endif
    #define JSON_TEXT(s) s
    #define json_strlen strlen
    #define json_strcmp strcmp
#endif

#ifdef JSON_LESS_MEMORY
    #define BITS(x) :x  //tells the compiler how many bits to use for a field
    #define START_MEM_SCOPE {
    #define END_MEM_SCOPE }
    typedef float json_number;
#else
    #define BITS(x)
    #define START_MEM_SCOPE
    #define END_MEM_SCOPE
    typedef double json_number;
#endif

#if defined JSON_DEBUG || defined JSON_SAFE
    #ifdef JSON_LIBRARY
	   typedef void (*json_error_callback_t)(const json_char *);
    #else
	   typedef void (*json_error_callback_t)(const json_string &);
    #endif
#endif

#ifdef JSON_INDEX_TYPE
    typedef JSON_INDEX_TYPE json_index_t;
#else
    typedef unsigned int json_index_t;
#endif

#ifdef JSON_BOOL_TYPE
    typedef JSON_BOOL_TYPE json_bool_t;
#else
    typedef int json_bool_t;
#endif

typedef void (*json_mutex_callback_t)(void *);
typedef void (*json_free_t)(void *);
#ifndef JSON_LIBRARY
    typedef void * (*json_malloc_t)(size_t);
    typedef void * (*json_realloc_t)(void *, size_t);
#else
    #define JSONNODE void  //so that JSONNODE* is void*
    typedef JSONNODE** JSONNODE_ITERATOR;
    typedef void * (*json_malloc_t)(unsigned long);
    typedef void * (*json_realloc_t)(void *, unsigned long);
#endif

#endif //JSONDEFS_H
