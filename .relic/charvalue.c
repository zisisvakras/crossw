#include <stdio.h>
#include <string.h>

int values[26][2];

void ssort(int words[26][2], int size) {
    int temp1, temp2;
    for (int i = 0 ; i < size ; i++) {
        for (int j = i + 1 ; j < size ; j++) {
            if (words[i][0] > words[j][0]) {
                temp1 = words[i][0];
                temp2 = words[i][1];
                words[i][0] = words[j][0];
                words[i][1] = words[j][1];
                words[j][0] = temp1;
                words[j][1] = temp2;
            }
        }
    }
}

int main(void) {
    FILE* dict = fopen("data/Words.txt", "r");
    
    char buffer[81];
    while (fscanf(dict, "%80s", buffer) == 1) {
        int len = strlen(buffer);
        for (int i = 0 ; i < len ; i++) {
            values[buffer[i] - 'a'][0]++;
        }
    }
    for (int i = 0 ; i < 26 ; i++) values[i][1] = i + 'a';
    ssort(values, 26);
    for (int i = 0 ; i < 26 ; i++) printf("%c: %d\n", values[i][1], values[i][0]);

    fclose(dict);
}