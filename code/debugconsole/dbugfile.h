#ifndef _DBUGFILE_HEADER_
#define _DBUGFILE_HEADER_

void dbugfile_init();
void dbugfile_deinit();
void dbugfile_sprintf(int line, char *file, const char *format, ...);

#ifdef DBUGFILE_ACTIVE 

#define DBUGFILE_INIT()   dbugfile_init();
#define DBUGFILE_DEINIT() dbugfile_deinit();

#define DBUGFILE_OUTPUT_0(t) dbugfile_sprintf(__LINE__, __FILE__, t);
#define DBUGFILE_OUTPUT_1(t,a) dbugfile_sprintf(__LINE__, __FILE__, t,a);
#define DBUGFILE_OUTPUT_2(t,a,b) dbugfile_sprintf(__LINE__, __FILE__, t,a,b);
#define DBUGFILE_OUTPUT_3(t,a,b,c) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c);
#define DBUGFILE_OUTPUT_4(t,a,b,c,d) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d);
#define DBUGFILE_OUTPUT_5(t,a,b,c,d,e) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d,e);
#define DBUGFILE_OUTPUT_6(t,a,b,c,d,e,f) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d,e,f;
#define DBUGFILE_OUTPUT_7(t,a,b,c,d,e,f,g) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d,e,f,g);
#define DBUGFILE_OUTPUT_9(t,a,b,c,d,e,f,g,h) dbugfile_sprintf(__LINE__, __FILE__, t,a,b,c,d,e,f,g,h);

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
#define DBUGFILE_OUTPUT_9(t,a,b,c,d,e,f,g,h) ;

#endif

#endif

