#include "assembler.h"

/** for debug **/
void print_data_list()
{
    img_node *curr = g_data_head;
    while(curr != NULL)
    {
        printf("data: %d | address: %d\n",curr->data, curr->address);
        curr = curr->next_node;
    }
}
/** for debug **/
void print_line_nodes(line_node *line_list_head)
{
    char **tokenz;
    int i;
    line_node *curr_node = line_list_head;

    while(curr_node != NULL)
    {
        printf("line:%d ; ",curr_node->line_num);
        tokenz = curr_node->tokenz;
        for (i=0;curr_node->num_tokenz > i;i++ )
        {
            printf("[%s] ", tokenz[i]);
        }
        printf("\n");

        curr_node = curr_node->next_node;
    }
}
/** for debug **/
void print_curr_line_node(line_node *curr_node)
{
    char **tokenz;
    int i;

    printf("line:%d | ",curr_node->line_num);
    tokenz = curr_node->tokenz;
    for (i=0;curr_node->num_tokenz > i;i++ )
    {
        printf("[%s] ", tokenz[i]);
    }
    printf(" | num_tokenz:%d  ",curr_node->num_tokenz);
    printf(" | label_flag:%d  ",curr_node->label_flag);
    printf(" | error_flag:%d  ",curr_node->error_flag);

    printf("\n");
}
/** for debug **/
void print_curr_label_node(label_node *curr_node)
{
    printf("address:%d ",curr_node->address);
    printf(" | dirc_type:%s  ",curr_node->dirc_type);
    printf(" | label:%s  ",curr_node->label);

    printf("\n");
}
/********************************************//**
 * \brief here we pars data of directive line(string\data)
 *
 * \return Nonde
 ***********************************************/
void pars_data(line_node *curr_line_node, int *DC)
{
    char *token_p = curr_line_node->tokenz[curr_line_node->tok_idx];

    if (strstr(token_p, ".string"))
    {
        curr_line_node->tok_idx++;
        insert_string2data_list(DC, curr_line_node);
    }
    else if (strstr(token_p, ".data"))
    {
        curr_line_node->tok_idx++;
        insert_int2data_list(DC, curr_line_node);
    }
}
/********************************************//**
 * \brief creates a label struct and initializes
 *
 * \return None
 ***********************************************/

void insert_label(line_node *curr_line_node, char *type, int DC, bool extern_entry)
{
    label_node *curr_label_node = curr_line_node->label;
    char *label_str;
    int tok_idx = curr_line_node->tok_idx;

    curr_label_node->address = DC;
    if (!extern_entry)
        label_str = get_label(curr_line_node);
    else
    {
        label_str = curr_line_node->tokenz[++tok_idx];
    }
    curr_label_node->label = label_str;
    curr_label_node->dirc_type = type;

    curr_line_node->tok_idx++;
}

/********************************************//**
 * \brief takes a set of tokens and creates, initializes line node with tokens set
 *
 * \return the newly created line node
 ***********************************************/
line_node *insert_set2line_list(line_node **line_node_head, int *line_count, char ***token_set, int *token_count)
{
    line_node *new_line_node;

    new_line_node = create_line_node(line_node_head);

    new_line_node->num_tokenz   = (*token_count);
    new_line_node->line_num     = (*line_count);
    new_line_node->tokenz       = (*token_set);
    new_line_node->tok_idx      = 0;

    return new_line_node;
}

/********************************************//**
 * \brief insert tokens into set, by allocating array of chars, copying token into it,
 *        reallocating token_set to new memory with the new token
 * \return None
 *
 ***********************************************/
void insert_token2set(char *token, char ***token_set, int *tok_count, int *tok_set_size)
{
    char **token_set_new;
    char *token_new = malloc(sizeof(char) * ((strlen(token) + 1)));;

    if (!token_new)
    {
        printf("[ERROR] Failed to allocate memory for token_new.\n");
        return;
    }
    strcpy(token_new, token);

    if (*tok_count >= (*tok_set_size))
    {
        token_set_new = realloc(*token_set, sizeof(*token_set_new) * (*tok_count + 1));
        if (!token_set_new)
        {
            printf("[ERROR] Failed to reallocate memory for token_set.\n");
            return;
        }
        *token_set = token_set_new;
    }

    (*token_set)[*tok_count] = token_new;
    (*tok_set_size)++;
}

/********************************************//**
 * \brief converts string of char to tokens and insert into set(array strings)
 *
 * \param token_set - array of char strings
 * \return None
 *
 ***********************************************/
