#include <stdio.h>
#include "libcsv.h"


int main() {
    
    const char csv[] = "header1,header2,header3\n1,2,3\n4,5,6\n7,8,9";
    processCsv(csv, "header1,header3", "header1>1\nheader3<8");

    const char csvFilePath[] = "./data.csv";
    processCsvFile(csvFilePath, "header1","");

    return 0;
}
