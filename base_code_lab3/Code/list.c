/*-----------------------------------------------------------------*/
/*
 Licence Informatique - Structures de données
 Mathias Paulin (Mathias.Paulin@irit.fr)
 
 Implantation du TAD List vu en cours.
 */
/*-----------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "list.h"

typedef struct s_LinkedElement {
	int value;
	struct s_LinkedElement* previous;
	struct s_LinkedElement* next;
} LinkedElement;

/* Use of a sentinel for implementing the list :
 The sentinel is a LinkedElement* whose next pointer refer always to the head of the list and previous pointer to the tail of the list
 */
struct s_List {
	LinkedElement* sentinel;
	int size;
};


/*-----------------------------------------------------------------*/

List* list_create(void) {
	List* l = malloc(sizeof(struct s_List));
	l->sentinel = malloc(sizeof(LinkedElement));
	l->sentinel->next = l->sentinel;
	l->sentinel->previous = l->sentinel;
	l->size = 0;
	return l;
}

/*-----------------------------------------------------------------*/

List* list_push_back(List* l, int v) {

	// ajout du nouvel elt à la fin et initialisation du next, prev et value
	LinkedElement* newElt = malloc(sizeof(LinkedElement));
	newElt->next=l->sentinel;
	newElt->value = v;
	newElt->previous = l->sentinel->previous;
	
	// mise à jour de la sentinelle et l'ancienne queue de liste + incrémentation taille liste
	l->sentinel->previous->next= newElt;
	l->sentinel->previous = newElt;
	(l->size)++;
	return l;
}

/*-----------------------------------------------------------------*/

void list_delete(ptrList* l) {
	LinkedElement* save;
	LinkedElement* elemCourant = (*l)->sentinel->next;
	while(elemCourant != (*l)->sentinel){
		save = elemCourant->next;
		free(elemCourant);
		elemCourant = save;
	}
	free((*l)->sentinel);
	free(*l);
	*l = NULL;
}

/*-----------------------------------------------------------------*/

List* list_push_front(List* l, int v) {

	// ajout du nouvel elt au début et initialisation du next, prev et value
	LinkedElement* newElt = malloc(sizeof(LinkedElement));
	newElt->previous = l->sentinel;
	newElt->value = v;
	newElt->next = l->sentinel->next;

	// mise à jour de la sentinelle et l'ancienne tête de liste + incrémentation taille liste
	l->sentinel->next->previous = newElt;
	l->sentinel->next = newElt;
	(l->size)++;

	return l;
}

/*-----------------------------------------------------------------*/

int list_front(const List* l) {
	assert(!list_is_empty(l));
	return l->sentinel->next->value;
}

/*-----------------------------------------------------------------*/

int list_back(const List* l) {
	assert(!list_is_empty(l));
	return l->sentinel->previous->value;
}

/*-----------------------------------------------------------------*/

List* list_pop_front(List* l) {
	assert(!list_is_empty(l)); // pré-condition
	LinkedElement* save = l->sentinel->next;
    l->sentinel->next = save->next; // changement de la tête 
	save->next->previous = l->sentinel; // changemet du prec pour la nouvelle tête
	free(save); 
	(l->size)--;

	return l;
}

/*-----------------------------------------------------------------*/

List* list_pop_back(List* l){
	assert(!list_is_empty(l)); // pré-condition
	LinkedElement* save = l->sentinel->previous;
	l->sentinel->previous = save->previous;  // changement de la queue
	save->previous->next = l->sentinel;  // changement du suivant pour la nouvelle queue
	free(save);
	(l->size)--;

	return l;
}

/*-----------------------------------------------------------------*/

List* list_insert_at(List* l, int p, int v) {
	assert( (0 <= p) && (list_size(l) >= p) );

	// création de l'elt à insérer
	LinkedElement* newElt = malloc(sizeof(LinkedElement));
	newElt->value = v;

	// parcours de la liste jusqu'à l'indice p
	LinkedElement* elemCourant = l->sentinel->next;
	for (int i = 0; i < p; i++){
		elemCourant = elemCourant->next;
	}

	// initialisation du prec et next pour le nouvel elt
	newElt->next = elemCourant;
	newElt->previous = elemCourant->previous;

	// mise à jour de prec et next de elem courant et son prec
	newElt->next->previous = newElt;
	newElt->previous->next = newElt;

	(l->size)++;
	return l;
}

/*-----------------------------------------------------------------*/

List* list_remove_at(List* l, int p) {
	assert( (!list_is_empty(l)) && (0 <= p) && (list_size(l) > p) );

	LinkedElement* elemCourant = l->sentinel->next;
	for (int i = 0; i < p; i++){
		elemCourant = elemCourant->next;
	}

	elemCourant->next->previous = elemCourant->previous;
	elemCourant->previous->next = elemCourant->next;
	free(elemCourant);
	(l->size)--;

	return l;
}

