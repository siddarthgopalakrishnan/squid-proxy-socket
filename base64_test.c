#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* base64Encoding(const char* str, int len) {
    char *finalstr = (char *)malloc(10000 * sizeof(char));
    char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int base64ind, numOfEquals = 0;
    int stringInd = 0;
    for(int i=0; i<len; i=i+3) {
        int count = 0, val = 0, no_of_bits = 0;
        for(int j = i; (j<len && j<=i+2); j++) {
            ++count;
            val = val << 8;
            val = val | str[j];
        }
        no_of_bits = count * 8;
        if(count != 3) numOfEquals = no_of_bits % 3;
        while(no_of_bits != 0) {
            if(no_of_bits >= 6) {
                base64ind = (val >> (no_of_bits-6)) & 63;
                no_of_bits = no_of_bits-6;
            }
            else {
                base64ind = (val << (6-no_of_bits)) & 63;
                no_of_bits = 0;
            }
            char tempchar = base64chars[base64ind];
            finalstr[stringInd] = tempchar;
            ++stringInd;
        }
    }
    for(int i = 1; i <= numOfEquals; i++) finalstr[stringInd++] = '=';
    finalstr[stringInd] = '\0';
    return finalstr;
}

int main() {
    FILE* examplesFile;
    FILE* answersFile;
    FILE* myAnswersFile;

    examplesFile = fopen("test.txt", "r");
    answersFile = fopen("res.txt", "r");
    myAnswersFile = fopen("myRes.txt", "w");

    int lineNum = 0;
    char* line = NULL;
    char* answer = NULL;
    char* c;
    size_t lineLen = 0, answerLen = 0;
    ssize_t read;
    while ((read = getline(&line, &lineLen, examplesFile)) != -1) {
        ++lineNum;
        c = strchr(line, '\n');
        if (c) *c = 0;
        lineLen = strlen(line);

        read = getline(&answer, &answerLen, answersFile);
        c = strchr(answer, '\n');
        if (c) *c = 0;
        answerLen = strlen(answer);

        char* myAnswer = base64Encoding(line, lineLen);
        int myAnswerLen = strlen(myAnswer);
        fputs(myAnswer, myAnswersFile);

        if (strcmp(answer, myAnswer) != 0) {
            printf(">> Cucked at line %d.\n", lineNum);
        }
        free(myAnswer);
        myAnswerLen = 0;
        free(line);
        lineLen = 0;
        free(answer);
        answerLen = 0;
    }
    fclose(examplesFile);
    fclose(answersFile);
    fclose(myAnswersFile);
    return 0;
}