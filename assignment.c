#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include <ctype.h>

#define MAXNUMBOATS 120
#define MAXNAMELEN 127

typedef enum {
    SLIP,
    LAND,
    TRAILOR,
    STORAGE,
    NO_PLACE
} PlaceType;

//Function Prototypes
void addBoat(const char *line);
PlaceType StringToPlaceType(char * PlaceString);
char * PlaceToString(PlaceType Place);
void loadData(const char *fileName);
void removeBoat(const char *name);
void saveData(const char *fileName);
void printBoats();
void acceptPayment(const char *name);
void updateAmountOwed();
int compareBoats(const void *a, const void *b);
void sortBoats();


typedef union {
    int slip_number;
    char bay_letter;
    char trailor_license[MAXNAMELEN];
    int storage_number;
} PlaceInfo;

typedef struct {
    char name[MAXNAMELEN];
    int boatLength; 
    PlaceType place;
    PlaceInfo info;
    double moneyOwed;
} Boat;

//Make our array of Boats and a boat counter
Boat *boats[MAXNUMBOATS];
int boatCount = 0;

//Convert a string to a place
PlaceType StringToPlaceType(char * PlaceString) {

    if (!strcasecmp(PlaceString,"slip")) {
        return(SLIP);
    }
    if (!strcasecmp(PlaceString,"land")) {
        return(LAND);
    }
    if (!strcasecmp(PlaceString,"trailor")) {
        return(TRAILOR);
    }
    if (!strcasecmp(PlaceString,"storage")) {
        return(STORAGE);
    }
    return(NO_PLACE);
}

//Convert a place to a string
char * PlaceToString(PlaceType Place) {

    switch (Place) {
        case SLIP:
            return("slip");
        case LAND:
            return("land");
        case TRAILOR:
            return("trailor");
        case STORAGE:
            return("storage");
        case NO_PLACE:
            return("no_place");
        default:
            printf("How the faaark did I get here?\n");
            exit(EXIT_FAILURE);
            break;
    }
}

//load data function
void loadData(const char *fileName){
    FILE *file = fopen(fileName, "r");
    if(!file) return;

    char line[256];
    while(fgets(line, sizeof(line), file) && boatCount < MAXNUMBOATS){
        addBoat(line);
    }
    sortBoats();
    fclose(file);
}

//helper function for the qsort
int compareBoats(const void *a, const void *b) {
    Boat *boatA = *(Boat **)a;
    Boat *boatB = *(Boat **)b;
    return strcasecmp(boatA->name, boatB->name);
}

//utilize qsort to sort boats alphabetically (case-insensitive)
void sortBoats() {
    qsort(boats, boatCount, sizeof(Boat *), compareBoats);
}

//function to add a boat and its info
void addBoat(const char *line){
    if(boatCount >= MAXNUMBOATS) return;

    Boat *b = malloc(sizeof(Boat));
    if(!b) return;

    char placeStr[10], placeInfo[128];  // store the raw place field and its associated info
    int parsed = sscanf(line, "%126[^,],%d,%9[^,],%127[^,],%lf", b->name, &b->boatLength, placeStr, placeInfo, &b->moneyOwed);

    if(parsed != 5) { //check for valid user input
        printf("Error: Invalid data format. Skipping line: %s\n", line);
        free(b);
        return;
    }

    // Normalize and convert place type
    for (int i = 0; placeStr[i]; i++) placeStr[i] = tolower(placeStr[i]);
    b->place = StringToPlaceType(placeStr);

    // Populate the union based on place type
    switch(b->place) {
        case SLIP:
            b->info.slip_number = atoi(placeInfo);
            break;
        case LAND:
            b->info.bay_letter = placeInfo[0];  // assumes first char is the bay letter
            break;
        case TRAILOR:
            strncpy(b->info.trailor_license, placeInfo, MAXNAMELEN - 1);
            b->info.trailor_license[MAXNAMELEN - 1] = '\0';  // null-terminate
            break;
        case STORAGE:
            b->info.storage_number = atoi(placeInfo);
            break;
        default:
            printf("Invalid place type: %s\n", placeStr);
            free(b);
            return;
    }

    boats[boatCount++] = b; //add the new boat to the array
}

//function to remove a boat
void removeBoat(const char *name){
    for(int i = 0; i < boatCount; i++){

        if (strcasecmp(name, boats[i]->name) == 0){
            free(boats[i]);
            //adjust position of all of the boats after removal
            for(int j = i; j < boatCount - 1; j++){
                boats[j] = boats[j+1];
            }
            boatCount--;
            return;
        }

    }
    printf("No boat with that name\n"); 
}

