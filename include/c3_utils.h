#ifndef C3_UTIL_H
#define C3_UTIL_H

void debug_log (int level, const char *format, ...);
int c3_compare_symbol (const int32_t *a, const int32_t *b);
int c3_compare_value (const int32_t *a, const int32_t *b);
void* c3_copy_value (const int32_t *a);
FILE* c3_file_open (const char *file, const char *mode);
char* c3_file_read (FILE *fp, long *len);
void c3_print_cnf (C3 *c3);

#endif