/*-----------------------------------------------------------------*/

int list_at(const List* l, int p) {
	assert( (!list_is_empty(l)) && (!list_size(l) > p));
	LinkedElement* elemCourant = l->sentinel->next;
	for (int i = 0; i < p; i++){
		elemCourant = elemCourant->next;
	}
	return elemCourant->value;
}

/*-----------------------------------------------------------------*/

bool list_is_empty(const List* l) {
	if( (l->sentinel->next == l->sentinel) && (l->sentinel->previous == l->sentinel) ){
		return true;
	}
	return false;
}

/*-----------------------------------------------------------------*/

int list_size(const List* l) {
	return l->size;  // 0(1) respecté 
}

/*-----------------------------------------------------------------*/

List* list_map(List* l, ListFunctor f, void* environment) {
	LinkedElement* elemCourant = l->sentinel->next;

	while(elemCourant != l->sentinel){
		elemCourant->value = f(elemCourant->value, environment); // appication de f a chaque elem de l
		elemCourant = elemCourant->next; // passage à l'elt suivant
	}

	return l;
}

/*-----------------------------------------------------------------*/

typedef struct s_SubList{
	LinkedElement* head;
	LinkedElement* tail;
} SubList;

SubList list_split(SubList l){

	// on gère le cas où l est vide ou n'a que 1 elt
	if(l.head == NULL || l.head == l.tail){
		return l;
	}

	// parcourt de la liste avec deux pointeurs pour trouver le milieu et ainsi la diviser en deux sous-listes
	LinkedElement* ptr1 = l.head;
	LinkedElement* ptr2 = l.head;

	while(ptr2 != l.tail && ptr2->next != l.tail){
		ptr1 = ptr1->next;
		ptr2 = ptr2->next->next;
	}

	// initialisation de la struct a return
	SubList listDivide;
	listDivide.head = ptr1;
	listDivide.tail = ptr1->next;

	return listDivide;
}

SubList list_merge(SubList leftlist, SubList rightlist, OrderFunctor f) {
	SubList listFusion;
    LinkedElement* elemCourant;
    if (leftlist.head == NULL){
		return rightlist;   // si la liste gauche est vide on retourne la droite 
	}

    if (rightlist.head == NULL){
		return leftlist;   // par analogie on retourne la gauche
	} 

	// cas du premier elt 
    if (f(leftlist.head->value, rightlist.head->value)) {
        listFusion.head = leftlist.head;
        leftlist.head = leftlist.head->next;
    } else {
        listFusion.head = rightlist.head;
        rightlist.head = rightlist.head->next;
    }
    elemCourant = listFusion.head;

    // reste des elts qu'on fusionne
    while (leftlist.head != NULL && rightlist.head != NULL) {
        if (f(leftlist.head->value, rightlist.head->value)) {
            elemCourant->next = leftlist.head;
            leftlist.head = leftlist.head->next;
        } else {
            elemCourant->next = rightlist.head;
            rightlist.head = rightlist.head->next;
        }
        elemCourant = elemCourant->next;
    }

	// cas de la liste pas vide qu'on lie
    if (leftlist.head != NULL) {
        elemCourant->next = leftlist.head;
    } else {
        elemCourant->next = rightlist.head;
    }

	// affectation de la queue
    while (elemCourant->next != NULL) {
        elemCourant = elemCourant->next;
    }
    listFusion.tail = elemCourant;

    return listFusion;
}

SubList list_mergesort(SubList l, OrderFunctor f) {
	if (l.head == l.tail || (l.head == NULL && l.tail == NULL)){
		return l;
	}
	SubList listSplit = list_split(l);
	SubList leftList, rightList, listFusion;
	leftList.head = l.head;
	leftList.tail = listSplit.head;
	if(leftList.head != NULL){
		leftList.head->previous = NULL;
	}
	if(leftList.tail != NULL){
		leftList.tail->next = NULL;
	}
	leftList = list_mergesort(leftList, f);

	rightList.head = listSplit.tail;
	rightList.tail = l.tail;
	if(rightList.head != NULL){
		rightList.head->previous = NULL;
	}
	if(rightList.tail != NULL){
		rightList.tail->next = NULL;
	}
	rightList = list_mergesort(rightList, f);

	listFusion = list_merge(leftList, rightList, f);
	return listFusion;
}

List *list_sort(List *l, OrderFunctor f) {
	SubList subList;
	subList.head = l->sentinel->next;
	subList.tail = l->sentinel->previous;

	subList = list_mergesort(subList, f);

	l->sentinel->next = subList.head;
	l->sentinel->previous = subList.tail;
	subList.head->previous = l->sentinel;
	subList.tail->next = l->sentinel;

	return l;
}


