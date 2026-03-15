

#ifndef r_exec_dll_h
#define r_exec_dll_h


#if defined EXECUTIVE_EXPORTS
#define r_exec_dll dll_export
#define r_exec_tpl
#else
#define r_exec_dll dll_import
#define r_exec_tpl extern
#endif


#endif
