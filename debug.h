/* 
 * File:   debug.h
 * Author: hanky
 *
 * Created on February 23, 2014, 4:18 PM
 */

#ifndef DEBUG_H
#define	DEBUG_H


#ifdef	__cplusplus
extern "C" {
#endif

#define FNASSO_DEBUG 1
#define FNASSO_EVENT 2
#define FNASSO_WARN 3
#define FNASSO_ERROR 4

#define DEBUG_VERSION
    
#ifdef DEBUG_VERSION

#define F_BEGIN() {\
                fnasso_func_start(__FUNCTION__);\
        }

#define RETURN(status) {\
                fnasso_func_end(FNASSO_DEBUG, __FUNCTION__, status);\
                return status;\
        }


#define E_RETURN(status, args...) {\
                fnasso_printf(FNASSO_ERROR, __FUNCTION__, fnasso_strerror(status), ##args);\
                fnasso_func_end(FNASSO_DEBUG, __FUNCTION__, status);\
                return status;\
        }

#define E_THROW(status) {\
                fnasso_func_end(FNASSO_DEBUG, __FUNCTION__, status);\
                return status;\
        }

#define F_EXIT(status, args...) {\
                fnasso_printf(FNASSO_ERROR, __FUNCTION__, fnasso_strerror(status), ##args);\
                fnasso_func_end(FNASSO_ERROR, __FUNCTION__, status);\
                exit(status);\
        }

#define DEBUG(fmt, args...) {\
                fnasso_printf(FNASSO_DEBUG, __FUNCTION__, fmt, ##args);\
        }

#define EVENT(fmt, args...) {\
                fnasso_printf(FNASSO_EVENT, __FUNCTION__, fmt, ##args);\
        }

#define WARN(fmt, args...) {\
                fnasso_printf(FNASSO_WARN, __FUNCTION__,fmt, ##args);\
        }

#define ERROR(fmt, args...) {\
                fnasso_printf(FNASSO_ERROR, __FUNCTION__,fmt, ##args);\
        }

#else /* DEBUG_VERSION */


#define F_BEGIN() {\
        }

#define RETURN(status) {\
                return status;\
        }


#define E_RETURN(status, args...) {\
                fnasso_printf(FNASSO_ERROR, __FUNCTION__, fnasso_strerror(status), ##args);\
                return status;\
        }

#define E_THROW(status) {\
                return status;\
        }

#define F_EXIT(status, args...) {\
                fnasso_printf(FNASSO_ERROR, __FUNCTION__, fnasso_strerror(status), ##args);\
                exit(status);\
        }

#define DEBUG(fmt, args...) {\
        }

#define EVENT(fmt, args...) {\
                fnasso_printf(FNASSO_EVENT, __FUNCTION__, fmt, ##args);\
        }

#define WARN(fmt, args...) {\
                fnasso_printf(FNASSO_WARN, __FUNCTION__,fmt, ##args);\
        }

#define ERROR(fmt, args...) {\
                fnasso_printf(FNASSO_ERROR, __FUNCTION__,fmt, ##args);\
        }


#endif


#ifdef	__cplusplus
}
#endif

#endif	/* DEBUG_H */

