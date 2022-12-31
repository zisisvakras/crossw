unsigned long hash(unsigned char *str);
int dict_length(Dictnode* dictionary, int max_word_size);
char** make_dict_hash(char* dictionary_path, int max_word_size, int length);

/* djb2 algorithm http://www.cse.yorku.ca/~oz/hash.html */
unsigned long hash(unsigned char *str, int size) {
    unsigned long hash = 5381;
    for (int i = 0 ; i < size ; i++) {
        hash = ((hash << 5) + hash) + str[i]; /* hash * 33 + c */
    }
    return hash;
}

//TODO review comments
char** make_dict_hash(char* dictionary_path, int max_word_size, int length) {

    FILE* dictionary_file = fopen(dictionary_path, "r");

    if (dictionary_file == NULL) { /* File error handling */
        fprintf(stderr, "Error while handling dictionary: %s", strerror(errno));
        return NULL;
    }

    /* Allocate enough linked lists for all needed word sizes */
    char** dictionary_hash = malloc(length * sizeof(char*));
    if (dictionary_hash == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return NULL;
    }

    char* buffer = malloc(81 * sizeof(char)); /* Hopefully no word will be larger than 80 chars long */
    if (buffer == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        return NULL;
    }
    while (fscanf(dictionary_file, "%80s", buffer) == 1) { /* Scan 1 word at a time */
        int word_size = strlen(buffer);
        if (word_size > max_word_size) continue; /* No need to allocate larger words than needed */
        int hash_value = hash(buffer, word_size) % length;
        if (dictionary_hash[hash_value]) {
            while (dictionary_hash[0]) {
                
            }
        }
        else {
            dictionary_hash[hash_value] = malloc(word_size * sizeof(char));
            strcpy(dictionary_hash[hash_value], buffer);
        }
    }
    
    fclose(dictionary_file);
    free(buffer);
    return dictionary_hash;
}

int dict_length(Dictnode* dictionary, int max_word_size) {
    int count = 0;
    for (int i = 0 ; i < max_word_size ; i++) { 
        Dictnode node = dictionary[i];
        char* word;
        while ((word = node->word) != NULL) {
            count++;
            node = node->next;
        }
    }
    return count;
}