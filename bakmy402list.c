#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "my402list.h"


/*
 * Structure to hold the 
 * transactions
 */

#define BUFLENGTH 1026

typedef struct tagMy402ListEntry{
    
    unsigned long int timestamp;
    char tran_type[1];
    int trans_amt;
    char *trans_des;

}My402ListEntry;


void sortInsert(My402List *list, My402ListEntry *Entry)
{
    My402ListElem *temp;
    My402ListEntry *tempobj;
    temp = list->anchor.next;


    while(temp){
        tempobj = (My402ListEntry *) temp->obj;
        if(Entry->timestamp <= tempobj->timestamp) {
            My402ListInsertBefore(list, Entry, temp);
            return;
        }
        temp = temp->next;
    }
    My402ListAppend(list, Entry);
}

My402ListEntry* My402Parser(char *tok)
{
    My402ListEntry *Entry;
    int num_fields = 0;
    int desc_len;

    Entry = malloc(sizeof(My402ListEntry));
    memset(Entry, 0, sizeof(My402ListEntry));

    while (tok != NULL)
    {
        if(num_fields > 3) {
            printf("Too many fields\n");
            return NULL;
        }

        if(num_fields == 0) {
            //Check if it is +/-
            strcpy(Entry->tran_type, tok);
        }

        if(num_fields == 1){
            /*if(!isdigit(tok)){
                printf("Malformed Entry %s\n", tok);
                return NULL;
            }*/
            Entry->timestamp = strtoul(tok, NULL, 10);
        }

        if(num_fields == 2){
            /*if(!isdigit(tok)) {
                printf("Malformed Entry %s\n", tok);
                return NULL;
            }*/
            //If amount is more than 10 million
            Entry->trans_amt =  strtoul(tok, NULL, 10);
        }

        if(num_fields == 3){
            desc_len = strlen(tok);
            Entry->trans_des = malloc(desc_len * sizeof(char) + 1);
            strncpy(Entry->trans_des, tok, (desc_len +1));
            if(Entry->trans_des[desc_len-1] == '\n'){
                Entry->trans_des[desc_len-1] = '\0';
            }

        }
         num_fields++;
        tok = strtok(NULL, "\t");
    }
    return Entry;
}


int My402Input(My402List *list, char *file)
{
    int i;
    My402ListEntry *Entry;
    FILE *fid;
    char line[BUFLENGTH];
    char *tok;

    fid = fopen(file, "r");

    if(fid == NULL){
        return 1;
    }

    memset(line, 0, BUFLENGTH);

    while(fgets(line, BUFLENGTH, fid) != NULL) {
        if(line[1024] != '\n' && line[1024] != '\0') {
            printf("Sorry, the entry is too long\n");
            return 0;
        }
        tok = strtok(line,"\t");
        if(!tok) {
            printf("Malformed sentence\n");
            return 0;
        }
        Entry = My402Parser(tok);
        sortInsert(list, Entry);
        memset(line, 0, BUFLENGTH);
    }
    return 1;
}

int My402ListLength(My402List *list)
{
    return list->num_members;
}

int My402ListEmpty(My402List *list)
{
    if(list->num_members == 0) {
        return 1;
    }
    return 0;
}

int My402ListAppend(My402List *list, void *Entry)
{
    My402ListElem *listElem;

    listElem = malloc(sizeof(My402ListElem));
    memset(listElem, 0, sizeof(My402ListElem));
    listElem->obj = Entry;

    if(list->num_members == 0) {
        list->anchor.next = listElem;
        list->anchor.prev = listElem;
        listElem->next = NULL;
        listElem->prev = NULL;
        list->num_members = 1;
    } else {
        listElem->prev = list->anchor.prev;
        list->anchor.prev->next = listElem;
        list->anchor.prev = listElem;
        listElem->next = NULL;
        ++(list->num_members);
    }

    return 1;
}

void My402ListUnlink(My402List *list, My402ListElem* listEle)
{
    My402ListElem *temp;

    temp = list->anchor.next;
    while(temp != listEle) {
        temp = temp->next;
    }
    if(temp->next) {
        temp->next->prev = temp->prev;
    }
    if(temp->prev) {
        temp->prev->next = temp->next;
    }
    free(temp);
    --(list->num_members);
}

void My402ListUnlinkAll(My402List *list)
{
    My402ListElem *temp, *temp1;

    temp = list->anchor.next;
    while(temp) {
        temp1 = temp;
        temp =  temp->next;
        free(temp1);
    }
    list->anchor.next = NULL;
    list->anchor.prev = NULL;
    list->num_members = 0;
}

int My402ListInsertAfter(My402List* list, void *obj, My402ListElem* elem)
{

    My402ListElem *newelem;
    newelem = malloc(sizeof(My402ListElem));
    memset(newelem, 0, sizeof(My402ListElem));

    newelem->obj = obj;
    if(elem->next){
        elem->next->prev = newelem;
        newelem->prev = elem;
        newelem->next = elem->next;
        elem->next = newelem;

    } else {
        elem->next = newelem;
        newelem->next = NULL;
        newelem->prev = elem;
        list->anchor.prev = newelem;
    }
    ++(list->num_members);

    return 1;

}

int My402ListInsertBefore(My402List* list, void *obj, My402ListElem* elem)
{
    My402ListElem  *newelem;
    newelem = malloc(sizeof(My402ListElem));
    memset(newelem, 0, sizeof(My402ListElem));
    newelem->obj = obj;

    if(elem->prev) {
        elem->prev->next = newelem;
        newelem->prev = elem->prev;
        elem->prev = newelem;
        newelem->next = elem;
    } else{
        newelem->next = elem;
        elem->prev = newelem;
        newelem->prev = NULL;
        list->anchor.next = newelem;
    }
   ++(list->num_members);

    return 1;

}

My402ListElem *My402ListFirst(My402List* list)
{
    return list->anchor.next;

}

My402ListElem *My402ListLast(My402List* list)
{
    return list->anchor.prev;

}

My402ListElem *My402ListNext(My402List *list, My402ListElem* listElem)
{
    return listElem->next;
}

My402ListElem *My402ListPrev(My402List *list, My402ListElem *listElem)
{
    return listElem->prev;
}

My402ListElem *My402ListFind(My402List *list, void* obj)
{
    My402ListElem *temp;


}


int My402ListInit(My402List *list)
{

    list->num_members = 0;
    list->anchor.next = NULL;
    list->anchor.prev = NULL;
    list->anchor.obj = NULL;

    return 1;
}

/*void PrintOutput(My402List *list)
{
    My402ListElem* tempelem;

    tempelem = list->anchor.next;

*/
        
/*
 * Will be keeping place holders for now
 */

int main(int argc, char *argv[])
{
    int res = 0;

   /* if(argc < 2){
        printf("Where are is file\n");
        return 0;
    }*/


    /*
     * I will first initialize the list
     */

    My402List list;
    memset(&list, 0, sizeof(My402List));
    res = My402ListInit(&list);

    if(!res) {
        printf("Error during Initialization of the list\n");
        exit(0);
    }

    /*
     * We will now call the parse function
     */

    My402Input(&list, "test.tfile");

    My402ListUnlinkAll(&list);

    exit(0);
}




