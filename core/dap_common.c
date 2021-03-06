/*
 * Authors:
 * Dmitriy A. Gearasimov <kahovski@gmail.com>
 * DeM Labs Inc.   https://demlabs.net
 * DeM Labs Open source community https://github.com/demlabsinc
 * Copyright  (c) 2017-2018
 * All rights reserved.

 This file is part of DAP (Deus Applications Prototypes) the open source project

    DAP (Deus Applicaions Prototypes) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DAP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with any DAP based project.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef DAP_OS_ANDROID
#include <android/log.h>
#endif

#ifndef _MSC_VER
#include <unistd.h> /* 'pipe', 'read', 'write' */
#include <pthread.h>
#include <syslog.h>
#elif defined(_MSC_VER)
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <process.h>
typedef HANDLE pthread_mutex_t;
#define popen _popen
#define pclose _pclose
#define PTHREAD_MUTEX_INITIALIZER 0
int pthread_mutex_lock(HANDLE **obj)
{
    return (( *obj = (HANDLE) CreateMutex(0, 1, 0) ) == NULL) ? 0 : 1;
}
int pthread_mutex_unlock(HANDLE *obj) {
    return (ReleaseMutex(obj) == 0) ? 0 : 1;
}
#endif
#include <time.h> /* 'nanosleep' */
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "dap_common.h"
#define LAST_ERROR_MAX 255

#define LOG_TAG "dap_common"

char last_error[LAST_ERROR_MAX]={0};
enum log_level log_level=L_DEBUG;
static FILE * s_log_file=NULL;

int dap_common_init( const char * a_log_file )
{
    if ( a_log_file ) {
        s_log_file=fopen( a_log_file , "a");
        if(s_log_file==NULL){
            fprintf(stderr,"Can't open log file %s to append\n", a_log_file);
            s_log_file=stdout;
            return -1;
        }
    }

	return 0;
}

void common_deinit()
{
    if(s_log_file) fclose(s_log_file);
}

void _log_it(const char * log_tag,enum log_level ll, const char * format,...)
{
    if(ll<log_level)
        return;

    va_list ap;


        
    va_start(ap,format);
    _vlog_it(log_tag,ll, format,ap);
    va_end(ap);
}

void _vlog_it(const char * log_tag,enum log_level ll, const char * format,va_list ap)
{
    va_list ap2;

    static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&mutex);
#ifdef DAP_OS_ANDROID
    char buf[4096];
    vsnprintf(buf,sizeof(buf),format,ap);
    switch (ll) {
        case L_INFO:
            __android_log_write(ANDROID_LOG_INFO,DAP_BRAND,buf);
        break;
        case L_WARNING:
            __android_log_write(ANDROID_LOG_WARN,DAP_BRAND,buf);
        break;
        case L_ERROR:
            __android_log_write(ANDROID_LOG_ERROR,DAP_BRAND,buf);
        break;
        case L_CRITICAL:
            __android_log_write(ANDROID_LOG_FATAL,DAP_BRAND,buf);
            abort();
        break;
        case L_DEBUG:
        default:
            __android_log_write(ANDROID_LOG_DEBUG,DAP_BRAND,buf);
    }
#endif


    va_copy(ap2,ap);
    if (s_log_file){
        time_t t=time(NULL);
        struct tm* tmp=localtime(&t);
        static char s_time[1024]={0};
        strftime(s_time,sizeof(s_time),"%x-%X",tmp);

        if (s_log_file ) fprintf(s_log_file,"[%s] ",s_time);
        printf("[%s] ",s_time);
    }
	/*if(ll>=ERROR){
		vsnprintf(last_error,LAST_ERROR_MAX,format,ap);
	}*/

    if(ll==L_DEBUG){
        if (s_log_file ) fprintf(s_log_file,"[DBG] ");
		printf(	"\x1b[37;2m[DBG] ");
    }else if(ll==L_INFO){
        if (s_log_file ) fprintf(s_log_file,"[   ] ");
		printf("\x1b[32;2m[   ] ");
    }else if(ll==L_NOTICE){
        if (s_log_file ) fprintf(s_log_file,"[ * ] ");
		printf("\x1b[32m[ * ] ");
    }else if(ll==L_WARNING){
        if (s_log_file ) fprintf(s_log_file,"[WRN] ");
		printf("\x1b[31;2m[WRN] ");
    }else if(ll==L_ERROR){
        if (s_log_file ) fprintf(s_log_file,"[ERR] ");
        printf("\x1b[31m[ERR] ");
    }else if(ll==L_CRITICAL){
        if (s_log_file ) fprintf(s_log_file,"[!!!] ");
        printf("\x1b[1;5;31m[!!!] ");
    }
    if (s_log_file ) fprintf(s_log_file,"[%8s]\t",log_tag);
    printf("[%8s]\t",log_tag);

    if (s_log_file ) vfprintf(s_log_file,format,ap);
	vprintf(format,ap2);
    if (s_log_file ) fprintf(s_log_file,"\n");
	printf("\x1b[0m\n");
	va_end(ap2);
    if (s_log_file ) fflush(s_log_file);
	fflush(stdout);
    pthread_mutex_unlock(&mutex);
}

