#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include "my402list.h"


/*
 * Structure to hold the 
 * transactions
 */

#define BUFLENGTH 1026

typedef struct tagListEntry{
    
    long int timestamp;
    char trans_type[1];
    int trans_amt;
    char *trans_des;
    int balance;

}ListEntry;

int sortInsert(My402List *list, ListEntry *Entry)
{
    My402ListElem *temp;
    ListEntry *tempobj;
    temp = My402ListNext(list, &list->anchor);

    while(temp){
        tempobj = (ListEntry *) temp->obj;
        if(Entry->timestamp == tempobj->timestamp) {
            fprintf(stderr, "Transactions have Identical timestamp %ld\n", Entry->timestamp);
            return 0;
        }
        if(Entry->timestamp < tempobj->timestamp) {
            if(!My402ListInsertBefore(list, Entry, temp)){
                return 0;
            }
            return 1;
        }
        temp = My402ListNext(list, temp);
    }
    if(!My402ListAppend(list, Entry)){
        return 0;
    }
    return 1;
}

int isTypeValid(char *tok)
{
    if(strlen(tok) > 1){
        fprintf(stderr, "Malformed input %s\n", tok);
        return 0;
    }
    if(*tok == '+' || *tok == '-'){
        return 1;
    }
    fprintf(stderr, "Malformed input %s\n", tok);
    return 0;
}

int isTimestampValid(char *tok)
{
    int i;
    int len =0;
    len = strlen(tok);
    if(len > 10) {
        fprintf(stderr, "Malformed timestamp %s\n", tok);
        return 0;
    }
    if(tok[len-1] == '\n'){
        fprintf(stderr, "Other fields missing for timestamp %s", tok);
        return 0;
    }
    for(i = 0; i<len; i++){
        if(!isdigit(tok[i])){
            fprintf(stderr, "Malformed timestamp %s\n", tok);
            return 0;
        }
    }
    return 1;
}

int isAmountValid(char *tok)
{
    int i, dot = 0, countAfterDecimal = 0;

    int len = 0;

    len = strlen(tok);

    if(tok[len-1] == '\n'){
        fprintf(stderr, "Other fields missing for amount %s", tok);
        return 0;
    }
    for(i = 0; i<len; i++){
        if(tok[i] == '.'){
            dot++;
        }
        if(!isdigit(tok[i])) {
            if(tok[i] == '.' && dot == 1) {
                continue;
            }
            fprintf(stderr, "Malformed amount %s\n", tok);
            return 0;
        }
        if(dot == 1){
            countAfterDecimal++;
        }
    }
    if(countAfterDecimal <= 1 || countAfterDecimal > 2){
        fprintf(stderr, "Wrong amount format %s: format should "
                "have 2 digits after the decimal point: ??.??\n", tok);
        return 0;
    }
    return 1;
}



