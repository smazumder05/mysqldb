#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

/*  CLI for mysqlite */
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


/** SQL statement processor, VM. */
typedef enum {
    METADATA_CMD_SUCCESS,
    METADATA_CMD_UNRECOGNIZED
} MetaCommandResult;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
    EXECUTE_DUPLICATE_KEY
} ExecuteResult;

typedef enum {
    STATEMENT_SELECT,
    STATEMENT_INSERT
} StatementType;


// Hardcoded table for now
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 32
typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
}Row;
//*******************************
typedef struct {
    StatementType type;
    Row row_to_insert;
} Statement;

/**
 *  Create a compact representation of a Row
 */
#define  size_of_attribute(Struct,Attribute) sizeof(((Struct*)0)->Attribute)
const uint32_t ID_SIZE = size_of_attribute(Row,id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row,username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row,email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET =  ID_SIZE + ID_OFFSET;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;


/**
 *  A serialization and deserialization method for each row.
 */
void serialize_row(Row* source, void* destination) {
    memcpy(destination + ID_OFFSET,&(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET,&(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET,&(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->id), source + EMAIL_OFFSET, EMAIL_SIZE);
}

// Next a table structure that contains pages of max row size.
// Page size  is 4Kb the same as OS page size, so that it will be easier
// to move pages in and out of memory with less fragmentation.
// Rows should not cross page boundaries since pages may not exist next
// to each other.

const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES  100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS =  ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
    uint32_t  row_counter;
    void*  pages[TABLE_MAX_PAGES];
} Table;

/**
 * This method figures out where to read and write in memory, in order to
 * insert or read a row from a page.
 */
void* row_slot(Table* tbl, uint32_t row_num){
    uint32_t pagenum = row_num / ROWS_PER_PAGE;
    void* page = tbl->pages[pagenum];
    if (page == NULL) {
        //Allocate memory only when trying to access a page
        page = tbl->pages[pagenum] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset =  row_num %  ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;

    return page + byte_offset;
}

/**
 * Initialization and free methods for a table.
 */
Table*  new_table(){
    Table* table = malloc(sizeof(Table));
    table->row_counter = 0;
    for (int i=0; i< TABLE_MAX_PAGES;i++){
        table->pages[i] = NULL;
    }
    return table;
}

void free_table(Table* tbl) {
    for (int i=0;tbl->pages[i];i++) {
        free(tbl->pages[i]);
    }
    free(tbl);
}
/**
 * Prepare statement accepts INSERT and SELECT. It parses the arguments of insert where tablename
 * is hardcoded for now as
 *  insert id name email
 *  select  - just returns all rows in the hardcoded table
 */
PrepareResult prepare_statement(Inputbuffer* input_buffer, Statement* statement) {
    if (strncmp(input_buffer->buffer, "insert",6) == 0) {
        statement->type = STATEMENT_INSERT;
        int  insert_args = sscanf(input_buffer->buffer, "insert %d %s %s",
                                                                        &statement->row_to_insert.id,
                                                                        &statement->row_to_insert.username,
                                                                        &statement->row_to_insert.email);
        if (insert_args < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
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
ExecuteResult execute_insert(Statement* statement, Table* table){
    if (table->row_counter > TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    Row* row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert,row_slot(table,table->row_counter));
    table->row_counter += 1;
    return EXECUTE_SUCCESS;

}
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

/**
 * Clean up before exit command
 */
MetaCommandResult exec_meta_data_cmd(Inputbuffer* ibuffer,Table* table){
    if(strcmp(ibuffer->buffer,".exit") == 0) {
        close_buffer(ibuffer);
        free_table(table);
        exit(EXIT_SUCCESS);
    } else
        return METADATA_CMD_UNRECOGNIZED;
}

/**
 * Main for db engine.
 */
int main(int argc, char *argv[] ) {
    Inputbuffer*   mysql_buffer = create_new_buffer();
    Table* table = new_table();
    while(true) {
        print_prompt();
        read_input(mysql_buffer);

        if(mysql_buffer->buffer[0] == '.') {
            switch(exec_meta_data_cmd(mysql_buffer,table)) {
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
            case (PREPARE_SYNTAX_ERROR):
                  printf("Syntax error: '%s'.\n",mysql_buffer->buffer);
                  continue;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                 printf("Unrecognized command: %s\n", mysql_buffer->buffer);
                 continue;
        }
        execute_statement(&statement);
        printf("Executed statement.\n");
    }
}
