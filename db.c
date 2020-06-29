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

typedef enum {
    METADATA_CMD_SUCCESS,
    METADATA_CMD_UNRECOGNIZED
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
    STATEMENT_SELECT,
    STATEMENT_INSERT
} StatementType;

typedef  struct {
    StatementType type;
} Statement;


void print_prompt() {
    printf("mysqlite-db > ");
}

/**
 * Processes input from the CLI
 */
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

PrepareResult prepare_statement(Inputbuffer* input_buffer, Statement* statement) {
    if (strncmp(input_buffer->buffer, "insert",6) == 0) {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    if (strncmp(input_buffer->buffer, "select",6) == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return  PREPARE_UNRECOGNIZED_STATEMENT;
}

/**
 * The virtual machine for the simple database.
 */
void execute_statement(Statement* statement) {
    switch(statement->type) {
        case STATEMENT_INSERT:
            printf("[SQL-VM]: will execute an insert statement. \n");
            break;
        case STATEMENT_SELECT:
            printf("[SQL-VM]: will execute a select statement. \n");
            break;
    }
}

MetaCommandResult exec_meta_data_cmd(Inputbuffer* ibuffer){
    if(strcmp(ibuffer->buffer,".exit") == 0) {
        close_buffer(ibuffer);
        exit(EXIT_SUCCESS);
    } else
        return METADATA_CMD_UNRECOGNIZED;
}

int main(int argc, char *argv[] ) {
    Inputbuffer*   mysql_buffer = create_new_buffer();
    while(true) {
        print_prompt();
        read_input(mysql_buffer);

        if(mysql_buffer->buffer[0] == '.') {
            switch(exec_meta_data_cmd(mysql_buffer)) {
                case (METADATA_CMD_SUCCESS):
                    continue;
                case (METADATA_CMD_UNRECOGNIZED):
                    printf("Unrecognized command: %s\n", mysql_buffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch(prepare_statement(mysql_buffer,&statement)) {
            case (PREPARE_SUCCESS):
                  break;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                 printf("Unrecognized command: %s\n", mysql_buffer->buffer);
                 continue;
        }
        execute_statement(&statement);
        printf("Executed statement.\n");
    }
}
