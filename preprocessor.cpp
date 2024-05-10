#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 128

// �̹� include�� ���̺귯���� ���Ե� ����
typedef struct IncludedStack {
    char library[MAX_LEN];
    struct IncludedStack* next;
} iStack;

iStack* includedTop = NULL;

void push(char* library) {
    iStack* node = (iStack*)malloc(sizeof(iStack));

    strcpy_s(node->library, MAX_LEN, library);

    node->next = includedTop;
    includedTop = node;
}

iStack* pop() {
    iStack* node = (iStack*)malloc(sizeof(iStack));

    node = includedTop;
    includedTop = includedTop->next;

    return node;
}

// iStack �޸� ����
void clearStack() {
    while (includedTop != NULL) {
        iStack* node = (iStack*)malloc(sizeof(iStack));

        node = includedTop;
        includedTop = includedTop->next;

        free(node);
    }
}

// �̹� include�� ���̺귯������ Ȯ��
bool isIncluded(char* library) {
    iStack* tmp = includedTop;

    while (tmp != NULL) {
        if (strcmp(tmp->library, library) == 0) return true;
        tmp = tmp->next;
    }

    return false;
}

// ���� ��� + ���� �̸� -> ���� ��� ����
char* extractFilePath(char* fileName) {
    int copySize = 0;
    char filePath[MAX_LEN] = "";

    for (int i = strlen(fileName) - 1; i >= 0; i--) {
        if (fileName[i] == '\\') {
            copySize = i + 1;
            break;
        }
    }
    strcpy_s(filePath, MAX_LEN, fileName);
    filePath[copySize] = '\0';

    return filePath;
}

// #include "���� �̸�" -> ���� �̸� ����
char* extractHeaderFile(char* buf) {
    char* context = NULL;
    char* token = strtok_s(buf, "\"", &context);

    while (token != NULL) {
        if (strstr(token, "#include") != NULL) {
            token = strtok_s(NULL, "\"", &context);
            continue;
        }
        else if (strtok_s(NULL, "\"", &context) != NULL && strstr(token, ".h")) return token;
        else return NULL;
    }
}

// ���� ��� + ���� �̸� -> ���� �̸� ����
char* extractFileName(char* buf) {
    int copySize = 0;
    char fileName[MAX_LEN] = "";

    for (int i = strlen(buf) - 1; i >= 0; i--) {
        if (buf[i] == '\\') {
            copySize = i + 1;
            break;
        }
    }
    strcpy_s(fileName, buf + copySize);
    fileName[strlen(buf) - copySize] = '\0';

    return fileName;
}

// #define var val -> var ����
char* extractVar(char* buf) {
    char* context = NULL;

    if (strcmp(strtok_s(buf, " ", &context), "#define") == 0) {
        return strtok_s(NULL, " ", &context);
    }
    else {
        printf("Wrong Function Called.\n");
        exit(1);
    }
}

// #define var val -> val ����
char* extractVal(char* buf) {
    char* context = NULL;
    char token[MAX_LEN];

    if (strcmp(strtok_s(buf, " ", &context), "#define") == 0) {
        strtok_s(NULL, " ", &context);
        strcpy_s(token, MAX_LEN, strtok_s(NULL, " ", &context));
        if (token[strlen(token) - 1] == '\n') token[strlen(token) - 1] = '\0';
        return token;
    }
    else {
        printf("Wrong Function Called.\n");
        exit(1);
    }
}

char* replaceVarToVal(char* buf, char* var, char* val) {
    char result[MAX_LEN];
    char* ptr = strstr(buf, var);

    if (ptr != NULL) {
        strcpy_s(result, MAX_LEN, buf);
        result[strlen(buf) - strlen(ptr)] = '\0';
        strcat_s(result, val);
        strcat_s(result, ptr + strlen(var));
    }

    return result;
}

