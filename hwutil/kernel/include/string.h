#ifndef _STRING_H_
#define _STRING_H_

#include "rtos.h"

#ifdef __cplusplus
    extern "C" {
#endif 


#define CH_IS_TEXT              0L
#define CH_IS_BACKSPACE         1L
#define CH_IS_ENTER             2L      
      
/*@{*/

void* memcpy( void *dest, const void *src, size_t count );
void* memset( void *src, const char ch, size_t count );
int memcmp( const char *s1, const char *s2, unsigned int count );
int strcmp( const char *s1, const char *s2 );
int strncmp( const char *s1, const char *s2, size_t count );
int strlen( const char *src );
int strnlen( const char *src, size_t count );
char* strchr( const char *str, char ch );
char* strstr( const char *s1, const char *s2 );
    
int skip_atoi( const char **src );
char* itoa( int number, char * chBuffer );
int atoi( const char *str );
    
int ch_classify(char ch);

/*@}*/    
    
#ifdef __cplusplus
    extern }
#endif

#endif
