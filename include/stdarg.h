#ifndef _STDARG_H
#define _STDARG_H

#ifdef __MWERKS__

typedef struct {
    char gpr;
    char fpr;
    char reserved[2];
    char* input_arg_area;
    char* reg_save_area;
} __va_list[1];
typedef __va_list va_list;

void* __va_arg(va_list v_list, unsigned char type);

#define va_start(ap, fmt) ((void) fmt, __builtin_va_info(&ap))
#define va_arg(ap, t) (*((t*) __va_arg(ap, _var_arg_typeof(t))))
#define va_end(ap) (void) 0

#else

struct __va_list_tag {
    unsigned char gpr;
    unsigned char fpr;
    void* overflow_arg_area;
    void* reg_save_area;
};

typedef struct __va_list_tag va_list[1];

#define va_start(ap, last)   ((void)(last), *(ap) = *(struct __va_list_tag *)__builtin_saveregs())
#define va_end(ap) ((void)0)

#endif

#endif