ListEntry* Parser(char *tok)
{
    ListEntry *Entry;
    int num_fields = 0;
    int desc_len;
    int value_1;
    int value_2;
    char *copy, *ptr;

    Entry = malloc(sizeof(ListEntry));
    if(!Entry){
        fprintf(stderr, "Error in allocating memory: %s\n", strerror(errno));
        return 0;
    }
    memset(Entry, 0, sizeof(ListEntry));

    while (tok != NULL)
    {
        if(num_fields > 3) {
            fprintf(stderr, "Too many fields\n");
            free(Entry);
            return NULL;
        }

        if(num_fields == 0) {
            if(!isTypeValid(tok)){
                free(Entry);
                return NULL;
            }
            strcpy(Entry->trans_type, tok);
        }

        if(num_fields == 1){
            if(!isTimestampValid(tok)){
                free(Entry);
                return NULL;
            }
            Entry->timestamp = strtoul(tok, NULL, 10);
        }

        if(num_fields == 2){
            if(!isAmountValid(tok)){
                free(Entry);
                return NULL;
            }else {
                /* value = strtod(tok, NULL); */
                sscanf(tok, "%d.%d", &value_1, &value_2);
                value_1 = 100*value_1 + value_2;
                if(value_1 >= 1000000000){
                    fprintf(stderr, "Amount is greater than 10 million %s\n", tok);
                    free(Entry);
                    return NULL;
                }
                Entry->trans_amt = value_1;
            }
        }

        if(num_fields == 3){
            desc_len = strlen(tok);
            if(!desc_len){
                free(Entry);
                fprintf(stderr, "Entry has no description\n");
                return NULL;
            }
            copy = malloc(desc_len * sizeof(char) + 1);
            if(!copy){
                fprintf(stderr, "Error in allocating memory: %s\n", strerror(errno));
                free(Entry);
                return NULL;
            }
            memset(copy, 0, (desc_len * sizeof(char) + 1));
            strncpy(copy, tok, (desc_len +1));
            ptr = copy;
            while(*copy == ' ') {
                copy++;
            }
            if(copy && *copy == '\n'){
                fprintf(stderr, "Entry has with timestamp %ld has no description\n", Entry->timestamp); 
                free(Entry);
                free(ptr);
                return NULL;
            }
            desc_len = strlen(copy);
            if(!desc_len) {
                free(ptr);
                free(Entry);
                return NULL;
            }
            Entry->trans_des = malloc(desc_len * sizeof(char) + 1);
            if(!Entry->trans_des){
                fprintf(stderr, "Error in allocating memory: %s\n", strerror(errno));
                free(Entry);
                free(ptr);
                return NULL;
            }
            memset(Entry->trans_des, 0, (desc_len * sizeof(char) + 1));
            strncpy(Entry->trans_des, copy, (desc_len +1));
            if(Entry->trans_des[desc_len-1] == '\n'){
                Entry->trans_des[desc_len-1] = '\0';
            }
            free(ptr);

        }
        num_fields++;
        tok = strtok(NULL, "\t");
    }
    return Entry;
}

int checkFile(char *file)
{
    FILE* fid;
    struct stat stat_buf;

    if(stat(file, &stat_buf) != 0){
        fprintf(stderr, "Error, in file status: %s\n", strerror(errno));
        return 0;
    }

    if (S_ISDIR (stat_buf.st_mode)) {
        fprintf(stderr, "Error, %s is a directory\n", file);
        return 0;
    }
    fid = fopen(file, "r");
    if(!fid) {
        fprintf(stderr, "warmup1: Could not open the file %s: %s\n",
                file, strerror(errno));
        return 0;
    }
    fclose(fid);
    return 1;

}




int Input(My402List *list, char *file)
{
    ListEntry *Entry;
    FILE *fid;
    char line[BUFLENGTH];
    char *tok;

    if(file) {
        fid = fopen(file, "r");
        if(!fid){
            fprintf(stderr, "warmup1: Could not open the file %s: %s\n",
                    file, strerror(errno));
            return 0;
        }
    } else{
        fid = stdin;
    }

    memset(line, 0, BUFLENGTH);

    while(fgets(line, BUFLENGTH, fid) != NULL) {
        if(line[1024] != '\n' && line[1024] != '\0') {
            fprintf(stderr, "Sorry, the entry %s is too long\n", line);
            goto error;
        }
        tok = strtok(line,"\t");
        if(!tok) {
            fprintf(stderr, "Malformed sentence\n");
            goto error;
        }
        Entry = Parser(tok);
        if(!Entry){
            goto error;
        }
        if(!sortInsert(list, Entry)){
            free(Entry->trans_des);
            free(Entry);
            goto error;
        }

        memset(line, 0, BUFLENGTH);
    }
    if(file){
        fclose(fid);
    }
    return 1;

error:
    if(file){
        fclose(fid);
    }
    return 0;
}

void PrintLines()
{
    /*
     * Column space required 80 in total
     */
    int i, j;
    int lines[4] = {18, 27, 17, 17};

    /*
     * Totally 80 characters
     */
    for(i = 0; i<4; i++){
        printf("+");
        for(j = 1; j<lines[i]; j++){
            printf("-");
        }
    }
    printf("+\n");
}


