#include <stdlib.h>
#include <assert.h>

#include "skiplist.h"
#include "rng.h"

typedef struct s_Link {
    struct s_Noeud* prec;
    struct s_Noeud* suiv;
} Link;

typedef struct s_Noeud {
    int nbLevel;
    Link* lienTab;
    int value;
} Noeud;

struct s_SkipList {
    Noeud* sentinelle;
    int nbLevel;
    int taille;
    RNG prob;
};

SkipList* skiplist_create(int nblevels) {
	assert(nblevels > 0);

    SkipList* skipList = malloc(sizeof(SkipList) + sizeof(Noeud) + nblevels * sizeof(Link));

    Noeud* sentinelle = (Noeud*)(skipList + 1);
    skipList->sentinelle = sentinelle;
    skipList->taille = 0;
    skipList->nbLevel = nblevels;
    skipList->prob = rng_initialize(0, nblevels);

	Link* lienTab = (Link*)(sentinelle + 1);
    sentinelle->nbLevel = nblevels;
    sentinelle->lienTab = lienTab;

    for (int i = 0; i < nblevels; i++) {
        lienTab[i].prec = sentinelle;
        lienTab[i].suiv = sentinelle;
    }

    return skipList;
}

	

void skiplist_delete(SkipList** d) {
	assert(d != NULL && *d != NULL);
	Noeud* noeudCourant = (*d)->sentinelle->lienTab[0].suiv;
	Noeud* procNoeud = noeudCourant->lienTab[0].suiv;
	for (int i = 0; i < (*d)->taille; i++){
		free(noeudCourant);
		noeudCourant = procNoeud;
		procNoeud = noeudCourant->lienTab[0].suiv;
	}
	free(*d);
}

unsigned int skiplist_size(const SkipList* d) {
    return d->taille;
}

int skiplist_at(const SkipList* d, unsigned int i) {
    assert(d != NULL && i < skiplist_size(d));

    unsigned int indiceCourant = 0;
    Noeud* noeudCourant = d->sentinelle->lienTab[0].suiv;

	// on va jusqu'à l'indice i
    while (noeudCourant != d->sentinelle) {
        if (indiceCourant == i) {
            return noeudCourant->value;
        }
        noeudCourant = noeudCourant->lienTab[0].suiv;
        indiceCourant++;
    }
    return -1;   // si l'elt qu'on cherche n'est pas dans la liste
}

void skiplist_map(const SkipList* d, ScanOperator f, void* user_data) {
    assert(d != NULL && d->sentinelle != NULL);

    Noeud* noeudCourant = d->sentinelle->lienTab[0].suiv;
    while (noeudCourant != d->sentinelle) {
        f(noeudCourant->value, user_data);
        noeudCourant = noeudCourant->lienTab[0].suiv;
    }
}

SkipList* skiplist_insert(SkipList* d, int value) {

    Noeud** noeudInsert = malloc(d->nbLevel * sizeof(Noeud*));
    Noeud* noeudCourant = d->sentinelle;
    int niveau = (d->nbLevel) - 1;

	// recherche de l'emplacement où l'on doit insérer le nouveau noeud
    while (niveau >= 0) {
        Noeud* procNoeud = noeudCourant->lienTab[niveau].suiv;
        if (procNoeud == d->sentinelle || procNoeud->value > value) {
            noeudInsert[niveau] = noeudCourant;
            niveau--;
        } else if (procNoeud->value < value) {
            noeudCourant = procNoeud;
        } else {
            free(noeudInsert);
            return d;
        }
    }

	// on recupére la valeur de la proba pour le nbNiveau
    int nbNiveau = rng_get_value(&(d->prob)) + 1;
    if (nbNiveau > d->nbLevel){    // dans le cas où il y a une erreur de proba
		nbNiveau = d->nbLevel;
	}

	// création et initialisation du noeud a insérer
    Noeud* newNoeud = malloc(sizeof(Noeud) + nbNiveau * sizeof(Link)); // on évite la dispersion mémoire
    newNoeud->lienTab = (Link*)(newNoeud + 1);
    newNoeud->nbLevel = nbNiveau;
    newNoeud->value = value;
    
	// insertion du noeaud et mise a jour des ptr
    for (int niveau = 0; niveau < nbNiveau; niveau++) {
        Noeud* procNoeud = noeudInsert[niveau]->lienTab[niveau].suiv;
        noeudInsert[niveau]->lienTab[niveau].suiv = newNoeud;
        newNoeud->lienTab[niveau].prec = noeudInsert[niveau];
        newNoeud->lienTab[niveau].suiv = procNoeud;
        
        procNoeud->lienTab[niveau].prec = newNoeud;
    }
    (d->taille)++;

    free(noeudInsert);
    return d;
}

bool skiplist_search(const SkipList* d, int value, unsigned int* nb_operations){
    *nb_operations = 0;

    Noeud* noeudCourant = d->sentinelle;
    int niveau = (d->nbLevel)-1;

    while(niveau >= 0){

        // tant que les clés des nœuds accessibles sont inférieures à la clé recherchée
        while( (noeudCourant->lienTab[niveau].suiv != d->sentinelle) &&
                (noeudCourant->lienTab[niveau].suiv->value < value))
        {
            noeudCourant = noeudCourant->lienTab[niveau].suiv;
            (*nb_operations)++;
        }

        // si la clé courant est égale à cele qu'on cherche alors on renvoie true
        if((noeudCourant->lienTab[niveau].suiv != d->sentinelle) && 
            (noeudCourant->lienTab[niveau].suiv->value == value)){
            (*nb_operations)++;
            return true;

        // si supérieur alors on décremente le nombre de niveau
        } else{
            niveau--;
        }
        
    }
    (*nb_operations)++;
    return false;
}

