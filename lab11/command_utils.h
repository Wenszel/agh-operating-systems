#ifndef COMMAND_UTILS_H
#define COMMAND_UTILS_H

char* get_command(const char* input);
char* get_string_after_command(const char* input);
char* get_param_after_command(const char* input);
char* get_string_after_param(const char* input);
#endif 