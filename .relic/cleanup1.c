
//TODO after finished project check if filters are getting freed
char* create_filter(char** crossword, Word word) {
    int flag = word.orientation;
    int filter_size = word.end - word.begin + 1;
    char* filter = malloc((filter_size + 1) * sizeof(char));
    if (filter == NULL) { /* Malloc error handling */
        fprintf(stderr, "Error while allocating memory: %s", strerror(errno));
        exit(1);
    }

    if (flag) {
        for (int i = word.begin, j = 0 ; i <= word.end ; i++, j++) {
            char temp = crossword[i][word.constant];
            filter[j] = temp == '-' ? '?' : temp;
        }
    }
    else {
        for (int i = word.begin, j = 0 ; i <= word.end ; i++, j++) {
            char temp = crossword[word.constant][i];
            filter[j] = temp == '-' ? '?' : temp;
        }
    }
    filter[filter_size] = '\0';
    return filter;
}

/**
 * @deprecated
*/
int* create_map(Bitmaps maps, int* map_sizes, char* filter) {
    int word_size = strlen(filter);
    int map_size = map_sizes[word_size - 1];
    int* map = malloc(map_size * sizeof(int));
    memcpy(map, maps[word_size - 1][word_size][0], map_size * sizeof(int));
    for (int i = 0 ; i < word_size ; ++i) {
        if (filter[i] != '?') {
            join_map(map, maps[word_size - 1][i][filter[i] - 'a'], map_size);
        }
    }
    return map;
}


void delete_word(char** crossword, Word node, char* word) {
    if (node.orientation) { /* Vertical */
        for (int i = node.begin, j = 0 ; i <= node.end ; i++, j++) {
            if (word[j] != '?') crossword[i][node.constant] = '-';
        }
    }
    else { /* Horizontal */
        for (int i = node.begin, j = 0 ; i <= node.end ; i++, j++) {
            if (word[j] != '?') crossword[node.constant][i] = '-';
        }
    }
}

char* word_written(char* word, char* filter) {
    int size = strlen(word);
    char* written = malloc((size + 1) * sizeof(char));
    for (int i = 0 ; i < size ; i++) {
        written[i] = filter[i] == '?' ? word[i] : '?';
    }
    written[size] = '\0';
    return written;
}