void PrintInitial()
{
    int i;

    PrintLines();
    /*
     * Next row
     */
    printf("|");
    for(i = 1; i< 8; i++){
        printf(" ");
    }
    printf("Date");
    for(i = 1; i< 7; i++){
        printf(" ");
    }
    printf("| ");
    printf("Description");
    for(i =1; i< 15; i++){
        printf(" ");
    }
    printf("|");
    for(i =1; i< 10; i++){
        printf(" ");
    }
    printf("Amount |");

    for( i=1 ; i<9; i++){
        printf(" ");
    }
    printf("Balance |\n");
    PrintLines();
}

char* pDate(My402ListElem *elem, char* date)
{
    ListEntry *Entry;
    int i= 0, j=0;
    /*
     * 3 for week
     * 3 for month
     * 2 date
     * 4 year
     * 3 spaces between the above
     * 1 NULL character
     * total = 16
     * + 11 if time is considered 
     */
    char time[25];

    Entry = (ListEntry*) elem->obj;

    /*
     * Adding date to the line
     */
    snprintf(time, 25,"%s", ctime(&Entry->timestamp));
    for(i = 0; i< 11; i++){
        date[i]=time[i];
    }
    /*
     * The time starts from the 11th character
     */
    j = 11;
    /*
     * Year starts at the 20th character
     */
    i = 20;
    while(time[i] != '\0') {
        date[j] = time[i];
        j++;
        i++;
    }
    date[j]='\0';

    return date;
}

char* pDescription(My402ListElem *elem, char *desc)
{
    /*
     * 24 characters allowed for description
     * 1 for NULL
     */

    ListEntry *Entry;
    int i = 0;

    Entry = (ListEntry *)elem->obj;
    memset(desc, 0, 25);

    snprintf(desc, 25, "%s", Entry->trans_des);
    int len = strlen(desc);
    if(len < 24){
        len = 24 - len;
        for(i = 0; i<len; i++){
            strcat(desc, " ");
        }
    }
    return desc;
}

char* pValue(char* amt, char *temp)
{
    int i = 0, j = 0, cnt = 0, dec = 0;
    int len = 0;
   
    if(!strncmp(temp, "0", 15)) {
        snprintf(amt, 15, "0.00");
        return amt;
    }

    len = strlen(temp);
    j = len-1;

    /*
     * If we have just 3 digits or less
     * the +2 , for decimal and "," 
     */
    if(len <= 5){
        i = j + 1;
    }else if(len > 5 && len <=8){
        i = j + 2;
    } else if(len > 8) {
        i = j + 3;
    }
    while(j>=0){
        if(dec == 2){
            amt[i] = '.';
            i--;
            cnt = 0;
            dec++;
            continue;
        }
        if(cnt == 3){
            amt[i] = ',';
            i--;
            cnt = 0;
            continue;
        }
        amt[i] = temp[j];
        if(dec < 2){
            dec++;
        }
        cnt++;
        i--;
        j--;
    }

    return amt;
}


char* pAmount(My402ListElem *elem, char *amt)
{
    ListEntry *Entry;
    
    /*
     * 14 for amount along with brackets
     * 1 for null
     */
    char temp[15];


    Entry = (ListEntry *)elem->obj;
    if(Entry->trans_amt >= 1000000000){
        if(Entry->trans_type[0] == '+'){
            strncpy(amt,"?,???,???.?? ", 15);
        }else {
            strncpy(amt, "(?,???,???.??", 15);
            strcat(amt, ")");
        }
        return amt;
    }
    snprintf(temp, 15, "%03d", Entry->trans_amt);

    /*
     * number of spaces to be given
     */
    amt = pValue(amt, temp);
    if(Entry->trans_type[0] == '+'){
        memset(temp, 0, 15);
        strncpy(temp, amt, 15);
        memset(amt, 0, 15);
        snprintf(amt, 15, "%13s",temp);
        strncat(amt, " ", 15);
    } else{
        memset(temp, 0, 15);
        strncpy(temp, amt, 15);
        memset(amt, 0, 15);
        snprintf(amt, 15, "(%12s",temp);
        strncat(amt, ")", 15);
    }
    return amt;
}

