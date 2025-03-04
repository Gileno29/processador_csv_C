#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libcsv.h"
#define MAX_COLUMNS 256
#define MAX_LINE_LENGTH 1024

typedef struct {
    char *header;
    int columnIndex;
    char comparator;
    char *value;
} Filter;

void parseFilters(const char *rowFilterDefinitions, Filter *filters, int *filterCount);
int parseSelectedColumns(const char *selectedColumns, char **columns);
void filterAndPrintCsv(char *csv, char **selectedColumns, int selectedColumnCount, Filter *filters, int filterCount);
int applyFilter(char **row, Filter *filters, int filterCount, char ** headers, int headerCount);
int compare(const char *a, const char *b, char comparator);

void processCsv(const char csv[], const char selectedColumns[], const char rowFilterDefinitions[]) {
    char *csvCopy = strdup(csv);

    if (!csvCopy){
        perror("Failed to allocate memory");
        return;

    }

    
    Filter filters[MAX_COLUMNS];
    int filterCount = 0;
    parseFilters(rowFilterDefinitions, filters, &filterCount);

    char *selectedColumnsArray[MAX_COLUMNS];
    int selectedColumnCount = parseSelectedColumns(selectedColumns, selectedColumnsArray);
    filterAndPrintCsv(csvCopy, selectedColumnsArray, selectedColumnCount, filters, filterCount);

    free(csvCopy);
}

void parseFilters(const char *rowFilterDefinitions, Filter *filters, int *filterCount) {
     char *definitions = strdup(rowFilterDefinitions);
    if (!definitions) {
        perror("Failed to allocate memory");
        return;
    }

    char *line = strtok(definitions, "\n");
    while (line != NULL) {
        // Find the position of the comparator
        char *headerEnd = strpbrk(line, "><=");
        if (headerEnd == NULL) {
            fprintf(stderr, "Error parsing header: no comparator found\n");
            break;
        }

        // Extract header
        size_t headerLen = headerEnd - line;
        char *header = (char *)malloc(headerLen + 1);
        if (!header) {
            perror("Failed to allocate memory for header");
            break;
        }
        strncpy(header, line, headerLen);
        header[headerLen] = '\0';

        // Extract comparator
        char comparator = *headerEnd;
        if (comparator != '>' && comparator != '<' && comparator != '=') {
            fprintf(stderr, "Error: invalid comparator '%c'\n", comparator);
            free(header);
            line = strtok(NULL, "\n");
            continue;
        }

        // Extract value
        char *value = headerEnd + 1;
        if (value == NULL) {
            fprintf(stderr, "Error parsing value\n");
            free(header);
            break;
        }

        filters[*filterCount].header = header;
        filters[*filterCount].comparator = comparator;
        filters[*filterCount].value = strdup(value);

        if (filters[*filterCount].header == NULL || filters[*filterCount].value == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            break;
        }

        (*filterCount)++;
        line = strtok(NULL, "\n");
    }

    free(definitions);
}

int parseSelectedColumns(const char *selectedColumns, char **columns) {
    if (selectedColumns == NULL || strlen(selectedColumns) == 0) {
        // selectedColumns is empty, return 0
        return 0;
    }
    int count = 0;
    char *columnsCopy = strdup(selectedColumns);
    char *column = strtok(columnsCopy, ",");
    while (column != NULL) {
        columns[count++] = strdup(column);
        column = strtok(NULL, ",");
    }
    free(columnsCopy);
    return count;
}



int applyFilter(char **row, Filter *filters, int filterCount, char **headers, int headerCount) {
    for (int i = 0; i < filterCount; i++) {
        int columnIndex = -1;

      
        // Procurar o nome da coluna literal nos cabeçalhos
      //  printf("\n################## antes do headercount ###########");
        for (int j = 0; j < headerCount; j++) {
            //printf("\nComparando header: '%s' com filtro: '%s'", headers[j], filters[i].header);
            if (strcmp(headers[j], filters[i].header) == 0) {
                columnIndex = j;
                break;
            }
            
        }

       // printf("\nFiltro %d: header='%s', comparator='%c', value='%s', columnIndex=%d", i, filters[i].header, filters[i].comparator, filters[i].value, columnIndex);

        if (columnIndex < 0 || columnIndex >= MAX_COLUMNS) {
            fprintf(stderr, "Error: invalid column index %d\n", columnIndex);
            return 0;
        }

        if (row[columnIndex] == NULL) {
            fprintf(stderr, "Error: NULL value in row at column index %d\n", columnIndex);
            return 0;
        }

        //printf("\nComparando valor: '%s' com filtro valor: '%s' usando comparator '%c'", row[columnIndex], filters[i].value, filters[i].comparator);
        if (!compare(row[columnIndex], filters[i].value, filters[i].comparator)) {
            return 0;
        }
    }
    return 1;
}

