/*
** This file is part of the CSC209 -- Fall 2023 Assignment 2
**
** All of the files in this directory and all subdirectories are:
** Copyright (c) 2023 Demetres Kostas
*/

#include "kar_tree.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


// This function is provided to help you get started
// You are not required to use it.
arch_tree_node *read_node(FILE *archive){
    arch_tree_node *node = malloc(sizeof(arch_tree_node));

    // Initialize the pointer values to NULL
    (node->name)[0] = '\0';
    node->dir_contents = NULL;
    node->next_file = NULL;

    if (!node) {
        perror("Failed to allocate memory for node");
        return NULL;
    }
    fread(node, sizeof(arch_tree_node), 1, archive);
    return node;
}


int extract_archive(char *archive_name){
    FILE* archive = fopen(archive_name, "r");
    if (!archive) {
        perror("Failed to open archive.\n");
        return 1;
    }

    kar_tree root;
    arch_tree_node **current = &root.root;
    // Read the data structures from the archive
    build_from_archive(archive, current);
    // Close the file
    fclose(archive);

    FILE* archive_1 = fopen(archive_name, "r");
    if (!archive_1) {
        perror("Failed to open archive.\n");
        return 1;
    }
    // Such that you can re-create the archived files
    re_create(archive_1, root.root, "./");
    // Close the file
    fclose(archive_1);

    // Free tree
    free_tree(root.root);
    return 0;
}


// Helper function to recursively build the tree (the pointer to pointer to node) from the given archive file
void build_from_archive(FILE *archive, arch_tree_node **node) {
    *node = read_node(archive);

    // Prioritize building the next dir_contents subtree if dir_contents pointer is not null
    if ((*node)->dir_contents != NULL) {
        build_from_archive(archive, &((*node)->dir_contents));
    } 

    // Skip the contents of the file
    if ((*node)->is_directory == 0) {
        fseek(archive, (*node)->size, SEEK_CUR);
    } 

    // Build the next_file subtree once done with dir_contents, preserving the format recursively
    if ((*node)->next_file != NULL) {
        build_from_archive(archive, &((*node)->next_file));
    } 
}


// Recursive helper function to recreate the files from the tree given archive and root path
// The root path is necessary because we need to recursively call the function in subdirectories
void re_create(FILE *archive, arch_tree_node *node, char *root_path) {
    while (node != NULL) {

        // Skip the space of a node struct
        fseek(archive, sizeof(arch_tree_node), SEEK_CUR);

        // Read the archive at the current offset and create the file in the according directory
        if (node->is_directory == 0) {
            char path[FILENAME_MAX];
            valid_path(path, root_path, node->name);
            FILE *reg_file = fopen(path, "w");
            if (!archive) {
                printf("Failed to open %s.\n", path); fflush(stdout);
            }
            buffered_read_write(archive, reg_file, node->size, WRITE_BUFFER_SIZE);
            // char content[node->size];
            // fread(content, 1, node->size, archive);
            // fwrite(content, 1, node->size, reg_file);
            fclose(reg_file);

        // Recursively re-create on the subdirectory
        } else {
            char path[FILENAME_MAX];
            valid_path(path, root_path, node->name);
            mkdir(path, 750);

            // Call the function recursively on the subdirectory path
            re_create(archive, node->dir_contents, path);
        } 
        node = node->next_file;
    }
}
