
#ifndef _DBUGFILE_HEADER_
#define _DBUGFILE_HEADER_

#define MAX_COUNTERS 10

void dbugfile_init();
void dbugfile_deinit();
void dbugfile_sprintf(int line, char *file, const char *format, ...);
void dbugfile_print_matrix_3x3(int line, char *file, float *matrix, char *text);
void dbugfile_print_matrix_4x4(int line, char *file, float *matrix, char *text);
void dbugfile_print_counter(int line, char *file, int counter_num, char *string);

#ifdef DBUGFILE_ACTIVE 

#define DBUGFILE_INIT()   dbugfile_init();
#define DBUGFILE_DEINIT() dbugfile_deinit();

#define DBUGFILE_OUTPUT_0(t) dbugfile_sprintf(__LINE__, __FILE__, t);
#define DBUGFILE_OUTPUT_1(t,a) dbugfile_sprintf(__LINE__, __FILE__, t,a);
#define DBUGFILE_OUTPUT_2(t,a,b) dbugfile_sprintf(__LINE__, __FILE__, t,a,b);
#define DBUGFILE_OUTPUT_3(t,a,b,c) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c);
#define DBUGFILE_OUTPUT_4(t,a,b,c,d) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d);
#define DBUGFILE_OUTPUT_5(t,a,b,c,d,e) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d,e);
#define DBUGFILE_OUTPUT_6(t,a,b,c,d,e,f) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d,e,f);
#define DBUGFILE_OUTPUT_7(t,a,b,c,d,e,f,g) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d,e,f,g);
#define DBUGFILE_OUTPUT_8(t,a,b,c,d,e,f,g,h) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d,e,f,g,h);
#define DBUGFILE_OUTPUT_9(t,a,b,c,d,e,f,g,h,i) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d,e,f,g,h,i);

#define DBUGFILE_OUTPUT_MATRIX_3X3(m,t) dbugfile_print_matrix_3x3(__LINE__, __FILE__, m,t);
#define DBUGFILE_OUTPUT_MATRIX_4X4(m,t) dbugfile_print_matrix_4x4(__LINE__, __FILE__, m,t);

extern int dbugfile_counters[MAX_COUNTERS];
#define DBUGFILE_INC_COUNTER(n) if(n < MAX_COUNTERS) dbugfile_counters[n]++;
#define DBUGFILE_DEC_COUNTER(n) if(n < MAX_COUNTERS) dbugfile_counters[n]--;

#define DBUGFILE_OUTPUT_COUNTER(n,s)	dbugfile_print_counter(__LINE__, __FILE__, n, s);

#else

#define DBUGFILE_INIT()   ;
#define DBUGFILE_DEINIT() ;

#define DBUGFILE_OUTPUT_0(t) ;
#define DBUGFILE_OUTPUT_1(t,a) ;
#define DBUGFILE_OUTPUT_2(t,a,b) ;
#define DBUGFILE_OUTPUT_3(t,a,b,c) ;
#define DBUGFILE_OUTPUT_4(t,a,b,c,d) ;
#define DBUGFILE_OUTPUT_5(t,a,b,c,d,e) ;
#define DBUGFILE_OUTPUT_6(t,a,b,c,d,e,f) ;
#define DBUGFILE_OUTPUT_7(t,a,b,c,d,e,f,g) ;
#define DBUGFILE_OUTPUT_8(t,a,b,c,d,e,f,g,h) ;
#define DBUGFILE_OUTPUT_9(t,a,b,c,d,e,f,g,h,i) ;

#define DBUGFILE_OUTPUT_MATRIX_3X3(m,t) ;
#define DBUGFILE_OUTPUT_MATRIX_4X4(m,t) ;

#define DBUGFILE_INC_COUNTER(n) ;
#define DBUGFILE_DEC_COUNTER(n) ;

#define DBUGFILE_OUTPUT_COUNTER(n,s) ;

#endif

#endif