/*  filtra e  printa no stdout */
void filterAndPrintCsv(char *csv, char **selectedColumns, int selectedColumnCount, Filter *filters, int filterCount) {
char *lines[MAX_LINE_LENGTH];
    int lineCount = 0;

    char *line = strtok(csv, "\n");
    while (line != NULL) {
        lines[lineCount++] = line;
        line = strtok(NULL, "\n");
    }

    char *headers[MAX_COLUMNS];
    int headerCount = 0; // Inicializar a contagem de colunas
    line = lines[0];
    char *header = strtok(line, ",");
    while (header != NULL) {
        headers[headerCount++] = header; // Atualizar a contagem de colunas
        header = strtok(NULL, ",");
    }

    int selectedIndices[MAX_COLUMNS];
    if (selectedColumnCount > 0) {
        for (int i = 0; i < selectedColumnCount; i++) {
            for (int j = 0; j < headerCount; j++) {
                if (strcmp(selectedColumns[i], headers[j]) == 0) {
                    selectedIndices[i] = j;
                    break;
                }
            }
        }
    } else {
        // Se selectedColumns estiver vazio, selecionar todas as colunas
        for (int i = 0; i < headerCount; i++) {
            selectedIndices[i] = i;
        }
        selectedColumnCount = headerCount;
    }

    for (int i = 0; i < selectedColumnCount; i++) {
        printf("%s", headers[selectedIndices[i]]);
        if (i < selectedColumnCount - 1) {
            printf(",");
        }
    }
    printf("\n");

    for (int i = 1; i < lineCount; i++) {
        char *row[MAX_COLUMNS];
        int colIndex = 0;
        char *token = strtok(lines[i], ",");
        while (token != NULL) {
            row[colIndex++] = token;
            token = strtok(NULL, ",");
        }
        
        if (applyFilter(row, filters, filterCount, headers, headerCount)) {
            for (int j = 0; j < selectedColumnCount; j++) {
                printf("%s", row[selectedIndices[j]]);
                if (j < selectedColumnCount - 1) {
                    printf(",");
                }
            }
            printf("\n");
        }
    }
}


int compare(const char *a, const char *b, char comparator) {

   if (a == NULL || b == NULL) {
        fprintf(stderr, "Error: one of the strings is NULL in compare function.\n");
        return 0;
    }


    if (comparator != '>' && comparator != '<' && comparator != '=') {
        fprintf(stderr, "Error: invalid comparator '%c' in compare function.\n", comparator);
        return 0;
    }

    int cmp = strcmp(a, b);
    //printf("Comparing '%s' with '%s' using comparator '%c': cmp = %d\n", a, b, comparator, cmp);

    switch (comparator) {
        case '>': return cmp > 0;
        case '<': return cmp < 0;
        case '=': return cmp == 0;
        default:
            fprintf(stderr, "Error: invalid comparator '%c' in compare function.\n", comparator);
            return 0;
    }
}

void processCsvFile(const char csvFilePath[], const char selectedColumns[], const char rowFilterDefinitions[]) {
    FILE *file = fopen(csvFilePath, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    fseek(file, 0, SEEK_END);  //movimenta o cursor par ao final da do arquivo
    long fileSize = ftell(file); // pega o tamanho completo
    fseek(file, 0, SEEK_SET); // retorna o cursor par ao inicio do arquivo
    
    

    char *csvData = (char *)malloc(fileSize + 1);
    fread(csvData, 1, fileSize, file);
    csvData[fileSize] = '\0';
    //printf("estou na funcao de leitura de csv");
    fclose(file);
    
    //printf("Conteúdo de csvData:\n%s\n", csvData);

    processCsv(csvData, selectedColumns, rowFilterDefinitions);
    //printf("estou na funcao de leitura de csv 02");

   free(csvData);
}

