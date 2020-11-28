#include "operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void initialize_vector(int vector[], int limit) {
	for (int i = limit - 1; i >= 0; i--) {
		vector[i] = FREE_INODE;
	}
}

void disable_locks(int vector[], int limit) {
	for (int i = limit - 1; i >= 0; i--) {
		if (vector[i] != FREE_INODE) {
			inode_lock_disable(vector[i]);
			vector[i] = FREE_INODE;
		}
	}
}


/* Given a path, fills pointers with strings for the parent path and child
 * file name
 * Input:
 *  - path: the path to split. ATENTION: the function may alter this parameter
 *  - parent: reference to a char*, to store parent path
 *  - child: reference to a char*, to store child file name
 */
void split_parent_child_from_path(char * path, char ** parent, char ** child) {

	int n_slashes = 0, last_slash_location = 0;
	int len = strlen(path);

	// deal with trailing slash ( a/x vs a/x/ )
	if (path[len-1] == '/') {
		path[len-1] = '\0';
	}

	for (int i=0; i < len; ++i) {
		if (path[i] == '/' && path[i+1] != '\0') {
			last_slash_location = i;
			n_slashes++;
		}
	}

	if (n_slashes == 0) { // root directory
		*parent = "";
		*child = path;
		return;
	}

	path[last_slash_location] = '\0';
	*parent = path;
	*child = path + last_slash_location + 1;

}


/*
 * Initializes tecnicofs and creates root node.
 */
void init_fs() {
	inode_table_init();

	/* create root inode */
	int root = inode_create(T_DIRECTORY);

	if (root != FS_ROOT) {
		printf("failed to create node for tecnicofs root\n");
		exit(EXIT_FAILURE);
	}
}


/*
 * Destroy tecnicofs and inode table.
 */
void destroy_fs() {
	inode_table_destroy();
}


/*
 * Checks if content of directory is not empty.
 * Input:
 *  - entries: entries of directory
 * Returns: SUCCESS or FAIL
 */

int is_dir_empty(DirEntry *dirEntries) {
	if (dirEntries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if (dirEntries[i].inumber != FREE_INODE) {
			return FAIL;
		}
	}
	return SUCCESS;
}


/*
 * Looks for node in directory entry from name.
 * Input:
 *  - name: path of node
 *  - entries: entries of directory
 * Returns:
 *  - inumber: found node's inumber
 *  - FAIL: if not found
 */
int lookup_sub_node(char *name, DirEntry *entries) {
	if (entries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if (entries[i].inumber != FREE_INODE && strcmp(entries[i].name, name) == 0) {
			return entries[i].inumber;
		}
	}
	return FAIL;
}

void display_create(char * name, type nodeType){
	if (nodeType == T_FILE){
		printf("Create file: %s\n", name);
	}
	else{ /* nodeType == T_Directory*/
		printf("Create directory: %s\n", name);
	}
}


/*
 * Creates a new node given a path.
 * Input:
 *  - name: path of node
 *  - nodeType: type of node
 * Returns: SUCCESS or FAIL
 */
int create(char *name, type nodeType){
	int vector_inumber[INODE_TABLE_SIZE];
	int i = 0;

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType;
	union Data pdata;

	initialize_vector(vector_inumber, INODE_TABLE_SIZE);

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = lookup(parent_name);

	if (parent_inumber == FAIL) {
		display_create(name, nodeType);
		printf("failed to create %s, invalid parent dir %s\n",
				name, parent_name);
		return FAIL;
	}

	inode_lock_enable(parent_inumber, 'w');
	vector_inumber[i++] = parent_inumber;

	inode_get(parent_inumber, &pType, &pdata);

	if(pType != T_DIRECTORY) {
		display_create(name, nodeType);
		printf("failed to create %s, parent %s is not a dir\n",
				name, parent_name);

		disable_locks(vector_inumber, INODE_TABLE_SIZE);
		return FAIL;
	}

	if (lookup_sub_node(child_name, pdata.dirEntries) != FAIL) {
		display_create(name, nodeType);
		printf("failed to create %s, already exists in dir %s\n",
				child_name, parent_name);

		disable_locks(vector_inumber, INODE_TABLE_SIZE);
		return FAIL;
	}

	/* create node and add entry to folder that contains new node */
	child_inumber = inode_create(nodeType);

	if (child_inumber == FAIL) {
		display_create(name, nodeType);
		printf("failed to create %s in  %s, couldn't allocate inode\n",
				child_name, parent_name);

		disable_locks(vector_inumber, INODE_TABLE_SIZE);
		return FAIL;
	}

	inode_lock_enable(child_inumber, 'w');
	vector_inumber[i] = child_inumber;

	if (dir_add_entry(parent_inumber, child_inumber, child_name) == FAIL) {
		display_create(name, nodeType);
		printf("could not add entry %s in dir %s\n",
				child_name, parent_name);

		disable_locks(vector_inumber, INODE_TABLE_SIZE);
		return FAIL;
	}

	disable_locks(vector_inumber, INODE_TABLE_SIZE);
	display_create(name, nodeType);
	return SUCCESS;
}


