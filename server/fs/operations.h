#ifndef FS_H
#define FS_H
#include "state.h"

void disable_locks(int vector[], int limit);
void initialize_vector(int vector[], int limit);
void init_fs();
void destroy_fs();
int is_dir_empty(DirEntry *dirEntries);
int create(char *name, type nodeType);
int delete(char *name);
int lookup(char *name);
int move(char* current_pathname, char* new_pathname);
void print_tecnicofs_tree(FILE *fp);

#endif /* FS_H */
