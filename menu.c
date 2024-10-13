#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "transcoder_headers.h"

int menu() {
    printf("1. Convert to MOV\n");
    printf("2. Convert to MP4\n");
    printf("3. Convert to AVI\n");
    printf("4. Exit\n");

    int choice;
    printf("Select the new file type: ");
    scanf("%d", &choice);

    char input_file[256];
    printf("Enter the input file name: ");
    scanf("%s", input_file);

    switch (choice) {
    case 1:
        mov(input_file);
        break;
    case 2:
        //mp4();
        printf("MP4\n");
        break;
    case 3:
        //avi();
        printf("AVI\n");
        break;
    case 4:
        printf("Exiting...\n");
        break;
    default:
        printf("Invalid choice\n");
        break;
    }
    return 0;
}