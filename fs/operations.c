#include "operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void initialize_vector(int vector[]) {
	for (int i = INODE_TABLE_SIZE - 1; i >= 0; i--) {
		vector[i] = FREE_INODE;
	}
}

void disable_locks(int vector[]) {
	for (int i = INODE_TABLE_SIZE - 1; i >= 0; i--) {
		if (vector[i] != FREE_INODE) {
			//////printf("Trying to unlock %d\n", i);
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

	initialize_vector(vector_inumber);

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

		disable_locks(vector_inumber);		
		return FAIL;
	}

	if (lookup_sub_node(child_name, pdata.dirEntries) != FAIL) {
		display_create(name, nodeType);
		printf("failed to create %s, already exists in dir %s\n",
				child_name, parent_name);

		disable_locks(vector_inumber);  
		return FAIL;
	}

	/* create node and add entry to folder that contains new node */
	child_inumber = inode_create(nodeType);

	if (child_inumber == FAIL) {
		display_create(name, nodeType);
		printf("failed to create %s in  %s, couldn't allocate inode\n",
				child_name, parent_name);

		disable_locks(vector_inumber);		
		return FAIL;
	}

	inode_lock_enable(child_inumber, 'w');
	vector_inumber[i] = child_inumber;

	if (dir_add_entry(parent_inumber, child_inumber, child_name) == FAIL) {
		display_create(name, nodeType);
		printf("could not add entry %s in dir %s\n",
				child_name, parent_name);

		disable_locks(vector_inumber);
		return FAIL;
	}

	disable_locks(vector_inumber);
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

	initialize_vector(vector_inumber);

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

		disable_locks(vector_inumber);	  		
		return FAIL;
	}

	child_inumber = lookup_sub_node(child_name, pdata.dirEntries);

	if (child_inumber == FAIL) {
		printf("Delete: %s\n", name);
		printf("could not delete %s, does not exist in dir %s\n",
				name, parent_name);

		disable_locks(vector_inumber);	
		return FAIL;
	}

	inode_lock_enable(child_inumber, 'w');
	vector_inumber[i++] = child_inumber;
	inode_get(child_inumber, &cType, &cdata);

	if (cType == T_DIRECTORY && is_dir_empty(cdata.dirEntries) == FAIL) {
		printf("Delete: %s\n", name);
		printf("could not delete %s: is a directory and not empty\n",
				name);

		disable_locks(vector_inumber);
		return FAIL;
	}

	/* remove entry from folder that contained deleted node */
	if (dir_reset_entry(parent_inumber, child_inumber) == FAIL) {
		printf("Delete: %s\n", name);
		printf("failed to delete %s from dir %s\n",
				child_name, parent_name);

		disable_locks(vector_inumber);
		return FAIL;
	}

	if (inode_delete(child_inumber) == FAIL) {
		printf("Delete: %s\n", name);
		printf("could not delete inode number %d from dir %s\n",
				child_inumber, parent_name);

		vector_inumber[child_inumber] = FREE_INODE;
		disable_locks(vector_inumber);
		return FAIL;
	}

	disable_locks(vector_inumber);
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

	initialize_vector(vector_inumber);

	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	//////printf("Trying to lock %d lookup\n", current_inumber);
	inode_lock_enable(current_inumber, 'r');
	vector_inumber[i++] = current_inumber;

	/* get root inode data */
	inode_get(current_inumber, &nType, &data);

	char *path = strtok_r(full_path, delim, &saveptr);

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL) {
		//////printf("Trying to lock %d lookup\n", current_inumber);
		inode_lock_enable(current_inumber, 'r');
		vector_inumber[i++] = current_inumber;

		inode_get(current_inumber, &nType, &data);
		path = strtok_r(NULL, delim, &saveptr);
	}

	disable_locks(vector_inumber);
	return current_inumber;
}


int move(char* current_pathname, char* new_pathname) {
	int vector_inumber[INODE_TABLE_SIZE];
	int i = 0, flag = 0;

	int current_parent_inumber, child_inumber, new_parent_inumber;	
	char *current_parent_name, *current_child_name;
	char *new_parent_name, *new_child_name;
	char current_pathname_copy[MAX_FILE_NAME];
	char new_pathname_copy[MAX_FILE_NAME];

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
	
	while (!flag) {
		if (inode_lock_enable(current_parent_inumber, 'w')) {
			if (inode_lock_try(current_parent_inumber, 'w')) {
				vector_inumber[i++] = current_parent_inumber;
				flag = 1;
			}
			else {
				inode_lock_disable(current_parent_inumber);
				sleep(rand());
			}
		}
	}
	//inode_lock_enable(current_parent_inumber, 'w');

	/* Example: "m /a /a/a". Prevent loops */
	if (!strcmp(current_pathname_copy, new_parent_name)) {
		printf("Moving: %s to %s\n", current_pathname_copy, new_pathname_copy);
		printf("failed to move %s to %s, loop would occur\n",
				current_pathname, new_pathname);

		disable_locks(vector_inumber);
		return FAIL;
	}

	/* removes the current child from the parent in the current pathname*/
	if (dir_reset_entry(current_parent_inumber, child_inumber) == FAIL) {
		printf("Moving: %s to %s\n", current_pathname_copy, new_pathname_copy);
		printf("failed to delete %s from dir %s\n",
				current_child_name, current_parent_name);

		disable_locks(vector_inumber);
		return FAIL;
	}
	
	flag = 0;
	while (!flag) {
		if (inode_lock_enable(new_parent_inumber, 'w')) {
			if (inode_lock_try(new_parent_inumber, 'w')) {
				vector_inumber[i++] =(new_parent_inumber);
				flag = 1;
			}
			else {
				inode_lock_disable(new_parent_inumber);
				sleep(rand());
			}
		}
	}
	//inode_lock_enable(new_parent_inumber, 'w');
	//vector_inumber[i++] = new_parent_inumber;
	flag = 0;
	while (!flag) {
		if (inode_lock_enable(child_inumber, 'w')) {
			if (inode_lock_try(child_inumber, 'w')) {
				vector_inumber[i++] = child_inumber;
				flag = 1;
			}
			else {
				inode_lock_disable(child_inumber);
				sleep(rand());
			}
		}
	}
	//inode_lock_enable(child_inumber, 'w');
	//vector_inumber[i++] = child_inumber;

	/* adds the current child to the parent in the new pathname with the new name*/
	if (dir_add_entry(new_parent_inumber, child_inumber, new_child_name) == FAIL) {
		printf("Moving: %s to %s\n", current_pathname_copy, new_pathname_copy);
		printf("could not add entry %s in dir %s\n",
				current_child_name, new_parent_name);

		disable_locks(vector_inumber);
		return FAIL;
	}

	printf("Moving: %s to %s\n", current_pathname_copy, new_pathname_copy);
	disable_locks(vector_inumber);
	return SUCCESS;
}


/*
 * Prints tecnicofs tree.
 * Input:
 *  - fp: pointer to output file
 */
void print_tecnicofs_tree(FILE *fp){
	inode_print_tree(fp, FS_ROOT, "");
}