SkipList* skiplist_remove(SkipList* d, int value) {
    assert(d != NULL);

    Noeud** noeudPrecTab = malloc(d->nbLevel * sizeof(Noeud*));
    Noeud* noeudCourant = d->sentinelle;
    Noeud* noeudASupprimer;
    int niveau = d->nbLevel - 1;
    bool noeudTrouve = false;
    
    // recherche du noeud a supprimer
    while (niveau >= 0) {
        // mise à jour du tableau de prédecesseur 
        while (noeudCourant->lienTab[niveau].suiv != d->sentinelle &&
               noeudCourant->lienTab[niveau].suiv->value < value) 
        {
            noeudCourant = noeudCourant->lienTab[niveau].suiv;
        }
        noeudPrecTab[niveau] = noeudCourant;

        // a chaque tour on vérifie si on est sur la valeur qu'on veut supprimer
        if (noeudCourant->lienTab[niveau].suiv != d->sentinelle &&
            noeudCourant->lienTab[niveau].suiv->value == value)
        {
            noeudASupprimer = noeudCourant->lienTab[niveau].suiv;
            noeudTrouve = true;
        }
        niveau--;
    }

    // si on trouve pas le noeud
    if (noeudTrouve == false) {
        free(noeudPrecTab);
        return d;
    }

    // mise à jour des ptr
    for (int i = 0; i < noeudASupprimer->nbLevel; i++) {
        noeudPrecTab[i]->lienTab[i].suiv = noeudASupprimer->lienTab[i].suiv;
        noeudASupprimer->lienTab[i].suiv->lienTab[i].prec = noeudPrecTab[i];
    }
    
    free(noeudASupprimer);
    free(noeudPrecTab);
    (d->taille)--;
    return d;
}

// --> même fonction mais renvoyant un bool car sur le sujet marquer bool 
//     mais dans le .h c'était un SkipList* attendue donc flexibilité 

/*bool skiplist_remove(SkipList* d, int value){
    assert(d != NULL);

    Noeud* noeudCourant = d->sentinelle;
    Noeud* noeudPrec[d->nbLevel];
    Noeud* noeudASuppr;
    int niveau = (d->nbLevel)-1;
    
    while(niveau >= 0){
        Noeud* procNoeud = noeudCourant->lienTab[niveau].suiv;
        if(procNoeud == d->sentinelle || procNoeud->value > value){
            noeudPrec[niveau] = noeudCourant;
            niveau--;
        } else if (procNoeud->value < value){
            noeudCourant = procNoeud;
        } else{
            noeudASuppr = procNoeud;
            noeudPrec[niveau] = noeudCourant;
            niveau--;
        }
    }

    for (int niveau = 0; niveau < noeudASuppr->nbLevel; niveau++) {
        Noeud* procNoeud = noeudASuppr->lienTab[niveau].suiv;
        noeudPrec[niveau]->lienTab[niveau].suiv = procNoeud;
        procNoeud->lienTab[niveau].prec = noeudPrec[niveau];
    }

    free(noeudASuppr);
    (d->taille)--;

    return true;
}
*/


/*-----------------------*/
/* Iterator             */
/*-----------------------*/

struct s_SkipListIterator{
    SkipList* skipList;
    Noeud* noeudCourant;
    IteratorDirection direction;
};

SkipListIterator* skiplist_iterator_create(SkipList* d, IteratorDirection w){
    assert(d != NULL);

    SkipListIterator* skipIterator = malloc(sizeof(SkipListIterator));
    skipIterator->skipList = d;
    skipIterator->noeudCourant = d->sentinelle;
    skipIterator->direction = w;

    return skipIterator;
}

void skiplist_iterator_delete(SkipListIterator** it){
    assert(it != NULL && *it != NULL);
    free(*it);
}

SkipListIterator* skiplist_iterator_begin(SkipListIterator* it){
    if(it->direction == FORWARD_ITERATOR){
        it->noeudCourant = it->skipList->sentinelle->lienTab[0].suiv;
    } else{
        it->noeudCourant = it->skipList->sentinelle->lienTab[0].prec;
    }

    return it;
}

bool skiplist_iterator_end(SkipListIterator* it){
    if(it->noeudCourant == it->skipList->sentinelle){
        return true;
    }
    return false;
}

SkipListIterator* skiplist_iterator_next(SkipListIterator* it){
    if ( (skiplist_iterator_end(it) == false) && (it->direction == FORWARD_ITERATOR) ){
        it->noeudCourant = it->noeudCourant->lienTab[0].suiv;
    }

    if ( (skiplist_iterator_end(it) == false) && (it->direction == BACKWARD_ITERATOR) ){
        it->noeudCourant = it->noeudCourant->lienTab[0].prec;
    }

    return it;
}

int skiplist_iterator_value(SkipListIterator* it){
    return it->noeudCourant->value;
}




polynome = queue ->[(coef0, puiss0), (coef1, puiss1), (coef2, puiss2),......, (coefn, npuiss)] <-- tete
polynome[0][0]
polynome = X^n + X^n-1 ... + 1 

def function(liste, flottant):
    result_finale = 0
    for i in range(len(liste)):
        coef = list[i][0]
        puissance = list[i][1] 
        result_finale = result_finale + coef*(flottant**puissance])
    return result_finale

def 