const char * log_error()
{
	return last_error;
}

#define INT_DIGITS 19		/* enough for 64 bit integer */

char *itoa(int i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			/* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}

/**
 * @brief time_to_rfc822 Convert time_t to string with RFC822 formatted date and time
 * @param out Output buffer
 * @param out_size_mac Maximum size of output buffer
 * @param t UNIX time
 * @return Length of resulting string if ok or lesser than zero if not
 */
int time_to_rfc822(char * out, size_t out_size_max, time_t t)
{
    struct tm *tmp;
    tmp=localtime(&t);
    if(tmp== NULL){
        log_it(L_ERROR,"Can't convert data from unix fromat to structured one");
        return -2;
    }else{
        int ret;
        ret=strftime(out, out_size_max,"%a, %d %b %y %T %z",tmp);
        //free(tmp);
        if(ret>0){
            return ret;
        }else{
            log_it(L_ERROR,"Can't print formatted time in string");
            return -1;
        }
    }
}



static int breaker_set[2] = { -1, -1 };
static int initialized = 0;
static struct timespec break_latency = {0, 1 * 1000 * 1000 };
#ifndef _MSC_VER
int get_select_breaker()
{
    if (!initialized)
    {
    if (pipe(breaker_set) < 0) return -1;
    else initialized = 1;
    }

    return breaker_set[0];
}

int send_select_break()
{
    if (!initialized) return -1;
    char buffer[1];
    if (write(breaker_set[1], "\0", 1) <= 0) return -1;
    nanosleep(&break_latency, NULL);
    if (read(breaker_set[0], buffer, 1) <= 0 || buffer[0] != '\0') return -1;
    return 0;
}
#else
char *strndup(const char *s, size_t n) {
    char *p = memchr(s, '\0', n);
    if (p != NULL)
        n = p - s;
    p = malloc(n + 1);
    if (p != NULL) {
        memcpy(p, s, n);
        p[n] = '\0';
    }
    return p;
}
#endif

#ifdef ANDROID1
static u_long myNextRandom = 1;

double atof(const char *nptr)
{
    return (strtod(nptr, NULL));
}

int rand(void)
{
    return (int)((myNextRandom = (1103515245 * myNextRandom) + 12345) % ((u_long)RAND_MAX + 1));
}

void srand(u_int seed)
{
    myNextRandom = seed;
}

#endif

/**
 * @brief exec_with_ret
 * @param a_cmd
 * @return
 */
char * exec_with_ret(const char * a_cmd)
{
    FILE * fp;
    size_t buf_len = 0;
    char buf[4096] = {0};
    fp= popen(a_cmd, "r");
    if (!fp) {
        goto FIN;
    }
    memset(buf,0,sizeof(buf));
    fgets(buf,sizeof(buf)-1,fp);
    pclose(fp);
    buf_len=strlen(buf);
    if(buf[buf_len-1] =='\n')buf[buf_len-1] ='\0';
FIN:
    return strdup(buf);
}

char * exec_with_ret_multistring(const char * a_cmd)
{
    FILE * fp;
    size_t buf_len = 0;
    char buf[4096] = {0};
    fp= popen(a_cmd, "r");
    if (!fp) {
        goto FIN;
    }
    memset(buf,0,sizeof(buf));
    char retbuf[4096] = {0};
    while(fgets(buf,sizeof(buf)-1,fp)) {
        strcat(retbuf, buf);
    }
    pclose(fp);
    buf_len=strlen(retbuf);
    if(retbuf[buf_len-1] =='\n')retbuf[buf_len-1] ='\0';
FIN:
    return strdup(retbuf);
}

/**
 * @brief random_string_create
 * @param a_length
 * @return
 */
char * random_string_create(size_t a_length)
{
    const char l_possible_chars[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    char * ret = (char*) malloc(a_length+1);
    size_t i;
    for(i=0; i<a_length; ++i) {
        int index = rand() % (sizeof(l_possible_chars)-1);
        ret[i] = l_possible_chars[index];
    }
    return ret;
}
