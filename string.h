/**
 * 
 * A rather limited version of <string.h>
 *
 * @author Bruce Tan
 * @email brucetansh@gmail.com
 * 
 * @author Paul Monigatti
 * @email paulmoni@waikato.ac.nz
 * 
 * @create date 2016-09-19
 * 
*/
#ifndef _STRING_H_
#define _STRING_H_ 1

//void *memcpy(void *s1, const void *s2, size_t n);
//int strcmp(const char *s1, const char *s2);
//int strlen(const char *s);
//char *strcpy(char *dest,const char *src);
//char *strcat(char *dest, const char *src);
//char *strchr(char *s, int c);
//void *memset(void *dst, int c, size_t n);
int substring(char* buffer,char* original,int start_index,int length);

#endif