/*
 * Deletes a node given a path.
 * Input:
 *  - name: path of node
 * Returns: SUCCESS or FAIL
 */
int delete(char *name){
	int vector_inumber[INODE_TABLE_SIZE];
	int i = 0;

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType, cType;
	union Data pdata, cdata;

	initialize_vector(vector_inumber, INODE_TABLE_SIZE);

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = lookup(parent_name);

	if (parent_inumber == FAIL) {
		printf("Delete: %s\n", name);
		printf("failed to delete %s, invalid parent dir %s\n",
				child_name, parent_name);	 	

		return FAIL;
	}

	inode_lock_enable(parent_inumber, 'w');
	vector_inumber[i++] = parent_inumber;

	inode_get(parent_inumber, &pType, &pdata);

	if(pType != T_DIRECTORY) {
		printf("Delete: %s\n", name);
		printf("failed to delete %s, parent %s is not a dir\n",
				child_name, parent_name);

		disable_locks(vector_inumber, INODE_TABLE_SIZE);
		return FAIL;
	}

	child_inumber = lookup_sub_node(child_name, pdata.dirEntries);

	if (child_inumber == FAIL) {
		printf("Delete: %s\n", name);
		printf("could not delete %s, does not exist in dir %s\n",
				name, parent_name);

		disable_locks(vector_inumber, INODE_TABLE_SIZE);
		return FAIL;
	}

	inode_lock_enable(child_inumber, 'w');
	vector_inumber[i++] = child_inumber;
	inode_get(child_inumber, &cType, &cdata);

	if (cType == T_DIRECTORY && is_dir_empty(cdata.dirEntries) == FAIL) {
		printf("Delete: %s\n", name);
		printf("could not delete %s: is a directory and not empty\n",
				name);

		disable_locks(vector_inumber, INODE_TABLE_SIZE);
		return FAIL;
	}

	/* remove entry from folder that contained deleted node */
	if (dir_reset_entry(parent_inumber, child_inumber) == FAIL) {
		printf("Delete: %s\n", name);
		printf("failed to delete %s from dir %s\n",
				child_name, parent_name);

		disable_locks(vector_inumber, INODE_TABLE_SIZE);
		return FAIL;
	}

	if (inode_delete(child_inumber) == FAIL) {
		printf("Delete: %s\n", name);
		printf("could not delete inode number %d from dir %s\n",
				child_inumber, parent_name);

		vector_inumber[child_inumber] = FREE_INODE;
		disable_locks(vector_inumber, INODE_TABLE_SIZE);
		return FAIL;
	}

	disable_locks(vector_inumber, INODE_TABLE_SIZE);
	printf("Delete: %s\n", name);
	return SUCCESS;
}


/*
 * Lookup for a given path.
 * Input:
 *  - name: path of node
 * Returns:
 *  inumber: identifier of the i-node, if found
 *     FAIL: otherwise
 */
int lookup(char *name) {
	int vector_inumber[INODE_TABLE_SIZE];
	int i = 0;

	char full_path[MAX_FILE_NAME];
	char delim[] = "/";
	char *saveptr;

	initialize_vector(vector_inumber, INODE_TABLE_SIZE);

	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	inode_lock_enable(current_inumber, 'r');
	vector_inumber[i++] = current_inumber;

	/* get root inode data */
	inode_get(current_inumber, &nType, &data);

	char *path = strtok_r(full_path, delim, &saveptr);

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL) {
		
		inode_lock_enable(current_inumber, 'r');
		vector_inumber[i++] = current_inumber;

		inode_get(current_inumber, &nType, &data);
		path = strtok_r(NULL, delim, &saveptr);
	}

	disable_locks(vector_inumber, INODE_TABLE_SIZE);
	return current_inumber;
}

void sort_vector(int vector[3], int a, int b, int c){
	if (a < b){
		if (a < c) {
			if (c < b) {
				vector[0] = a;
				vector[1] = c;
				vector[2] = b;
			}

			else{
				vector[0] = a;
				vector[1] = b;
				vector[2] = c;
			}
		}

		else{
			vector[0] = c;
			vector[1] = a;
			vector[2] = b;
		}
	}

	else{ /* b < a */
		if(a < c){
			vector[0] = b;
			vector[1] = a;
			vector[2] = c;
		}
		else{ /* c < a*/
			if(c < b){
				vector[0] = c;
				vector[1] = b;
				vector[2] = a;
			}

			else{ /* b < c*/
				vector[0] = b;
				vector[1] = c;
				vector[2] = a;
			}
		}
	}
}


