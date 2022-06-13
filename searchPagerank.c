// Simple search engine
// Written by: Heqing Huang
// Date: 19.11.21

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// default number
#define DEFAULT 9999
#define MAX_LINE_LENGTH 128
#define BUFF_SIZE 1000

// Struct definition 

// Node struct 
struct Node {
    struct Node *next;
    char *word;
    struct Node *out;
    int count;
    double PR;
};
typedef struct Node *Node;
// List struct
struct List {
    Node head;
    Node tail;
    int size;
};
typedef struct List *List;

Node NodeNew (char *name);
List ListNew ();
int ListAppend(List l, Node n);
List GetInvertedIndex(FILE *fp, List l);
List UrlContainARGV (List l, char *array[]); 
void AddPR(List l, FILE *fp);
void ListFree(List l);
void NodeFree(Node n);
void ListInsertInOrder(List l, Node n);

int main(int argc, char *argv[]) {
    // take the command line argument
    FILE *fp;
    fp = fopen("invertedIndex.txt", "r");

    List l = ListNew();
    // get the list of invertedIndex
    // word list contains the word and their out link with url
    GetInvertedIndex(fp, l);

    // we take the command lien argument
    char *SearchWord[MAX_LINE_LENGTH + 1];
    for (int i = 0; i < argc; i++) {
        SearchWord[i] = argv[i + 1];
    }
    
    // make a list with url which contains the word
    List url_list = UrlContainARGV(l, SearchWord); 
    
    // add the PR to the node in the list
    FILE *PR_fp;
    PR_fp = fopen("pagerankList.txt", "r");
    AddPR(url_list, PR_fp);
    
    List order_l = ListNew();
    for (Node n = url_list->head; n != NULL; n = n->next) {
        ListInsertInOrder(order_l, n);
    }
    
    for (Node n = order_l->head; n != NULL; n = n->next) {
        printf ("%s\n", n->word);
    }
    
    
    ListFree(l);
    ListFree(url_list);
    ListFree(order_l);
    return 0;
}

/////////////////////////////////////////////////////////////////
//************************ Help Function **********************// 
/////////////////////////////////////////////////////////////////
Node NodeNew (char *name) {
    Node n = malloc(sizeof(*n));
    if (n == NULL) {
        fprintf(stderr, "Insufficient memory!\n");
        exit(EXIT_FAILURE);
    }
    n->word = strdup(name);
    n->next = NULL;
    n->out = NULL;
    n->count = 0;
    n->PR = 0;
    return n;
}

void NodeFree(Node n) {
    if (n != NULL) {
        free(n->word);
        free(n);
    }
}

List ListNew () {
    List l = malloc(sizeof(*l));
    if (l == NULL) {
		fprintf(stderr, "couldn't allocate List\n");
		exit(EXIT_FAILURE);
	}
    l->head = NULL;
    l->tail = NULL;
    l->size = 0;
    return l;
}

void ListFree(List l) {
    if (l == NULL) return;
    struct Node *curr = l->head;
    while (curr != NULL) {
        Node temp = curr;
        curr = curr->next;
        NodeFree(temp);
    }
    free(l);
}

int ListAppend (List l, Node n) {
    assert (l != NULL) ;
    if (l->head == NULL) {
        l->head = l->tail = n;
    } else {
        l->tail->next = n;
        l->tail = n;
    }
    l->size++;
    return l->size;
}

List GetInvertedIndex(FILE *fp, List l) {
    if (fp == NULL) {
        fprintf (stderr, "Can't open file: %s\n", "invertedIndex.txt");
        exit(1);
    }
    char line[MAX_LINE_LENGTH + 1];
    while(fgets(line, MAX_LINE_LENGTH, fp)) {
        char *line_string = strdup(line);
        int termi = 0;
        char *curr_token = NULL;
        char *next_token = NULL;
        char *token = strtok(line_string, " \n");
        if (token != NULL) {
            next_token = strdup(token);
        } 
        // keep toke the string of the line
        while (next_token != NULL) {
            if (curr_token != NULL) {
                free(curr_token);
            }
            // ignore the new line character and white space 
            curr_token = next_token;
            token = strtok(NULL, " \n");
            if (token != NULL) {
                next_token = strdup(token);
            } else {
                next_token = NULL;
            }
            if (strcmp(curr_token, " \n") == 0) {
                continue;
            }
            if (termi == 1) {
                Node word = l->tail;
                if (word->out != NULL) {
                    Node reg = word->out;
                    while (reg->next != NULL) {
                        reg = reg->next;
                    }
                    reg->next = NodeNew(curr_token);
                    continue;
                } else {
                    word->out = NodeNew(curr_token);
                    continue;
                }
            }
            Node n = NodeNew(curr_token);
            ListAppend(l, n);
            termi = 1;
        }
        free(curr_token);
        free(next_token);
        free(line_string);
    }
    fclose(fp);
    return l;
}    
    
