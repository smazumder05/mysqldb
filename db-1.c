#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>


typedef struct {
    char* buffer;
    size_t buffer_length;
    size_t input_length;
} Inputbuffer;

Inputbuffer* create_new_buffer() {
    Inputbuffer* ibuffer = (Inputbuffer*)malloc(sizeof(Inputbuffer));
    ibuffer->buffer = NULL;
    ibuffer->buffer_length = 0;
    ibuffer->input_length = 0;
    return ibuffer;
}

void print_prompt() {
    printf("mysqlite-db > ");
}

void read_input(Inputbuffer* ibuffer) {
    ssize_t bytes_read = getline(&(ibuffer->buffer), &(ibuffer->buffer_length), stdin);
    if (bytes_read <= 0) {
        printf("Error reading input. \n");
        exit(EXIT_FAILURE);
    }
    ibuffer->buffer_length = bytes_read -1;
    ibuffer->buffer[bytes_read - 1] = 0;
}

void close_buffer(Inputbuffer* ibuffer) {
    free(ibuffer->buffer);
    free(ibuffer);
}

int main(int argc, char *argv[] ) {
    Inputbuffer*   mysql_buffer = create_new_buffer();
    while(true) {
        print_prompt();
        read_input(mysql_buffer);

        if (strcmp(mysql_buffer->buffer, ".exit") == 0) {
            close_buffer(mysql_buffer);
            exit(EXIT_SUCCESS);
        } else {
            printf("Unrecognized command '%s' .\n",mysql_buffer->buffer);
        }
    }
}
