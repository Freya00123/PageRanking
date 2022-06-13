// Weighted pagerank
// Written by:Heqing Huang
// Date: 2021.11.15

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// defalut number
#define DEFAULT 9999
#define MAX_LINE_LENGTH 128
#define BUFF_SIZE 10000

// Node struct 
struct OutNode {
    struct OutNode *next;
    struct Node *to;
};
typedef struct OutNode *OutNode;

struct InNode {
    struct InNode *next;
    struct Node *from;
};
typedef struct InNode *InNode;

struct Node{
    struct Node *next;
    struct InNode *in;
    struct OutNode *out;
    int outCount;
    int inCount;
    char *name;
    double PR[10000];
};
typedef struct Node *Node;

// list struct
struct list{
    Node head;
    Node tail;
    int size;
};
typedef struct list *List;

struct outlist {
    OutNode head;
    OutNode tail;
    int out_size;
};
typedef struct outlist *OutList;

struct inlist {
    InNode head;
    InNode tail;
    int in_size;
};
typedef struct inlist *InList;

// helper functions
Node NodeNew (char *name);
void NodeFree(Node n);

List ListNew(void);
void ListFree(List l);
int ListAppend(List l, Node n);
void ListInsertInOrder (List l, Node n, int iter);


InNode InNodeNew();
void InNodeFree(InNode in);

InList InListNew(void);
void InListFree(InList in_l);
int InListAppend(InList in_l, InNode in);
InList GetInList (InNode in);


OutNode OutNodeNew();
void OutNodeFree(OutNode on);

OutList OutListNew(void);
int OutListAppend (OutList out_l, OutNode on);
void OutListFree(OutList out_l);


List GetCollection(FILE *fp, List l);
void LinkList(List l);

int main(int argc, char *argv[]) {

    // get the urls in the collection file
    FILE *fp;
    fp = fopen("collection.txt", "r");
    FILE *output;
    output = fopen("pagerankList.txt", "w+");
    
    // get the collection and their data
    List l = ListNew();
    GetCollection(fp, l);
    LinkList(l);
    
    if (argc != 4) {
        fprintf(stderr, "insufficient arguments!\n");
        exit(EXIT_FAILURE);
    } 

    // take the commandline argument
    double d = atof(argv[1]);
    double diffPR = atof(argv[2]);
    double max_Iteration = atof(argv[3]);
    double diff = diffPR;

    // initiate all url's PR to 1/N at first
    double N = l->size;
    Node n = l->head;
    for (int i = 0;i < N; i++) {
        n->PR[0] = 1/N;
        n = n->next;
    }

    // calculate pagerank
    int iter;
    for (iter = 0; iter < max_Iteration && diff >= diffPR; iter++) {
        double diff_sum = 0;
        // loop through the collection list
        for (Node pi = l->head; pi != NULL; pi = pi->next) {
            double sum = 0;
            double Win = 0;
            double Wout = 0;
            // for every node we check the node that point to it
            // that is pi's innode
            for (InNode pj = pi->in; pj != NULL; pj = pj->next){
                double sum_win = 0;
                double sum_wout = 0;
                // calculate the innode's outcount
                for (OutNode pk = pj->from->out; pk != NULL; pk = pk->next) {
                    // if the count is 0, make it 0.5
                    if (pk->to->outCount == 0) {                        
                        sum_wout += 0.5;
                        sum_win += pk->to->inCount;
                    } else if (pk->to->inCount == 0) {                       
                        sum_win += 0.5;
                        sum_wout += pk->to->outCount;
                    } else if (pk->to->inCount == 0 && pk->to->outCount == 0) {                       
                        sum_win += 0.5;
                        sum_wout += 0.5;
                    } else {                       
                        sum_win += pk->to->inCount;
                        sum_wout += pk->to->outCount;
                    }
                }
                // same for the innode's count
                if (pi->inCount == 0) {
                    Win = 0.5 / sum_win;
                    Wout = pi->outCount / sum_wout;                   
                } else if (pi->outCount == 0) {
                    Win = pi->inCount / sum_win;
                    Wout = 0.5 / sum_wout;                   
                } else if (pi->inCount == 0 && pi->inCount == 0) {
                    Win = 0.5 / sum_win;
                    Wout = 0.5 / sum_wout;                   
                } else {                    
                    Win = pi->inCount / sum_win;
                    Wout = pi->outCount / sum_wout;                    
                }    
                sum = sum + (pj->from->PR[iter]*Win*Wout);
            }
            pi->PR[iter + 1] = (1-d) / N + d * sum;
            diff_sum = fabs(diff_sum + pi->PR[iter + 1] - pi->PR[iter]);
        }
        diff = diff_sum;
    }

    // create a ordered list based on PR in descending order
    List PR_l = ListNew();
    Node x = l->head;
    for (int k = 0; k < l->size; k++) {
        ListInsertInOrder(PR_l, x, iter);
        x = x->next;
    }

    // write into file "pagerankList.txt"
    Node z = PR_l->head;
    for (int k = 0; k < l->size; k++) {
        fprintf (output, "%s, %d, %.7lf\n", z->name, z->outCount, z->PR[iter]);
        z = z->next;
    }
    fclose(output);
    
    ListFree(PR_l);
    ListFree(l);
    return 0;
}

