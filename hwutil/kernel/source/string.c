/*
 * File         : string.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-11-20       DongHeng        create
 */

#include "string.h"

#include "stdio.h"
/*@{*/

/**
  * memcpy - This function will copy memory content form source address to 
  *          destination  address.
  *
  * @param dest  the address of destination memory
  * @param src   the addrss of source memory
  * @param count the copied length of byte data
  *
  * @return the address of destination memory
  */
void* memcpy(void *dest, const void *src, size_t count)
{
    char *_d = (char *)dest, *_s = (char *)src;
    
    while (count--)
        *_d++ = *_s++;
    
    return dest;
}

/**
  * memset - This function will set the memory of the specified length to the 
  *          specified vlaue.
  *
  * @param src   the addrss of source memory
  * @param ch    the specified value to be set
  * @param count the set length of byte data
  *
  * @return the address of source memory
  */
void* memset(void *src, char ch, size_t count)
{
    char *_s = (char *)src;
    
    while (count--)
        *_s++ = ch;
    
    return src;
}

/**
  * memcmp - This function will compare two memory block with specified length.
  *
  * @param s1    the source 1 string point to be compared
  * @param s2    the source 2 string point to be compared
  * @param count the length of the memory block
  *
  * @return the result
  */
int memcmp( const char *s1, const char *s2, unsigned int count )
{
    while (--count && *s1 == *s2)
        s1++, s2++;
  
    return (*s1 - *s2);
}

/**
  * strcmp - This function will compare two string without specified length.
  *
  * @param s1 the source 1 string point to be compared
  * @param s2 the source 2 string point to be compared
  *
  * @return the result
  */
int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s1 == *s2)
        s1++, s2++;
  
    return (*s1 - *s2);
}

/**
  * strncmp - This function will compare two string with specified length.
  *
  * @param s1    the source 1 string point to be compared
  * @param s2    the source 2 string point to be compared
  * @param count the maxium length of string to be compared
  *
  * @return the result
  */
int strncmp(const char *s1, const char *s2, size_t count)
{
    while (count-- && *s1 && *s1 == *s2)
        s1++, s2++;
  
    return (*s1 - *s2);
}

/**
  * strlen - This function will return the length of the string.
  *
  * @param src the source string
  *
  * @return the length of string
  */
int strlen( const char *src )
{
    const char *_s = src;
    
    for ( _s = src; *_s; ++_s );
    
    return (_s - src);
}

/**
  * strnlen - This function will return the length of the string with specified and limited 
  * length.
  *
  * @param src   the source string
  * @param count the maxium length to be checked 
  *
  * @return the length of string with the specified and limited length
  */
int strnlen( const char *src, size_t count )
{
    const char *_s = src;
    
    for ( _s = src; count-- && *_s; ++_s );
    
    return (_s - src);
}

/**
  * strchr - This function will search the specified character
  *
  * @param src the source string
  * @param ch  the specified character 
  *
  * @return the point where the byte is the same as the specified character or 
  * return null if not find
  */
char* strchr( const char *str, char ch )
{
    while (*str && *str != ch)
        str++;
  
    return (*str == ch) ? (char *)str : NULL;
}

/**
  * strstr - This function will search the specified string length.
  *
  * @param s1 the source string
  * @param s2 the target string 
  *
  * @return the point where the source string is the same as the target string 
  *         or return null if not find
 */
char* strstr(const char *s1, const char *s2)
{
    int l1, l2;
    
    l2 = strlen(s2) - 1;
    l1 = strlen(s1);
    
    while (l1 >= l2)
    {
        if (!strncmp(s1, s2, l2))
            return (char *)s1;
        ++s1;
        --l1;
    }
    
    return NULL;
}

/**
  * skip_atoi - This function will transform the string number into intger number
  *
  * @param src the source string
  *
  * @return the intger number
  */
int skip_atoi( const char **src )
{
    int count = 0;
  
    while (is_digit(**src))
        count = count * 10 + *((*src)++) - '0';
    
    return count;
}

/**
  * atoi - This function will transform the string number into intger number
  *
  * @param src the source string
  *
  * @return the intger number
  */
int atoi(const char *str)
{
   int sign;
   int n;
   const char *p = str;
   
   while (is_space(*p) ) p++;
   
   sign = ('-' == *p) ? -1 : 1;
   if ('+' == *p || '-' == *p) p++;
   
   for (n = 0; is_digit(*p); p++){
      n = 10 * n + (*p - '0');
   }
   
   return (sign * n);
}

/**
  * itoa - This function will transform the intger number into string number
  *
  * @param number the intger number
  * @param chBuffer the point to store string number
  *
  * @return the point storing string number
  */
char* itoa( int number, char * chBuffer )
{
  int i = 1;
  char * pch = chBuffer;
  
  if( !pch ) {
    return 0;
  }
  
  while( number / i ) {
    i *= 10;
  }
  if( number < 0 ) {
    number = -number;
    *pch++ = '-';
  }
  if( 0 == number ) {
    i = 10;
  }
  while( i /= 10 ) {
    *pch++ = number / i + '0';
    number %= i;
  }
  *pch = '\0';
  
  return chBuffer;
}

/**
  * the function will classify the input charactor
  *
  * @param ch the input charactor
  *
  * @return the charactor class
  */
int ch_classify(char ch)
{
    int ret = -1;
    
    if (ch <= '9' && ch >= '0')
        ret = CH_IS_TEXT;
    else if (ch <= 'z' && ch >= 'a')
        ret = CH_IS_TEXT;
    else if (ch <= 'Z' && ch >= 'A')
        ret = CH_IS_TEXT;
    else if (ch == '_')
        ret = CH_IS_TEXT;
    else if (ch == ' ')
        ret = CH_IS_TEXT;
    else if (0x7f == ch)
        ret = CH_IS_BACKSPACE;
    else if (0x0d == ch)
        ret = CH_IS_ENTER;
    
    return ret;
}

/*@}*/

