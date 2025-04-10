#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


/*------------------------  BSTreeType  -----------------------------*/
typedef BinarySearchTree* (*AccessFunction)(const BinarySearchTree*);
typedef enum{red, black} NodeColor;

struct _bstree {
    BinarySearchTree* parent;
    BinarySearchTree* left;
    BinarySearchTree* right;
    int key;
    NodeColor color;
};

// pour ne pas avoir de problème de déclaratin lors de la commande make
BinarySearchTree* fixredblack_insert(ptrBinarySearchTree x);
BinarySearchTree* fixredblack_insert_case1(ptrBinarySearchTree x);
BinarySearchTree* fixredblack_insert_case2(ptrBinarySearchTree x);
BinarySearchTree* fixredblack_insert_case2_right(ptrBinarySearchTree x);
BinarySearchTree* fixredblack_insert_case2_left(ptrBinarySearchTree x);

BinarySearchTree* fixredblack_remove(BinarySearchTree* p, BinarySearchTree* x);
BinarySearchTree* fixredblack_remove_case1_left(BinarySearchTree* p);
BinarySearchTree* fixredblack_remove_case1_right(BinarySearchTree* p);
BinarySearchTree* fixredblack_remove_case1(BinarySearchTree* p, BinarySearchTree* x);
BinarySearchTree* fixredblack_remove_case2_left(BinarySearchTree* p);
BinarySearchTree* fixredblack_remove_case2_right(BinarySearchTree* p);
BinarySearchTree* fixredblack_remove_case2(BinarySearchTree* p, BinarySearchTree* x);


/*------------------------  BaseBSTree  -----------------------------*/

BinarySearchTree* bstree_create(void) {
    return NULL;
}

/* This constructor is private so that we can maintain the oredring invariant on
 * nodes. The only way to add nodes to the tree is with the bstree_add function
 * that ensures the invariant.
 */
BinarySearchTree* bstree_cons(BinarySearchTree* left, BinarySearchTree* right, int key) {
    BinarySearchTree* t = malloc(sizeof(struct _bstree));
    t->parent = NULL;
    t->left = left;
    t->right = right;
    if (t->left != NULL)
        t->left->parent = t;
    if (t->right != NULL)
        t->right->parent = t;
    t->key = key;
    t->color = red;
    return t;
}

void freenode(const BinarySearchTree* t, void* n) {
    (void)n;
    free((BinarySearchTree*)t);
}

void bstree_delete(ptrBinarySearchTree* t) {
    bstree_depth_postfix(*t, freenode, NULL);
}

bool bstree_empty(const BinarySearchTree* t) {
    return t == NULL;
}

int bstree_key(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->key;
}

BinarySearchTree* bstree_left(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->left;
}

BinarySearchTree* bstree_right(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->right;
}

BinarySearchTree* bstree_parent(const BinarySearchTree* t) {
    assert(!bstree_empty(t));
    return t->parent;
}

/*------------------------  BSTreeDictionary  -----------------------------*/

/* Obligation de passer l'arbre par référence pour pouvoir le modifier */
void bstree_add(ptrBinarySearchTree* t, int v) {
    assert(t != NULL);
    if (*t == NULL) {
        *t = bstree_cons(NULL, NULL, v);
        (*t)->color = black;  // racine toujours noire
    }

    ptrBinarySearchTree* current = t;
    BinarySearchTree* parent = NULL;

    // parcours pour trouver l'emplacement où ajouter le nouveaux noeuds
    while (*current != NULL) {
        parent = *current;
        if (v < (*current)->key) {
            current = &(*current)->left;

        } else if (v > (*current)->key) {
            current = &(*current)->right;

        } else{
            return;
        }
    }

    // création du nouveau noeud et mise à jour des liens
    BinarySearchTree* newNode = bstree_cons(NULL, NULL, v);
    newNode->parent = parent;
    newNode->color = red;

    if (v < parent->key) {
        parent->left = newNode;
    } else {
        parent->right = newNode;
    }
    BinarySearchTree* stop = fixredblack_insert(newNode);
    if (stop->parent == NULL){
        *t = stop;
    }
}

const BinarySearchTree* bstree_search(const BinarySearchTree* t, int v) {
    // parcourt de tout l'arbre
    while(t != NULL){
        if(v < t->key){
            t = t->left;
        } else if(v > t->key){
            t = t->right;
        } else{
            // si c'est pas à gauche ou pas à droite on est dessus alors on renvoie t
            return t;
        }
    }
    return NULL;
}