char* pBalance(My402List *list, My402ListElem *elem, char *bal)
{
    int balance = 0;
    char temp[15];

    /*
     * 14 characters for balance including "(", ")"
     * 1 extra for the null character
     */

    ListEntry* Entry = NULL, *prevEntry = NULL;
    Entry = (ListEntry *)elem->obj;
    if(My402ListPrev(list, elem)){
        prevEntry = (ListEntry *)elem->prev->obj;
    }
    if(Entry->trans_type[0] == '+') {
        if(prevEntry) {
            Entry->balance =  prevEntry->balance + Entry->trans_amt;
        }else {
            Entry->balance = Entry->trans_amt;
        }
    }else{
        if(prevEntry) {
            Entry->balance = prevEntry->balance - Entry->trans_amt;
        }else {
            Entry->balance = (Entry->trans_amt) * (-1);
        }
    }

    if(Entry->balance < 0){
        balance = (Entry->balance) * (-1);
        if(balance >= 1000000000) {
            strncpy(bal, "(?,???,???.??", 15);
            strcat(bal, ")");
            return bal;
        }
        snprintf(temp, 15, "%d", balance);

        bal = pValue(bal, temp);
        strncpy(temp, bal, 15);
        memset(bal, 0, 15);
        snprintf(bal, 15, "(%12s",temp);

        strncat(bal,")", 15);
    } else{
        balance = Entry->balance;
        if(balance >= 1000000000) {
            strncpy(bal, "?,???,???.?? ", 15);
            return bal;
        }
        snprintf(temp, 15, "%d", balance);

        bal = pValue(bal, temp);
        memset(temp, 0 , 15);
        strncpy(temp, bal, 15);
        memset(bal, 0, 15);
        snprintf(bal, 15, "%13s",temp);
        strncat(bal," ", 15);
    }
    return bal;
}


void Output(My402List *list)
{

    /*
     * The allowed limit is 80
     */
    char date[25], desc[25], amt[15], bal[15];
    My402ListElem *temp;
    PrintInitial();
    memset(date, 0, 25);
    memset(desc, 0, 25);
    memset(amt, 0, 15);
    memset(bal, 0, 15);

    temp = My402ListNext(list, &list->anchor);
    while(temp) {
        printf("| %s | %s | %s | %s |\n",
                pDate(temp, date), pDescription(temp, desc), pAmount(temp, amt), pBalance(list, temp, bal));

        memset(date, 0, 25);
        memset(desc, 0, 25);
        memset(amt, 0, 15);
        memset(bal, 0, 15);

        temp = My402ListNext(list, temp);
    }
    PrintLines();

}



void FreeObjMem(My402List *list)
{
    ListEntry *Entry;
    My402ListElem *temp;

    temp = My402ListNext(list, &list->anchor);

    while(temp) {
        Entry = (ListEntry *) temp->obj;
        free(Entry->trans_des);
        free(Entry);
        temp = My402ListNext(list, temp);
    }
}


int main(int argc, char *argv[])
{
    int res = 0;
    /*
     * Check if the number of arguments passed 
     * if less than 2 return error with the
     * expected format.
     */
    if(argc < 2 ){
        fprintf(stderr, "Missing the sort command\n");
        return 0;
    }
    
    /*
     * Make sure the second argument 
     * is sort
     */
    if(strncmp(argv[1], "sort", 4)){
        fprintf(stderr, "Wrong command, please enter \"warmup1 sort [tfile]\"\n");
        return 0;
    }
    
    /*
     * If file is given:
     * Check if the file is
     *  Accessible
     *  Valid
     *  Not a directory
     */

    if(argc == 3){
        if(argv[2]){
            if(!checkFile(argv[2])){
                    return 0;
            }
        }
    }

    /*
     * Initialize the circular list
     */

    My402List list;
    memset(&list, 0, sizeof(My402List));
    res = My402ListInit(&list);

    if(!res) {
        fprintf(stderr, "Error during Initialization of the list\n");
        exit(0);
    }

    /*
     * We will now call the parse function
     */
    if(!Input(&list, argv[2])) {
        FreeObjMem(&list);
        My402ListUnlinkAll(&list);
        exit(0);
    }
    
    /*
     * Output the balance
     */
    Output(&list);

    /*
     * free the memory space used for all the 
     * objects
     */
    FreeObjMem(&list);

    /*
     * Unlink all the list elements
     */
    My402ListUnlinkAll(&list);

    exit(0);
}




