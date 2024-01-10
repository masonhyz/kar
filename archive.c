/*
** This file is part of the CSC209 -- Fall 2023 Assignment 2
**
** All of the files in this directory and all subdirectories are:
** Copyright (c) 2023 Demetres Kostas
*/

#include "kar_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

// void test_create(arch_tree_node *atn) {
//     printf("name: %s\n", atn->name);
//     printf("is directory: %d\n", atn->is_directory);
//     if (atn->is_directory != 0) {
//         if (atn->dir_contents != NULL) {
//             printf("directory %s start!!!\n", atn->name);
//             test_create(atn->dir_contents);
//             printf("directory %s ends!!\n", atn->name);
//         } 
//     } else {
//         printf("size: %d\n", atn->size);
//     }
//     if (atn->next_file != NULL) {
//         test_create(atn->next_file);
//     }
// }

// void test_create(arch_tree_node *atn, int indent) {
//     for (int i = 0; i < indent; i++) {
//         printf("    "); 
//     }
//     printf("name: %s\n", atn->name);
//     for (int i = 0; i < indent; i++) {
//         printf("    ");
//     }
//     printf("is directory: %d\n", atn->is_directory);
//     if (atn->is_directory != 0) {
//         if (atn->dir_contents != NULL) {
//             // for (int i = 0; i < indent; i++) {
//             //     printf("    "); 
//             // }
//             // printf("directory %s start!!!\n", atn->name);

//             // Increase indentation
//             test_create(atn->dir_contents, indent + 1); 
//             // for (int i = 0; i < indent; i++) {
//             //     printf("    ");
//             // }
//             // printf("directory %s ends!!\n", atn->name);
//         } 
//     } else {
//         for (int i = 0; i < indent; i++) {
//             printf("    ");
//         }
//         printf("size: %d\n", atn->size);
//     }
//     if (atn->next_file != NULL) {
//         // Same indentation
//         test_create(atn->next_file, indent); 
//     }
// }


arch_tree_node* create_tree_node(char *filepath){
    // Get the file's metadata
    struct stat st;
    if (stat(filepath, &st) == -1) {
        perror("Failed to get file metadata");
        return NULL;
    }

    // Create the node
    char fullpath[FILENAME_MAX];
    strcpy(fullpath, filepath);
    only_filename(filepath);
    arch_tree_node *atn = malloc(sizeof(arch_tree_node));

    // Initialize the pointer to NULL
    (atn->name)[0] = '\0';
    atn->next_file = NULL;
    atn->dir_contents = NULL;
    
    // Write the two non-null attributes
    strcpy(atn->name, filepath);
    atn->is_directory = S_ISDIR(st.st_mode);

    // Create the dir_contents if dealing with a directory
    if (atn->is_directory != 0) {
        DIR *dir = opendir(fullpath);
        if (dir == NULL) {
            printf("Error opening directory %s", fullpath); fflush(stdout);
        }
        struct dirent *file_in_dir;
        // Initializing current to point to the address of the (null) dir_contents pointer
        arch_tree_node **current = &(atn->dir_contents);

        // Recursively create the linked list with starting node atn->dir_contents
        while (((file_in_dir = readdir(dir)) != NULL)) {
            // Get rid of "." and ".." directories
            if ((strcmp(file_in_dir->d_name, ".") != 0) && (strcmp(file_in_dir->d_name, "..") != 0)) {
                char path[FILENAME_MAX];
                valid_path(path, fullpath, file_in_dir->d_name);
                arch_tree_node *atn_in_dir = create_tree_node(path);
                // Setting the value current point to to be the pointer of the next tree node file
                *current = atn_in_dir;
                // Setting current to point to the address of the next file, which is atm a NULL pointer
                current = &((*current)->next_file);
            }
        }
        closedir(dir);

    // Record the size if dealing with regular files
    } else {
        atn->size = st.st_size;
    }

    // Return node;
    return atn;
}


int create_archive(char *archive_name, int num_files, char *files_to_add[num_files]){
    
    // Create the archive file
    FILE *archive = fopen(archive_name, "w");
    if (!archive) {
        perror("Failed to open archive file");
        return 1;
    }
    kar_tree root;
    arch_tree_node **current = &root.root;
    // TODO Build the tree and write the archive you may implement this as you see fit, but
    // you must use the structures given to you and build the data structure that we have asked

    // Link the outmost layer's files with their next_file pointers
    for (int i = 0; i < num_files; i++) {
        *current = create_tree_node(files_to_add[i]);
        current = &((*current)->next_file);
    } 
    // *current = create_tree_node(files_to_add[0]);
    // for (int i = 1; i < num_files; i++) {
    //     (*current)->next_file = create_tree_node(files_to_add[i]);
    //     current = &((*current)->next_file);
    // } 
    write_kar(archive, root.root, "./");
    fclose(archive);

    // Free the tree's root
    free_tree(root.root);
    return 0;
}


// Recursively write the kar file given a kar_file and node pointer
// The parent_dir pointer is necessary because we will be calling the function recursively but in different subdirectories
void write_kar(FILE *kar_file, arch_tree_node *node, char *parent_dir) {
    while (node != NULL) {
        fwrite(node, sizeof(arch_tree_node), 1, kar_file);
        if (node->is_directory != 0) {
            char path[FILENAME_MAX];
            valid_path(path, parent_dir, node->name);
            
            // Recursively write the subdirectories on the subdirectory path
            write_kar(kar_file, node->dir_contents, path);

        // Write the regular files accordingly
        } else {
            char path[FILENAME_MAX];
            valid_path(path, parent_dir, node->name);
            FILE *current_file = fopen(path, "r");
            if (current_file == NULL) {
                printf("Could not open file %s", path); fflush(stdout);
            }
            buffered_read_write(current_file, kar_file, node->size, WRITE_BUFFER_SIZE);
            fclose(current_file);
        }

        // Write the next file
        node = node->next_file;
    }
}