/////////////////////////////////////////////////////////////////
//************************ Help Function **********************// 
/////////////////////////////////////////////////////////////////

// append a node at the end of the list, return the size of the list
int ListAppend(List l, Node n) {
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

// insert into the list in oeder of PR in descending order
void ListInsertInOrder(List PR_l, Node m, int iter) {
    // copy the node into a new node
    Node n = NodeNew(m->name);
    n->outCount = m->outCount;
    n->PR[iter] = m->PR[iter];

    // if empty list 
    if (PR_l->size == 0) {
        PR_l->head = n;
        PR_l->tail = n;
    } 
    // if the node needed to insert is the smallest number
    else if (n->PR[iter] <= PR_l->tail->PR[iter]) {
        PR_l->tail->next = n;
        PR_l->tail = n;
    }
    // if the inserted node is the largest number
    else if (n->PR[iter] >= PR_l->head->PR[iter]) {
        n->next = PR_l->head;
        PR_l->head = n;
    }
    // if the inserted node is in the middle
    else {
        struct Node *curr = PR_l->head;
        Node prev = NULL;
        // search from the head of the list
        for (int i = 0; i < PR_l->size; i++) {
            prev = curr;
            if (n->PR[iter] >= curr->next->PR[iter]) {
                n->next = prev->next;
                prev->next = n;
                break;
            } 
            curr = curr->next; 
        }
    }
    PR_l->size++;
}

// create a new node
Node NodeNew(char *name) {
    Node n = calloc(1, sizeof(*n));
    if (n == NULL) {
        fprintf(stderr, "Insufficient memory!\n");
        exit(EXIT_FAILURE);
    }
    n->name = strdup(name);
    for(int i=0;i<10000;i++) {
        n->PR[i] = 0;
    }
    n->next = NULL;
    n->inCount = 0;
    n->outCount = 0;
    return n;
}

// create a new list
List ListNew(void) {
    List l = calloc(1, sizeof(*l));
    if (l == NULL) {
		fprintf(stderr, "couldn't allocate List\n");
		exit(EXIT_FAILURE);
	}
    l->head = NULL;
    l->tail = NULL;
    return l;
}

// get list of the urls in file, and return the list
List GetCollection(FILE *fp, List l) {
    if (fp == NULL) {
        fprintf (stderr, "Can't open %s\n", "collection.txt");
        exit(1);
    }
    
    char line[MAX_LINE_LENGTH + 1];
    // loop through every linne in the file
    while (fgets(line, MAX_LINE_LENGTH, fp)) {
        char *line_string = strdup(line);
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
            if (strcmp(curr_token, "\n") == 0) {
                continue;
            }
            // append url to the list
            Node n = NodeNew(curr_token);       
            ListAppend(l, n);
        }
        free(curr_token);
        free(next_token);
        free(line_string);
    }
    fclose(fp);
    return l;
}

// create a new innode
InNode InNodeNew() {
    InNode in = calloc(1, sizeof(*in));
    if (in == NULL) {
        fprintf(stderr, "Insufficient memory!\n");
        exit(EXIT_FAILURE);
    }
    in->from = NULL;
    in->next = NULL;
    return in;
}

// create a new inlist
InList InListNew(void) {
    InList in_l = calloc(1, sizeof(*in_l));
    if (in_l == NULL) {
        fprintf(stderr, "couldn't allocate List\n");
		exit(EXIT_FAILURE);
    }
    in_l->head = NULL;
    in_l->tail = NULL;
    in_l->in_size = 0;
    return in_l;
}

// append the node at the end of the inlist, return the size of the inlist
int InListAppend(InList in_l, InNode in) {
    assert (in_l != NULL) ;
    if (in_l->head == NULL) {
        in_l->head = in_l->tail = in;
    } else {
        in_l->tail->next = in;
        in_l->tail = in;
    }
    in_l->in_size++;
    return in_l->in_size;
}

// get the inlist of the given innode
InList GetInList (InNode in) {
    InList inlist = InListNew();
    while (in != NULL){
        InListAppend(inlist, in);
        in = in->next;
    }
    return inlist;
}

