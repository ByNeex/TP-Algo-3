#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


/*------------------------  BSTreeType  -----------------------------*/

struct _bstree {
    BinarySearchTree* parent;
    BinarySearchTree* left;
    BinarySearchTree* right;
    int key;
};

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
    BinarySearchTree* new_node = bstree_cons(NULL, NULL, v);
    new_node->parent = parent;

    if (v < parent->key) {
        parent->left = new_node;
    } else {
        parent->right = new_node;
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

typedef BinarySearchTree* (*AccessFunction)(const BinarySearchTree*);

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

    

    BinarySearchTree* tempRight = from->right;
    BinarySearchTree* tempLeft = from->left;
    from->left = to->left;
    to->left = tempLeft;
    from->right = to->right;
    to->right = tempRight;
    
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


// t -> the tree to remove from, current -> the node to remove
void bstree_remove_node(ptrBinarySearchTree *t, ptrBinarySearchTree current) {
    assert(!bstree_empty(*t) && !bstree_empty(current));

    ptrBinarySearchTree change = NULL;
    if (current->left == NULL && current->right == NULL){
        change = NULL;
    } else if (current->right == NULL){
        change = current->left;
    }
    else if (current->left == NULL){
        change = current->right;
    }
    else {
        ptrBinarySearchTree successeur = (BinarySearchTree*)bstree_successor(current); // cast pour éviter le problème avec le const
        bstree_swap_nodes(t, current, successeur);
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