int move(char* current_pathname, char* new_pathname) {
	int vector_inumber[3];
	int count = 0, constant = 1, max, flag = 0;

	int current_parent_inumber, child_inumber, new_parent_inumber;	
	char *current_parent_name, *current_child_name;
	char *new_parent_name, *new_child_name;
	char current_pathname_copy[MAX_FILE_NAME];
	char new_pathname_copy[MAX_FILE_NAME];

	initialize_vector(vector_inumber, 3);
	strcpy(new_pathname_copy, new_pathname);
	strcpy(current_pathname_copy, current_pathname);
	
	child_inumber = lookup(current_pathname);

	/* checks if there is a directory/file with the current pathname*/
	if (child_inumber == FAIL) {
		printf("Moving: %s to %s\n", current_pathname_copy, new_pathname_copy);
		printf("failed to move %s to %s, %s doesn't exist\n",
				current_pathname, new_pathname, current_pathname);
		return FAIL;
	}

	/* checks if there isn't a directory/file with the new pathname*/
	if (lookup(new_pathname) != FAIL) {
		printf("Moving: %s to %s\n", current_pathname_copy, new_pathname_copy);
		printf("failed to move %s to %s, there is already a %s\n",
				current_pathname, new_pathname, new_pathname);
		return FAIL;
	}

	/* separates child from parent in the current pathname*/
	split_parent_child_from_path(current_pathname, &current_parent_name,
			&current_child_name);

	/* separates child from parent in the new pathname*/
	split_parent_child_from_path(new_pathname, &new_parent_name, &new_child_name);

	new_parent_inumber = lookup(new_parent_name);
	current_parent_inumber = lookup(current_parent_name);

	/* Example: "m /a /a/a". Prevent loops */
	if (new_parent_inumber != 0 && strstr(current_pathname_copy, new_parent_name) != NULL) {
		printf("Moving: %s to %s\n", current_pathname_copy, new_pathname_copy);
		printf("failed to move %s to %s, loop would occur\n",
				current_pathname_copy, new_pathname_copy);

		disable_locks(vector_inumber, 3);
		return FAIL;
	}

	/* Sorting the vector_inumber by ascending order*/
	sort_vector(vector_inumber, new_parent_inumber, current_parent_inumber, child_inumber);

	/* Locking the inodes by ascending order*/
	while (!flag) {
		max = constant * count;

		if (inode_lock_try(vector_inumber[0], 'w')) {
			if (inode_lock_try(vector_inumber[1], 'w')) {
				if (inode_lock_try(vector_inumber[2], 'w'))
					/* can lock all three inodes*/
					flag = 1;
				else{
					count ++;
					inode_lock_disable(vector_inumber[1]);
					inode_lock_disable(vector_inumber[0]);
					/* wait for a random number of seconds before trying to lock again*/
					sleep((rand() % (max + 1)));
				}
			}
			else {
				count ++;
				inode_lock_disable(vector_inumber[0]);
				/* wait for a random number of seconds before trying to lock again*/
				sleep((rand() % (max + 1)));
			}
		}
		else{
			count ++;
			/* wait for a random number of seconds before trying to lock again*/
			sleep((rand() % (max + 1)));
		}
	}

	/* removes the current child from the parent in the current pathname*/
	if (dir_reset_entry(current_parent_inumber, child_inumber) == FAIL) {
		printf("Moving: %s to %s\n", current_pathname_copy, new_pathname_copy);
		printf("failed to delete %s from dir %s\n",
				current_child_name, current_parent_name);

		disable_locks(vector_inumber, 3);
		return FAIL;
	}

	/* adds the current child to the parent in the new pathname with the new name*/
	if (dir_add_entry(new_parent_inumber, child_inumber, new_child_name) == FAIL) {
		printf("Moving: %s to %s\n", current_pathname_copy, new_pathname_copy);
		printf("could not add entry %s in dir %s\n",
				current_child_name, new_parent_name);

		disable_locks(vector_inumber, 3);
		return FAIL;
	}

	printf("Moving: %s to %s\n", current_pathname_copy, new_pathname_copy);
	disable_locks(vector_inumber, 3);
	return SUCCESS;
}


/* File opening with NULL checker */
FILE* openFile(char* name, char* mode) {
    FILE* fp = fopen(name, mode);

    if (fp == NULL) {
        fprintf(stderr, "Error: could not open file\n");
        exit(TECNICOFS_ERROR_FILE_NOT_FOUND);
    }

    return fp;
}

/*
 * Prints tecnicofs tree.
 * Input:
 *  - fp: pointer to output file
 */
void print_tecnicofs_tree(FILE *fp){
	inode_print_tree(fp, FS_ROOT, "");
}

int print(char *name){
    FILE *output = openFile(name, "w");

    print_tecnicofs_tree(output);
    printf("Print tree to: %s\n", name);

    fclose(output);

    return 0;
}
