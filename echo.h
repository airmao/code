/**
 * @file echo.h
 * @brief echo
 * @note 
 * @version 0.1
 */

#ifndef _ECHO_H_
#define _ECHO_H_

#define QLEN            20
#define BUFLEN          128
#define ECHO_DELAY_USEC (200 * 1000 * 1000)
#define TRUE            1
#define FALSE           0

typedef enum echo_serv_trs_type_e {
    ECHO_TYPE_UNSET = 0, 
    ECHO_TYPE_UTOL_ALL, 
    ECHO_TYPE_LTOU_ALL, 
    ECHO_TYPE_NUM,
} echo_serv_trs_type_t;

typedef struct echo_info_s {
    echo_serv_trs_type_t cvt_type;
    char                 buf[BUFLEN];
} echo_info_t;

#endif
