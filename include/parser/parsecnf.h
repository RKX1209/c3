#ifndef PARSECNF_H
#define PARSECNF_H

void skip_until_line (char **p);
void skip_lines (char **p);
bool c3_is_digit (char c);
int32_t get_digit_long (char **p);
int c3_parse_cnffile (C3 *c3, char *data);

#endif
