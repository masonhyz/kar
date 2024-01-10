/*
** This file is part of the CSC209 -- Fall 2023 Assignment 2
**
** All of the files in this directory and all subdirectories are:
** Copyright (c) 2023 Demetres Kostas
*/

#include "kar_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_tree(arch_tree_node *root) {
    if (root == NULL) {
    } else {
        free_tree(root->dir_contents);
        free_tree(root->next_file);
        free(root);
    }
}


/*
** Some helper functions so that you don't get
** bogged down by formatting paths correctly.
*/
void only_filename(char* filepath){
    int total_chars = strlen(filepath);

    if(filepath[total_chars - 1] == '/'){
        filepath[total_chars - 1] = '\0';
    }

    char* filename = strrchr(filepath, '/');
    for(size_t i = 0; filename && i <= strlen(filename); i++){
        filepath[i] = filename[i + 1];
    }
}

// WARNING: This function assumes that path_buffer is large enough
void valid_path(char path_buffer[], char *directory, char *filename){
    strcpy(path_buffer, directory);

    // If the directory doesn't end with a slash, add one
    if (strlen(path_buffer) > 0){
        if (path_buffer[strlen(path_buffer) - 1] != '/'){
            strcat(path_buffer, "/");
        }
    }
    strcat(path_buffer, filename);
}

// WARNING
int buffered_read_write(FILE *input, FILE *output, size_t read_remaining, size_t buffer_size){
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        perror("Failed to allocate memory for buffer");
        return -1;
    }
    // printf("Reading %zu bytes\n", read_remaining);

    size_t bytes_read;
    size_t next_read_size = buffer_size < read_remaining ? buffer_size : read_remaining;

    while (read_remaining > 0 && (bytes_read = fread(buffer, 1, next_read_size, input)) > 0) {
        // printf("Read %zu bytes\n", bytes_read);
        int to_write = bytes_read;

        char *buf_pt = buffer;
        while(to_write > 0){
            size_t num_written = fwrite(buf_pt, 1, to_write, output);
            // printf("Wrote %zu bytes\n", num_written);
            to_write -= num_written;
            buf_pt += num_written;
        }
        read_remaining -= bytes_read;
        next_read_size = buffer_size < read_remaining ? buffer_size : read_remaining;
    }
    free(buffer);
    return 0;
}
