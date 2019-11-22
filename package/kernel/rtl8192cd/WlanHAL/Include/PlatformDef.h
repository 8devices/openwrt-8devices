#ifndef __INC_PLATFORMDEF_H
#define __INC_PLATFORMDEF_H


// Filen: RT_PLATFORM==PLATFORM_LINUX
	// Macros for integer constants
        #define INT8_C(v)    (v)
        #define INT16_C(v)   (v)
        #define INT32_C(v)   (v ## L)
        #define INT64_C(v)   (v ## LL)

        #define UINT8_C(v)   (v)
        #define UINT16_C(v)  (v)
        #define UINT32_C(v)  (v ## UL)
        #define UINT64_C(v)  (v ## ULL)

        #define INTMAX_C(v)  (v ## LL)
        #define UINTMAX_C(v) (v ## ULL)

        #define i64fmt          "ll"


#endif // #ifndef __INC_PLATFORMDEF_H