void bstree_swap_nodes(ptrBinarySearchTree* tree, ptrBinarySearchTree from, ptrBinarySearchTree to) {
    assert(!bstree_empty(*tree) && !bstree_empty(from) && !bstree_empty(to));
    BinarySearchTree *parentTemp = from->parent;
    from->parent = to->parent;
    to->parent = parentTemp;

    if (to->parent) {
        if (to->parent->left == from) to->parent->left = to;
        else to->parent->right = to;
    } else {
        *tree = to;
    }

    if (from->parent != NULL) {
        if (from->parent->left == to){
            from->parent->left = from;
        }
        else{
            from->parent->right = from;
        }
    } else {
        *tree = from;
    }

    

    BinarySearchTree* tmpRight = from->right;
    BinarySearchTree* tmpLeft = from->left;
    from->left = to->left;
    to->left = tmpLeft;
    from->right = to->right;
    to->right = tmpRight;
    
    if (from->left){
        from->left->parent = from;
    }

    if (to->left){
        to->left->parent = to;
    }

    if (from->right){
        from->right->parent = from;
    } 
    if (to->right){
        to->right->parent = to;
    } 
}

typedef struct {
    AccessFunction child_left;
    AccessFunction child_right;
} ChildAccessors;

BinarySearchTree* find_next(const BinarySearchTree* x, ChildAccessors access) {
    if (x == NULL) {
        return NULL;
    }

    BinarySearchTree* current = access.child_right(x);

    if (current != NULL) {
        while (access.child_left(current) != NULL) {
            current = access.child_left(current);
        }
    } else {
        current = x->parent;
        while (current != NULL && x == access.child_right(current)) {
            x = current;
            current = current->parent;
        }
    }

    return current;
}


const BinarySearchTree* bstree_successor(const BinarySearchTree* x) {
    ChildAccessors access = {.child_left = bstree_left, .child_right = bstree_right};
    return find_next(x, access);
}

const BinarySearchTree* bstree_predecessor(const BinarySearchTree* x) {
    ChildAccessors access = {.child_left = bstree_right, .child_right = bstree_left};
    return find_next(x, access);
}

// t -> the tree to remove from, current -> the node to remove
void bstree_remove_node(ptrBinarySearchTree *t, ptrBinarySearchTree current) {
    assert(!bstree_empty(*t) && !bstree_empty(current));
    BinarySearchTree* change = NULL;
    if (current->left == NULL && current->right == NULL){
        change = NULL;
    } else if (current->right == NULL){
        change = current->left;
    }
    else if (current->left == NULL){
        change = current->right;
    }
    else {
       BinarySearchTree* successeur = (BinarySearchTree*)bstree_successor(current); // cast pour éviter le problème avec le const
        bstree_swap_nodes(t, current, successeur);
        // échanger les couleurs suite au swap
        NodeColor couleur = current->color;
        current->color = successeur->color;
        successeur->color = couleur;
        change = current->right;
    }

    if (change != NULL) {
        change->parent = current->parent;
    }

    if (!current->parent){
        *t = change;
    } else if (current->parent->left == current){
        current->parent->left = change;
    } else {
        current->parent->right = change;
    }

    /* fix the redblack properties if needed */
    if(current == NULL || current->color == black){
        if(change == NULL || change->color == black){
            /* substitute is double black : must fix */
            ptrBinarySearchTree subtreeroot = fixredblack_remove(current->parent, change);
            if(subtreeroot->parent == NULL){
                *t = subtreeroot;
            }
        } else{
            /* substitute becomes black */
            change->color = black;
        }
    }
    /* free the memory */
    free(current);
}

void bstree_remove(ptrBinarySearchTree *t, int v) {
    BinarySearchTree *courant = *t;

    while (courant != NULL && courant->key != v) {
        if (v < courant->key){
            courant = courant->left;
        }else{
            courant =  courant->right;
        }
    }
    if(courant == NULL){
        return;
    }
    bstree_remove_node(t, courant);
}

/*------------------------  BSTreeVisitors  -----------------------------*/

void bstree_depth_prefix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (t == NULL) {
        return; 
    }
    // prefix = des qu'on voit on traite donc on applique le foncteur en premier
    f(t, environment); 
    bstree_depth_prefix(t->left, f, environment);  // parcourt du s.a.g
    bstree_depth_prefix(t->right, f, environment);  // parcourt du s.a.d
}

