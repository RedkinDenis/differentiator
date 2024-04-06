#ifndef INPUT_OTPUT_H
#define INPUT_OTPUT_H

#include <stdio.h> 
#include "..\..\err_codes.h"
#include "differentiator.h"

int GetFileSize(FILE* fp);

err print_tree (Node* head);

void print_data (Node* head);

err fprint_tree (FILE* out, Node* head);

err importTree (FILE* read ,Node* head);

err draw_tree (Node* tree);

err fill_buffer (FILE* read, char** buf);

int get_g (const char* str);

#endif // INPUT_OTPUT_H