//function to save the data 
void saveData(const char *fileName){
    FILE *file = fopen(fileName, "w");
    if(!file) return;

    //write the place type first, then (depending on place type), populate place-specific info
    for(int i = 0; i < boatCount; i++){
        Boat *b = boats[i];
        char *placeStr = PlaceToString(b->place);

        switch (b->place) {
            case SLIP:
                fprintf(file, "%s,%d,%s,%d,%.2f\n", b->name, b->boatLength, placeStr, b->info.slip_number, b->moneyOwed);
                break;
            case LAND:
                fprintf(file, "%s,%d,%s,%c,%.2f\n", b->name, b->boatLength, placeStr, b->info.bay_letter, b->moneyOwed);
                break;
            case TRAILOR:
                fprintf(file, "%s,%d,%s,%s,%.2f\n", b->name, b->boatLength, placeStr, b->info.trailor_license, b->moneyOwed);
                break;
            case STORAGE:
                fprintf(file, "%s,%d,%s,%d,%.2f\n", b->name, b->boatLength, placeStr, b->info.storage_number, b->moneyOwed);
                break;
            default:
                fprintf(file, "%s,%d,no_place,unknown,%.2f\n", b->name, b->boatLength, b->moneyOwed);
                break;
        }
    }

    fclose(file);
}

void printBoats() {
    for (int i = 0; i < boatCount; i++) {
        Boat *b = boats[i];
        char *placeStr = PlaceToString(b->place);

        //print name and boat length first
        printf("%-15s %2d'   ", b->name, b->boatLength);

        //then print place-specific info
        if (b->place == SLIP) {
            printf("%-8s # %-5d", placeStr, b->info.slip_number);
        } else if (b->place == LAND) {
            printf("%-8s   %-5c", placeStr, b->info.bay_letter);
        } else if (b->place == TRAILOR) {
            printf("%-8s %-7s", placeStr, b->info.trailor_license);
        } else if (b->place == STORAGE) {
            printf("%-8s # %-5d", placeStr, b->info.storage_number);
        } else {
            printf("unknown    ");
        }

        // Print money owed
        printf(" Owes $%7.2f\n", b->moneyOwed);
    }
}

//accept a payment from the user, check its validity, and update the amount
void acceptPayment(const char *name){
    for(int i = 0; i < boatCount; i++){

        if (strcasecmp(name, boats[i]->name) == 0){

            printf("Please enter the amount to be paid                       : ");
            double amount;
            scanf("%lf", &amount);

            //check validity of amount to be paid
            if(amount > boats[i]->moneyOwed){
                printf("That is more than the amount owed, $%.2lf", boats[i]->moneyOwed);
                printf(" \n");
            } else{ boats[i]->moneyOwed -= amount; }

            return; 
        }
    }

    printf("No boat with that name\n");
}

// Updates each boat's amount owed based on its storage type
void updateAmountOwed(){
    if(boatCount == 0){ printf("no boats to update"); return; }

    for(int i = 0; i < boatCount; i++){

        //update amount owed based on place rate
        switch (boats[i]->place) {
            case SLIP:
                boats[i]->moneyOwed += (12.5 * boats[i]->boatLength);
                break;
            case LAND:
                boats[i]->moneyOwed += (14.0 * boats[i]->boatLength);
                break;
            case TRAILOR:
                boats[i]->moneyOwed += (25.0 * boats[i]->boatLength);
                break;
            case STORAGE:
                boats[i]->moneyOwed += (11.20 * boats[i]->boatLength);
                break;
            case NO_PLACE:
                printf("Boat stored at no place, so no money owed or to update\n");
                break;
            default:
                printf("update failed, invalid storage place\n");
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]){

    //check for correct amount of argements
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    loadData(argv[1]);

    printf(" \n");
    printf(" \n");
    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");
    printf(" \n");

    while(1){

        printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        char userInput; 
        scanf(" %c", &userInput);

        //check for a valid input ("add" could cause issues)
        int nextChar = getchar();
        if (nextChar != '\n') {
            printf("Error: Please enter only one character.\n");
            // Flush the rest of the line
            while (nextChar != '\n' && nextChar != EOF) {
                nextChar = getchar();
            }
            continue;  // restart the input loop
        }

        char TempUserInput = userInput;
        userInput = tolower(userInput);

        if(userInput == 'i'){
            printBoats();
        }
        else if(userInput == 'a'){
            printf("Please enter the boat data in CSV format                 : ");
            char userData[256];
            scanf(" %[^\n]", userData);
            addBoat(userData);
            sortBoats();
        }
        else if(userInput == 'r'){
            printf("Please enter the boat name                               : ");
            char boatName[MAXNAMELEN];
            scanf(" %[^\n]", boatName);
            removeBoat(boatName);
        }
        else if(userInput == 'p'){
            printf("Please enter the boat name                               : ");
            char boatName[MAXNAMELEN];
            scanf(" %[^\n]", boatName);
            acceptPayment(boatName);
        }
        else if(userInput == 'm'){
            updateAmountOwed();
        }
        else if(userInput == 'x'){
            saveData(argv[1]);
            printf(" \n");
            printf("Exiting the Boat Management System...\n");
            break;
        }
        else{
            printf("Invalid option %c\n", TempUserInput);
        }

        printf(" \n");
    }

    for (int i = 0; i < boatCount; i++) {
        free(boats[i]); //free all memory
    }

    return 0;
}