void bstree_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (t == NULL) {
        return; 
    }
    // infix = des qu'on traverse on traite donc on applique le foncteur entre le s.a.g et le s.a.d
    bstree_depth_infix(t->left, f, environment);  // parcourt du s.a.g
    f(t, environment);
    bstree_depth_infix(t->right, f, environment);  // parcourt du s.a.d
}

void bstree_depth_postfix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (t == NULL) {
        return;
    }
    // postfix = des qu'on remonte on traite donc on applique le foncteur après le s.a.g et s.a.d
    bstree_depth_postfix(t->left, f, environment);  // Parcourir le sous-arbre gauche
    bstree_depth_postfix(t->right, f, environment);  // Parcourir le sous-arbre droit
    f(t, environment);
}

void bstree_iterative_breadth(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    if (t == NULL) {
        return;
    }
    Queue* queue = create_queue();  
    queue_push(queue, t);

    // parcourt des éléments de la queue
    while (!queue_empty(queue)) {
        BinarySearchTree* current = (BinarySearchTree*)(queue_top(queue));
        queue_pop(queue);

        f(current, environment);

        if (current->left != NULL) {
            queue_push(queue, current->left);
        }
        if (current->right != NULL) {
            queue_push(queue, current->right);
        }
    }
    delete_queue(&queue);
}


void bstree_iterative_depth_infix(const BinarySearchTree* t, OperateFunctor f, void* environment) {
    const BinarySearchTree *current = t;
    const BinarySearchTree *next = t->parent;
    const BinarySearchTree *prev = t->parent;

    while (current != NULL) {
        if (prev == current->parent) {
            prev = current;
            next = current->left;
        }
        if (prev == current->left || next == NULL ) {
            f(current, environment);
            prev = current;
            next = current->right;
        }
        if (prev == current->right || next == NULL) {
            prev = current;
            next = current->parent;
        }
        current = next;
    }
}

/*------------------------  BSTreeIterator  -----------------------------*/

struct _BSTreeIterator {
    /* the collection the iterator is attached to */
    const BinarySearchTree* collection;
    /* the first element according to the iterator direction */
    const BinarySearchTree* (*begin)(const BinarySearchTree* );
    /* the current element pointed by the iterator */
    const BinarySearchTree* current;
    /* function that goes to the next element according to the iterator direction */
    const BinarySearchTree* (*next)(const BinarySearchTree* );
};

/* minimum element of the collection */
const BinarySearchTree* goto_min(const BinarySearchTree* e) {
    const BinarySearchTree* noeudCourant = e;
    // comme on est dans un ABR le noeud min est tout à gauche 
    while(noeudCourant->left != NULL){
        noeudCourant = noeudCourant->left;
    }

    return noeudCourant;
}

/* maximum element of the collection */
const BinarySearchTree* goto_max(const BinarySearchTree* e) {
	const BinarySearchTree* noeudCourant = e;
    // comme on est dans un ABR le noeud max est tout à droite 
    while(noeudCourant->right != NULL){
        noeudCourant = noeudCourant->right;   
    }

    return noeudCourant;
}

/* constructor */
BSTreeIterator* bstree_iterator_create(const BinarySearchTree* collection, IteratorDirection direction) {
    BSTreeIterator* iterateur = malloc(sizeof(BSTreeIterator));
    iterateur->collection = collection;
    if (direction == forward){
        iterateur->current = goto_min(collection);
        iterateur->next = bstree_successor;
        iterateur->begin = goto_min;
    } else if(direction == backward){
        iterateur->current = goto_max(collection);
        iterateur->next = bstree_predecessor;
        iterateur->begin = goto_max;
    } else{
        free(iterateur);
    }
    return iterateur;
}

/* destructor */
void bstree_iterator_delete(ptrBSTreeIterator* i) {
    free(*i);
    *i = NULL;
}

BSTreeIterator* bstree_iterator_begin(BSTreeIterator* i) {
    i->current = i->begin(i->collection);
    return i;
}

bool bstree_iterator_end(const BSTreeIterator* i) {
    return i->current == NULL;
}

BSTreeIterator* bstree_iterator_next(BSTreeIterator* i) {
    i->current = i->next(i->current);
    return i;
}

const BinarySearchTree* bstree_iterator_value(const BSTreeIterator* i) {
    return i->current;
}

/*------------------------  RedBlackTrees  -----------------------------*/