List UrlContainARGV (List l, char *array[])  {
    List url_list = ListNew();
    for (int i = 0; array[i] != NULL; i++) {
        for (Node curr = l->head; curr != NULL; curr = curr->next) {
            if (strcmp(curr->word, array[i]) == 0) {
                for (Node url = curr->out; url != NULL; url = url->next) {
                    int termi = 0;
                    // if the url node has already exists in the url_list;
                    // increase the count only
                    for (Node n = url_list->head; n != NULL; n = n->next) {
                        if (strcmp(n->word, url->word) == 0) {
                            termi = 1;
                            n->count += 1;
                            break;
                        }
                    }
                    // if not in the list
                    if (termi == 0) {
                        Node n = NodeNew(url->word);
                        n->count += 1;
                        ListAppend(url_list, n);
                    }
                    
                }
            }
        }
    }
    return url_list;
}

void ListInsertInOrder(List l, Node m) {
    // copy the node into a new node
    
    Node n = NodeNew(m->word);
    n->count = m->count;
    n->PR = m->PR;

    // if empty list 
    if (l->size == 0) {
        l->head = n;
        l->tail = n;
    } 

    // if the node needed to insert is the smallest number
    else if (n->count <= l->tail->count) {
        l->tail->next = n;
        l->tail = n;
    }

    // if the inserted node is the largest number
    else if (n->count >= l->head->count) {
        n->next = l->head;
        l->head = n;
    }

    // if the inserted node is in the middle
    else {
        struct Node *curr = l->head;
        Node prev = NULL;
        // search from the head of the list
        for (int i = 0; i < l->size; i++) {
            prev = curr;
            if (n->count > curr->next->count) {
                n->next = prev->next;
                prev->next = n;
                break;
            } else if (n->count == curr->next->count) {
                if (n->PR > curr->next->PR) {
                    n->next = prev->next;
                    prev->next = n;
                    break;
                } else if (n->PR == curr->next->PR) {
                    // if both count and PR are the same
                    // order the name alphabetically
                    if (strcmp(n->word, curr->next->word) > 0) {
                        n->next = prev->next;
                        prev->next = n;
                        break;
                    } 
                }
            }
            curr = curr->next; 
        }
    }
    l->size++;
}

void AddPR(List l, FILE *fp) {
    if (fp == NULL) {
            fprintf (stderr, "Can't open file: %s\n", "invertedIndex.txt");
            exit(1);
    }
    char line[MAX_LINE_LENGTH + 1];
    while(fgets(line, MAX_LINE_LENGTH, fp)) {
        
        
        char *line_string = strdup(line);
        
        char *curr_token = NULL;
        char *next_token = NULL;
        char *token = strtok(line_string, ", \n");
        if (token != NULL) {
            next_token = strdup(token);
        } 
        // keep toke the string of the line
        while (next_token != NULL) {
            if (curr_token != NULL) {
                free(curr_token);
            }
            // ignore the new line character and white space and comma
            curr_token = next_token;
            token = strtok(NULL, ", \n");
            if (token != NULL) {
                next_token = strdup(token);
            } else {
                next_token = NULL;
            }
            if (strcmp(curr_token, ", \n") == 0) {
                continue;
            }  
        
            for (Node n = l->head; n != NULL; n = n->next) {
                if (strcmp(n->word, curr_token) == 0) {
                    if (strlen(next_token) == 1) {
                        token = strtok(NULL, ", \n");
                        next_token = strdup(token);
                    }
                    n->PR = atof(next_token); 
                }
            }

        }
        free(curr_token);
        free(next_token);
        free(line_string);
    }
    fclose(fp);
}