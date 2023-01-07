#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extratypes.h"
#include "extrafuns.h"

void push_word(Actionnode* actions, int* map, char* changed, Wordnode word) {
    Actionnode newnode = malloc(sizeof(Action));
    newnode->map = map;
    newnode->changed = changed;
    newnode->wordnode = word;
    newnode->prev = *actions;
    *actions = newnode;
}

void pop_word(Actionnode* actions, int** map, char** changed, Wordnode* word) {
    if(map) *map = (*actions)->map;
    if(changed) *changed = (*actions)->changed;
    if(word) *word = (*actions)->wordnode;
    Actionnode to_free = (*actions);
    *actions = (*actions)->prev;
    free(to_free);
}