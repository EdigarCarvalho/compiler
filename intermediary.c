#define MAX_INTERMEDIATE_CODE 1000

typedef struct {
    char op[20];
    char arg1[50];
    char arg2[50];
    char result[50];
} IntermediateCode;

IntermediateCode intermediateCode[MAX_INTERMEDIATE_CODE];
int intermediateCodeCount = 0;

void generateIntermediateCode(const char* op, const char* arg1, const char* arg2, const char* result) {
    if (intermediateCodeCount < MAX_INTERMEDIATE_CODE) {
        strcpy(intermediateCode[intermediateCodeCount].op, op);
        strcpy(intermediateCode[intermediateCodeCount].arg1, arg1);
        strcpy(intermediateCode[intermediateCodeCount].arg2, arg2);
        strcpy(intermediateCode[intermediateCodeCount].result, result);
        intermediateCodeCount++;
    } else {
        printf("Erro: Limite de código intermediário excedido.\n");
    }
}

void printIntermediateCode() {
    printf("\n=== Código Intermediário ===\n");
    for (int i = 0; i < intermediateCodeCount; i++) {
        printf("%s %s, %s, %s\n",
            intermediateCode[i].op,
            intermediateCode[i].arg1[0] ? intermediateCode[i].arg1 : "_",
            intermediateCode[i].arg2[0] ? intermediateCode[i].arg2 : "_",
            intermediateCode[i].result);
    }
}