// #include, #define ��ó�� ����
void preprocessFile(char* filePath) {
    FILE* ppFile = NULL;
    FILE* tmpFile = NULL;
    char tmpFilePath[MAX_LEN];
    char buf[MAX_LEN];

    // ����� �ӽ� ���� ����
    strcpy_s(tmpFilePath, MAX_LEN, extractFilePath(filePath));
    strcat_s(tmpFilePath, "tmp.txt");
    if (fopen_s(&tmpFile, tmpFilePath, "w+") != 0) {
        printf("File Not Found.\n");
    }

    if (fopen_s(&ppFile, filePath, "r+") == 0) {
        while (fgets(buf, MAX_LEN, ppFile) != NULL) {
            if (strstr(buf, "#include") != NULL && strstr(buf, ".h\"") != NULL) {
                char headerFile[MAX_LEN];
                char headerName[MAX_LEN] = "";
                char headerPath[MAX_LEN] = "";

                // ������� ��ü ���, ���, �̸� ����
                strcpy_s(headerFile, extractHeaderFile(buf));
                if (strstr(headerFile, "\\") == NULL) {
                    strcpy_s(headerName, MAX_LEN, headerFile);
                    strcpy_s(headerFile, MAX_LEN, filePath);
                    strcat_s(headerFile, headerName);
                }
                else {
                    strcpy_s(headerName, MAX_LEN, extractFileName(headerFile));
                    strcpy_s(headerPath, MAX_LEN, extractFilePath(headerFile));
                }

                if (isIncluded(headerName) == false) {
                    push(headerName);

                    FILE* hFile = NULL;
                    if (fopen_s(&hFile, headerFile, "r") == 0) {
                        while (fgets(buf, MAX_LEN, hFile) != NULL) {
                            // ������ϰ� ������Ͻ� �ش� ������ ��θ� �������� �����η� �����Ͽ� ����
                            if (strstr(buf, "#include") != NULL && strstr(buf, ".h\"") != NULL && strstr(buf, "\\") == NULL) {
                                char tmp[MAX_LEN];
                                char* context = NULL;

                                strcpy_s(tmp, MAX_LEN, strtok_s(buf, "\"", &context));
                                strcat_s(tmp, "\"");
                                strcat_s(tmp, headerPath);
                                strcat_s(tmp, MAX_LEN, strtok_s(NULL, "\"", &context));
                                strcat_s(tmp, "\"\n");

                                strcpy_s(buf, MAX_LEN, tmp);
                            }
                            fputs(buf, tmpFile);
                        }

                        fputs("\n", tmpFile);
                        while (fgets(buf, MAX_LEN, ppFile) != NULL) fputs(buf, tmpFile);

                        // ppFile�� ����
                        fclose(ppFile);
                        rewind(tmpFile);
                        if (fopen_s(&ppFile, filePath, "w") == 0) {
                            while (fgets(buf, MAX_LEN, tmpFile) != NULL) fputs(buf, ppFile);
                        }

                        fclose(ppFile);
                        fclose(tmpFile);
                        remove(tmpFilePath);
                        preprocessFile(filePath);
                        return;
                    }
                }
            }
            else if (strstr(buf, "#define") != NULL) {
                char var[MAX_LEN];
                char val[MAX_LEN];
                char tmp[MAX_LEN];

                // #define var val -> var, val ����
                strcpy_s(tmp, MAX_LEN, buf);
                strcpy_s(var, MAX_LEN, extractVar(tmp));
                strcpy_s(tmp, MAX_LEN, buf);
                strcpy_s(val, MAX_LEN, extractVal(tmp));

                // var -> val ��ü
                while (fgets(buf, MAX_LEN, ppFile) != NULL) {
                    if (strstr(buf, "#define") != NULL && strstr(buf, var) != NULL) { // �ٸ� #define���� var �̸��� ��ĥ ��� ����ó��
                        fputs(buf, tmpFile);
                        while (fgets(buf, MAX_LEN, ppFile) != NULL) fputs(buf, tmpFile);
                        break;
                    }
                    else if (strstr(buf, var) != NULL) {
                        strcpy_s(buf, MAX_LEN, replaceVarToVal(buf, var, val));
                    }
                    fputs(buf, tmpFile);
                }

                // ppFile�� ����
                fclose(ppFile);
                rewind(tmpFile);
                if (fopen_s(&ppFile, filePath, "w") == 0) {
                    while (fgets(buf, MAX_LEN, tmpFile) != NULL) fputs(buf, ppFile);
                }

                fclose(ppFile);
                fclose(tmpFile);
                remove(tmpFilePath);
                preprocessFile(filePath);
                return;
            }
            else fputs(buf, tmpFile);
        }
    }
    // ppFile�� ����
    fclose(ppFile);
    rewind(tmpFile);
    if (fopen_s(&ppFile, filePath, "w") == 0) {
        while (fgets(buf, MAX_LEN, tmpFile) != NULL) fputs(buf, ppFile);
    }

    fclose(ppFile);
    fclose(tmpFile);
    remove(tmpFilePath);
}

int main() {
    FILE* file = NULL;
    char fileName[MAX_LEN];
    char ppFileName[MAX_LEN];

    printf("��ó�� �� ������ �Է��Ͻʽÿ�. (���� ���): ");
    scanf_s("%s", fileName, MAX_LEN);

    if (fopen_s(&file, fileName, "r") == 0) {
        char buf[MAX_LEN];
        char fileType[MAX_LEN] = ".";
        char name[MAX_LEN];
        char* context = NULL;
        FILE* ppFile = NULL;

        // ���ϸ� + ����Ÿ�� �и�
        strcpy_s(name, MAX_LEN, strtok_s(extractFileName(fileName), ".", &context));
        strcat_s(fileType, strtok_s(NULL, ".", &context));

        // ��� ���� ���� �� ����
        strcpy_s(ppFileName, MAX_LEN, extractFilePath(fileName));
        strcat_s(ppFileName, name);
        strcat_s(ppFileName, "_pp");
        strcat_s(ppFileName, fileType);
        if (fopen_s(&ppFile, ppFileName, "w") == 0) {
            while (fgets(buf, MAX_LEN, file) != NULL) {
                // ��������� ������Ͻ� �ش� ������ ��θ� �������� �����η� �����Ͽ� ����
                if (strstr(buf, "#include") != NULL && strstr(buf, ".h\"") != NULL && strstr(buf, "\\") == NULL) {
                    char tmp[MAX_LEN];
                    context = NULL;

                    strcpy_s(tmp, MAX_LEN, strtok_s(buf, "\"", &context));
                    strcat_s(tmp, "\"");
                    strcat_s(tmp, MAX_LEN, extractFilePath(ppFileName));
                    strcat_s(tmp, MAX_LEN, strtok_s(NULL, "\"", &context));
                    strcat_s(tmp, "\"\n");

                    strcpy_s(buf, MAX_LEN, tmp);
                }
                fputs(buf, ppFile);
            }
            fclose(file);
            fclose(ppFile);

            // ���� ��ó��
            preprocessFile(ppFileName);
            clearStack();
        }
        else {
            printf("File Not Opened.\n");
            exit(1);
        }
    }
    else printf("File Not Found.\n");

    return 0;
}