// create a new outnode
OutNode OutNodeNew() {
    OutNode on = calloc(1, sizeof(*on));
    if (on == NULL) {
        fprintf(stderr, "Insufficient memory!\n");
        exit(EXIT_FAILURE);
    }
    on->to = NULL;
    on->next = NULL;
    return on;
}

// create a new outlist
OutList OutListNew(void) {
    OutList out_l = calloc(1, sizeof(*out_l));

    if (out_l == NULL) {
        fprintf(stderr, "couldn't allocate List\n");
		exit(EXIT_FAILURE);
    }
    out_l->head = NULL;
    out_l->tail = NULL;
    out_l->out_size = 0;
    return out_l;
}

// append node at the end of the list, return the size of outlist
int OutListAppend (OutList out_l, OutNode on) {
    assert (out_l != NULL) ;
    if (out_l->head == NULL) {
        out_l->head = out_l->tail = on;
    } else {
        out_l->tail->next = on;
        out_l->tail = on;
    }
    out_l->out_size++;
    return out_l->out_size;
}

// free node
void NodeFree(Node n) {
    if (n != NULL) {
        InNodeFree(n->in);
        OutNodeFree(n->out);
        free(n->name);
        free(n);
    }
}

// free innode
void InNodeFree(InNode in) {
    if (in != NULL) {
        InNodeFree(in->next);
        free(in);        
    }
}

// free outnode
void OutNodeFree(OutNode on) {
    if (on != NULL) {
        OutNodeFree(on->next);
        free(on);        
    }
}

// free list
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

// free inlist
void InListFree (InList in_l) {
    if (in_l == NULL) return;
    free(in_l);
}

// free outlist
void OutListFree(OutList out_l) {
    if (out_l == NULL) return;
    free(out_l);
}

// get the data of the given url
void LinkList (List l) {
    Node curr_ref = l->head;
    FILE *fp;

    // loop every url in the list
    while (curr_ref != NULL) {
        // get the name of
        char curr[BUFF_SIZE];
        strcpy(curr, curr_ref->name);
        char filename[BUFF_SIZE];                
        strcpy(filename, curr);
        strcat(filename, ".txt");

        fp = fopen(filename, "r");
        if (fp == NULL) {
            fprintf (stderr, "Can't open %s\n", "collection.txt");
            exit(1);
        }
        char line[MAX_LINE_LENGTH + 1];
        OutList outlist = OutListNew();
        // terminator for entering an file with empty outlinks
        double termi = 0;
        // loop every line in the url.txt 
        while (fgets(line, MAX_LINE_LENGTH, fp)) {
            char *line_string = strdup(line);

            // break if the oulist already exists or reach end of the section 1
            if (outlist->head != NULL || termi != 0) {
                free(line_string);
                break;
            }

            // toke the line
            char *curr_token = NULL;
            char *next_token = NULL;
            char *token = strtok(line_string, " \n");
            if (token != NULL) {
                next_token = strdup(token);
            }
            // loop through each string
            while (next_token != NULL) {
                if (curr_token != NULL) {
                    free(curr_token);
                }                
                curr_token = next_token;
                token = strtok(NULL, " \n");
                if (token != NULL) {
                    next_token = strdup(token);
                } else {
                    next_token = NULL;
                }

                // skip the useless string
                // and terminate the loop once reach the end
                if (strcmp(curr_token, "\n") == 0 || strcmp(curr_token, curr) == 0) {
                    continue;
                } else if (strcmp(curr_token, "#start") == 0 || strcmp(curr_token, "Section-1") == 0) {
                    continue;
                } else if (strcmp(curr_token, "#end") == 0 && outlist->out_size == 0) {
                    termi = 0.5;
                    break;
                }
                // for useful string
                // loop through the collection list
                Node find = l->head;               
                OutNode on = OutNodeNew(); 
                InNode inn = InNodeNew();
                InList in_l;
                for (int i = 0; i < l->size; i++) {
                    // if we find the url in the list
                    if (strcmp(find->name, curr_token) == 0) {
                        // initiate innode and outnode
                        // make the outnode point to the url
                        // and innode point to the curr_ref
                        on->to = find;
                        inn->from = curr_ref;
                        // get the inlist of the find node
                        in_l = GetInList(find->in);
                        // append inn to the inlist
                        find->inCount = InListAppend(in_l, inn);
                        find->in = in_l->head;
                        InListFree(in_l);
                        break;
                    }
                    find = find->next;
                }
                // record the outcount
                curr_ref->outCount = OutListAppend(outlist, on);
            }
            free(curr_token);
            free(next_token);
            free(line_string);
        }
        // make the curr_ref's out point to the head of the oulist
        curr_ref->out = outlist->head; 
        OutListFree(outlist);
        curr_ref = curr_ref->next;
        fclose(fp);
    }
}



