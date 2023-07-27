/*
 * log_system.h
 *
 *  Created on: 6 de fev de 2023
 *      Author: francisco.felix
 */

#ifndef INC_LOG_SYSTEM_H_
#define INC_LOG_SYSTEM_H_

#include "stdio.h"

#define LOG_FORMAT(letter, format)   "[" #letter "][%s: %u] %s():  "format"\r\n", __FILE__, __LINE__, __FUNCTION__

/*Error
 * */
#if LOG_LEVEL >= 1
#define log_e(format,...) 			printf(LOG_FORMAT(E,format),##__VA_ARGS__)
#else
    #define log_e(format,...)
#endif

/*Warming
 * */
#if LOG_LEVEL >= 2
#define log_w(format,...) 			printf(LOG_FORMAT(W,format),##__VA_ARGS__)
#else
    #define log_w(format,...)
#endif

/*Info*/
#if LOG_LEVEL >= 3
#define log_i(format,...) 			printf(LOG_FORMAT(I,format),##__VA_ARGS__)
#else
    #define log_i(format,...)
#endif

/*Debug
 * */
#if LOG_LEVEL >= 4
#define log_d(format,...) 			printf(LOG_FORMAT(D,format),##__VA_ARGS__)
#else
    #define log_d(format,...)
#endif

/* Verbose
 * */
#if LOG_LEVEL >= 5
#define log_v(format,...) 			printf(LOG_FORMAT(V,format),##__VA_ARGS__)
#else
    #define log_v(format,...)
#endif


#endif /* INC_LOG_SYSTEM_H_ */