void tokenize_line(char ***token_set, char *tmp_line, int *tok_count)
{
    char *token;
    char *delims = " \t\n";
    int tok_set_size=0;

    *tok_count=0;

    token = strtok(tmp_line, delims);

    while (token != NULL)
    {
        insert_token2set(token, token_set, tok_count, &tok_set_size);
        token = strtok(NULL, delims);
        (*tok_count)++;
    }
}

/********************************************//**
 * \brief check if a line of assembly code is a comment or an empty line
 *
 * \return TRUE if it is else FALSE
 ***********************************************/

bool is_comment_empty(char *tmp_line, int *line_count)
{
    char *str_ptr;

    str_ptr = tmp_line;

    if (*str_ptr == ';')
    {
        return TRUE;
    }

    while(*str_ptr != '\n')
    {
        if (*str_ptr == ';')
        {
            print_error(*line_count, "Comment line should start with \';\'.");
            return TRUE;
        }

        if (*str_ptr != '\t' && *str_ptr != ' ')
        {
            return FALSE;
        }
        str_ptr++;
    }
    return TRUE;
}

/********************************************//**
 * \brief check if line is a empty or comment and ignore it.
 *        convert line to tokens and store in array of tokens(token_set).
 *        free token_set
 *
 *
 * \param tmp_line - pointer to line in the file.
 * \return None
 *
 ***********************************************/
void line_parser(line_node **line_list_head, char *tmp_line, int *line_count, int *error_count, int *IC, int *DC)
{
    char **token_set = NULL;
    int token_count = 0;
    line_node *curr_line_node;

    if(is_comment_empty(tmp_line, line_count))
    {
        return;
    }

    tokenize_line(&token_set, tmp_line, &token_count);
    curr_line_node = insert_set2line_list(line_list_head, line_count, &token_set, &token_count);

    if (is_label_decleration(curr_line_node))
    {
        curr_line_node->label_flag = TRUE;
    }


    switch (dirc_type(curr_line_node))
    {
        case DATA_STRING_DIRC:
            if(curr_line_node->label_flag)
            {
                create_label_node(curr_line_node);
                insert_label(curr_line_node, "data", *DC, FALSE);

                    /* debug */
                printf("\n");
                print_curr_label_node(curr_line_node->label);
                printf("\n");
            }

            pars_data(curr_line_node, DC);

            /* debug */
            printf("\n");
            print_curr_line_node(curr_line_node);
            printf("\n");

            *error_count =+ curr_line_node->error_flag;
            return;

        case ENTRY_DIRC:
            return;

        case EXTERN_DIRC:
            create_label_node(curr_line_node);
            insert_label(curr_line_node, "extern", 0, TRUE);

                        /* debug */
            printf("\n");
            print_curr_line_node(curr_line_node);
            print_curr_label_node(curr_line_node->label);
            printf("\n");
            return;

        case COMMAND_LINE:
            break;

    }

        /*  debug   */
    print_curr_line_node(curr_line_node);
}



/********************************************//**
 * \brief read_line - reads a line from file of max length, replacing new line char with null char
 *       if line is larger then max len keep reading until new line is found or eof
 *
 * \param file - pointer to file
 * \param tmp_line - pointer to string read from a file
 * \param max_len - maximum length of line to read
 * \return TRUE if read line was successful, otherwise FALSE
 *
 ***********************************************/
bool read_line(FILE *file, char *tmp_line, int max_len)
{

    char *end_of_line;

    if (!fgets(tmp_line, max_len, file))
    {
        return FALSE;
    }

    end_of_line = strchr(tmp_line, '\n');

    if (!end_of_line)
    {
        char c;
        while ((c=fgetc(file)))
        {
            if (c == '\n' || c == EOF)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}


/********************************************//**
 * \brief first_read - organizes all the actions for the first pass described by the project booklet
 *
 * \param file - pointer to file
 * \param line_list_head - pointer to the start of linked list of type line_node
 *
 * \return no value returned
 *
 ***********************************************/
void first_read(FILE *file, line_node **line_list_head, int *error_count, int *line_count, int *IC, int *DC)
{
    char tmp_line[MAX_LINE_LEN];

    while(!feof(file))
    {
        if(read_line(file, tmp_line, MAX_LINE_LEN))
        {
            (*line_count)++;

            line_parser(line_list_head, tmp_line, line_count, error_count, IC, DC);

        }
        else if(!feof(file))
        {
            printf("[ERROR] Line %d is too long, max size of line:%d.\n", *line_count, MAX_LINE_LEN);
            ++(*line_count);
            ++(*error_count);
        }
    }
}
