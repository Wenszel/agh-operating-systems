#include "command_utils.h"
#include <stdio.h>
#include <string.h>

char* get_command(const char* input) {
    static char result[256];  // Zakładając, że maksymalna długość stringa to 256 znaków
    int i = 0;

    // Przechodzenie przez string do napotkania spacji lub końca stringa
    while (input[i] != ' ' && input[i] != '\0') {
        result[i] = input[i];
        i++;
    }
    result[i] = '\0';  // Dodanie null-terminatora na końcu

    return result;
}

char* get_string_after_command(const char* input) {
    // Znajdź wskaźnik do pierwszej spacji w stringu
    const char* spacePos = strchr(input, ' ');
    
    // Jeżeli spacja nie została znaleziona, zwróć pusty string
    if (spacePos == NULL) {
        return "";
    }
    
    // Przesuń wskaźnik o jedno miejsce, aby pominąć spację
    spacePos++;
    
    // Zwróć wskaźnik do reszty stringa
    // Usuń \n z konca stringa:
    char* end = strchr(spacePos, '\n');
    if (end != NULL) {
        *end = '\0';
    }
    return (char*)spacePos;
}

char* get_param_after_command(const char* input) {
    static char result[256]; // Zakładając, że maksymalna długość stringa to 256 znaków
    int start = -1, end = -1;
    int i = 0;
    
    // Przechodzenie przez string w poszukiwaniu pierwszej i drugiej spacji
    while (input[i] != '\0') {
        if (input[i] == ' ') {
            if (start == -1) {
                start = i;
            } else if (end == -1) {
                end = i;
                break;
            }
        }
        i++;
    }
    
    // Jeżeli nie znaleziono dwóch spacji, zwróć pusty string
    if (start == -1 || end == -1) {
        return "";
    }
    
    // Skopiowanie substringa między pierwszą a drugą spacją
    int j;
    for (j = start + 1; j < end; j++) {
        result[j - start - 1] = input[j];
    }
    result[j - start - 1] = '\0'; // Dodanie null-terminatora na końcu
    
    return result;
}

char* get_string_after_param(const char* input) {
    const char* spacePos = strchr(input, ' ');
    if (spacePos == NULL) {
        return "";
    }
    spacePos = strchr(spacePos + 1, ' ');
    if (spacePos == NULL) {
        return "";
    }
    spacePos++;
    char* end = strchr(spacePos, '\n');
    if (end != NULL) {
        *end = '\0';
    }
    return (char*)spacePos;
}