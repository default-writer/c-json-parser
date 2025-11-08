#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern bool func(const char* json);

int main(void) {
    const char* filename = "test/test.json";
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* json = malloc(size + 1);
    if (!json) {
        fprintf(stderr, "malloc failed\n");
        fclose(fp);
        return 1;
    }

    fread(json, 1, size, fp);
    json[size] = '\0';
    fclose(fp);

    if (func(json)) {
        printf("JSON is valid!\n");
    } else {
        printf("JSON is invalid!\n");
    }

    free(json);
    return 0;
}