/*------------------------  Exercice 1 : Coloration  -----------------------------*/


void bstree_node_to_dot(const BinarySearchTree* t, void* stream){
    FILE *file = (FILE *) stream;

    
    fprintf(file, "\tn%d [label=\"{%d|{<left>|<right>}}\", %s];\n",
            bstree_key(t), 
            bstree_key(t), 
            t->color == red ? "style=filled, fillcolor=red" : "style=filled, fillcolor=grey");
            

    if (bstree_left(t)) {
        fprintf(file, "\tn%d:left:c -> n%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(bstree_left(t)));
    } else {
        fprintf(file, "\tlnil%d [style=filled, fillcolor=grey, label=\"NIL\"];\n", bstree_key(t));
        fprintf(file, "\tn%d:left:c -> lnil%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(t));
    }
    if (bstree_right(t)) {
        fprintf(file, "\tn%d:right:c -> n%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(bstree_right(t)));
    } else {
        fprintf(file, "\trnil%d [style=filled, fillcolor=grey, label=\"NIL\"];\n", bstree_key(t));
        fprintf(file, "\tn%d:right:c -> rnil%d:n [headclip=false, tailclip=false]\n",
                bstree_key(t), bstree_key(t));
    }
}

/*------------------------  Exercice 2 : Rotation  -----------------------------*/

void leftrotate(BinarySearchTree *x){
    assert(x != NULL);
    BinarySearchTree* y = x->right;
    if(y->left == NULL && y->right == NULL && y == NULL){
        return;
    }

    x->right = y->left;

    if(y->left != NULL){
        y->left->parent = x;
    }

    y->parent = x->parent;

    if(x->parent != NULL){
        if(x == x->parent->left){
            x->parent->left = y;
        } else{
            x->parent->right = y;
        }
    }
    y->left = x;
    x->parent = y;
}

void rightrotate(BinarySearchTree *y){
    assert(y != NULL);
    BinarySearchTree* x = y->left;
    if (y->left == NULL) {
        return; 
    }

    y->left = x->right;

    if(x->right != NULL){
        x->right->parent = y;
    }

    x->parent = y->parent;

    if(y->parent != NULL){
        if(y == y->parent->left){
            y->parent->left = x;
        } else{
            y->parent->right = x;
        }
    }

    x->right = y;
    y->parent = x;
}

void testrotateleft(BinarySearchTree* t){
    leftrotate(t);
}

void testrotateright(BinarySearchTree* t){
    rightrotate(t);
}

/*------------------------  Exercice 3 : Insertion  -----------------------------*/

BinarySearchTree* grandparent(BinarySearchTree* n){
    if (n != NULL && n->parent != NULL){
        return n->parent->parent;
    }
    return NULL;
}

BinarySearchTree* uncle(BinarySearchTree* n){
    BinarySearchTree* gP = grandparent(n);
    BinarySearchTree* parent = n->parent;

    if(gP != NULL){
        if(parent == gP->left){
            return gP->right;
        } else if(parent == gP->right){
            return gP->left;
        }
    }
    return NULL;
}


BinarySearchTree* fixredblack_insert(BinarySearchTree* x){
    if(x->parent != NULL && x->parent->color == red){
        if(grandparent(x) == NULL){
            x->parent->color = black;
            return x;
        } else{
            return fixredblack_insert_case1(x);
        }
    } else{
        // on remonte jusqu'a la racine car l'arbre est forcément équilibré si on passe dans le else et on color en noir la racine
        while(x->parent){
            x = x->parent;
        }
        x->color = black;
        return x;
    }
}

BinarySearchTree* fixredblack_insert_case1(BinarySearchTree* x){
    BinarySearchTree* grandParent = grandparent(x);
    BinarySearchTree* parent = x->parent;
    BinarySearchTree* oncle = uncle(x);
    if(oncle != NULL && oncle->color == red){
        grandParent->color = red;
        oncle->color = black;
        parent->color = black;
        return fixredblack_insert(grandParent);
    }
    return fixredblack_insert_case2(x);
}

BinarySearchTree* fixredblack_insert_case2_left(BinarySearchTree* x){
    BinarySearchTree* gP = grandparent(x);
    BinarySearchTree* parent = x->parent;
    if (parent->right == x) {
        leftrotate(parent);
        return fixredblack_insert_case2_left(parent);
    } else {
        rightrotate(gP);
        parent->color = black;
        gP->color = red;
        return parent;
    }
}

BinarySearchTree* fixredblack_insert_case2_right(BinarySearchTree* x){
    BinarySearchTree* gP = grandparent(x);
    BinarySearchTree* parent = x->parent;

    if (parent->left == x) {
        rightrotate(parent);
        return fixredblack_insert_case2_right(parent);
    } else {
        leftrotate(gP);
        parent->color = black;
        gP->color = red;

        return parent;
    }
}

BinarySearchTree* fixredblack_insert_case2(BinarySearchTree* x){
    BinarySearchTree* gP = grandparent(x);
    BinarySearchTree* parent = x->parent;

    if(gP->left == parent){
        return fixredblack_insert_case2_left(x);
    } else{
        return fixredblack_insert_case2_right(x);
    }
}

/*------------------------  Exercice 5 : Suppression  -----------------------------*/

BinarySearchTree* fixredblack_remove(ptrBinarySearchTree p, ptrBinarySearchTree x) {
    if(p == NULL) {
        if (x != NULL){
            x->color = black;
        }
        return x;
    }
    BinarySearchTree *f = NULL;
    // instruction d'initialisation de f comme frere de x
    if(p != NULL){
        if(p->left == x){
            f = p->right;
        } else{
            f = p->left;
        }
    }
    // appel des fonctions tiers
    if (f == NULL || f->color == black){
        return fixredblack_remove_case1(p, x);
    } else{
        return fixredblack_remove_case2(p, x);
    }
}

BinarySearchTree* fixredblack_remove_case1_left(BinarySearchTree* p){
    // suivre le pdf
    BinarySearchTree *x = p->left;
    BinarySearchTree *f = p->right;
    BinarySearchTree *g = f->left;
    BinarySearchTree *d = f->right;

    if ((g == NULL || g->color == black) && (d == NULL || d->color == black)){
        if (x != NULL){
            x->color = black;
        }
        f->color = red;
        if (p->color == red){
            p->color = black;
            return p->parent;
        }
        else {
            return fixredblack_remove(p->parent, p);
        }
    } else if(d != NULL && d->color == red){
        leftrotate(p);
        f->color = p->color;
        if (x != NULL){
            x->color = black;
        }
        p->color = black;
        d->color = black;
        return f;
    } else{
        rightrotate(f);
        g->color = black;
        if (d != NULL){
            d->color = red;
        }
        leftrotate(p);
        f->color = p->color;
        return g;
    }
    
}


BinarySearchTree* fixredblack_remove_case1_right(BinarySearchTree* p) {
    // symétrique a left
    BinarySearchTree* x = p->right;
    BinarySearchTree* f = p->left;
    BinarySearchTree* d = f->left;
    BinarySearchTree* g = f->right;
    
    if ((g == NULL || g->color == black) && (d == NULL || d->color == black)) {
        if (x != NULL) {
            x->color = black;
        }
        f->color = red;
        if (p->color == red) {
            p->color = black;
            return p->parent;
        } else {
            return fixredblack_remove(p->parent, p);
        }
    } else if (d != NULL && d->color == red) {
        rightrotate(p);
        f->color = p->color;
        if (x) {
            x->color = black;
        }
        p->color = black;
        d->color = black;
        return f;
    } else{
        leftrotate(f);
        g->color = black;
        if (d != NULL) {
            d->color = red;
        }
        rightrotate(p);
        f->color = p->color;
        return g;
    }
    
}

BinarySearchTree* fixredblack_remove_case1(BinarySearchTree* p, BinarySearchTree* x){
    if (p->left == x){
        return fixredblack_remove_case1_left(p);
    } else{
        return fixredblack_remove_case1_right(p);
    }
}

BinarySearchTree* fixredblack_remove_case2_left(BinarySearchTree* p){
    BinarySearchTree* frere = p->right;

    //frère rouge
    leftrotate(p);
    p->color = red;
    frere->color = black;
    return fixredblack_remove_case1_left(p);
}

BinarySearchTree* fixredblack_remove_case2_right(BinarySearchTree* p){
    BinarySearchTree* frere = p->left;

    //frère rouge
    rightrotate(p);
    p->color = red;
    frere->color = black;
    return fixredblack_remove_case1_right(p);
}

BinarySearchTree* fixredblack_remove_case2(BinarySearchTree* p, BinarySearchTree* x) {
    if (x == p->left) {
        return fixredblack_remove_case2_left(p);
    } else {
        return fixredblack_remove_case2_right(p);
    }
}


