#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "skiplist.h"
#include "rng.h"

/**
 *	@defgroup SkipListTest Test program for SkipList Implantation
 *  @brief Definition of the SkipList type and operators
 *  @{
 */

/** Print a usage message for this program.
 
 @code{.unparsed}
 $skiplisttest -id num
 where id is :
 	c : construct and print the skiplist with data read from file ../Test/test_files/construct_num.txt
 	s : construct the skiplist with data read from file ../Test/test_files/construct_num.txt and search elements from file test_files/search_num..txt
 		Print statistics about the searches.
 	i : construct the skiplist with data read from file ../Test/test_files/construct_num.txt and search, using an iterator, elements read from file test_files/search_num.txt
 		Print statistics about the searches.
 	r : construct the skiplist with data read from file test_files/construct_num.txt, remove values read from file test_files/remove_num.txt and print the list in reverse order
 
 and num is the file number for input.
 @endcode
 */
void usage(const char *command) {
	printf("usage : %s -id num\n", command);
	printf("where id is :\n");
	printf("\tc : construct and print the skiplist with data read from file test_files/construct_num.txt\n");
	printf("\ts : construct the skiplist with data read from file test_files/construct_num.txt and search elements from file test_files/search_num..txt\n\t\tPrint statistics about the searches.\n");
	printf("\ti : construct the skiplist with data read from file test_files/construct_num.txt and search, using an iterator, elements read from file test_files/search_num.txt\n\t\tPrint statistics about the searches.\n");
	printf("\tr : construct the skiplist with data read from file test_files/construct_num.txt, remove values read from file test_files/remove_num.txt and print the list in reverse order\n");
	printf("and num is the file number for input\n");
}

/** Return the filename associated with the action to perform and the number of a test.
 
 @param action Describe the action associated with the file to load. Must be one of "construct" "search" or "remove"
 @param filenumber Number of the file to load. Corresponds to the number of the test to perform.
 @return fileame  in a relative form
 
 @code{.c}
 char *filename = gettestfilename("construct", 2);
 @endcode
 will set filename to @code{.c}"../Test/test_files/construct_2.txt"@endcode
 
 */
char *gettestfilename(const char *action, int numtest) {
	const char *testdir ="../Test/test_files/";
	int l = strlen(testdir) + strlen(action) + strlen("_xxx.txt");
	char *name = malloc(l*sizeof(char));
	sprintf(name, "%s%s_%d.txt", testdir, action, numtest);
	return name;
}

int read_int(FILE* input) {
  int v;
  int r = fscanf(input,"%d", &v);
  if (r == 1) {
    return v;
  }
  perror("Unable to read int from input file\n");
  abort();
}

unsigned int read_uint(FILE* input) {
  unsigned int v;
  int r = fscanf(input,"%u", &v);
  if (r == 1) {
    return v;
  }
  perror("Unable to read uint from input file\n");
  abort();
}

/** Build a list corresponding to the fiven file number.
 */
SkipList* buildlist(int num) {
	SkipList* d;
	FILE *input;
	
	char *constructfromfile = gettestfilename("construct", num);
	input = fopen(constructfromfile, "r");
	if (input!=NULL) {
		d = skiplist_create(read_uint(input));
		unsigned int nb_values = read_uint(input);
		for (unsigned int i=0;i< nb_values; ++i) {
			d = skiplist_insert(d, read_int(input));
		}
	} else {
		printf("Unable to open file %s\n", constructfromfile);
		free(constructfromfile);
		exit (1);
	}
	free(constructfromfile);
	fclose(input);
	return d;
}

/*----------------------------------------------------------------------------------------------*/

/** Exercice 1.
 	Programming and test of skiplist construction.
 */
void test_construction(int num) {
    SkipList* d = buildlist(num);
    printf("Skiplist (%d)\n", skiplist_size(d));

    for (unsigned int i = 0; i < skiplist_size(d); ++i) {
        printf("%d ", skiplist_at(d, i));
    }
    printf("\n");

    skiplist_delete(&d);
}

/** Exercice 2.
 Programming and test of skiplist search operator.
 */
void test_search(int num){

	// Ouverture du fichier 
	SkipList* d = buildlist(num);
	char *testFile = gettestfilename("search", num);
	FILE *input = fopen(testFile, "r");
    if (input == NULL) {
        printf("Erreur d'ouverture du fichier %s\n", testFile);
        free(testFile);
        exit(1);
    }

	// initialisation des variables utiles pour l'affichage
    unsigned int nbValeurSearch= read_uint(input);   //premiere valeur du fichier : uint indiquant le nombre de valeur a chercher
    unsigned int valTrouve = 0, valPasTrouve = 0, operationTotal = 0, operationMax = 0;
    unsigned int operationMin = skiplist_size(d);
	unsigned int operationMoy;

	// on cherche la valeur qu'on veut en faisant appelle à skipliste_search
    for (unsigned int i = 0; i < nbValeurSearch; i++){
		int valASearch = read_int(input);    // toute les autres : entier signé devant être cherché dans la liste
        unsigned int operations = 0;
        bool valSkipList = skiplist_search(d, valASearch, &operations);
		// permet l'affichage de valeur ->true/false
		if(valSkipList == true){
			printf("%d -> true\n", valASearch);
			valTrouve++;
		} else{
			printf("%d -> false\n", valASearch);
			valPasTrouve++;
		}

		// pour modifier les variables qui vont servir a afficher ce qu'on veut
        operationTotal = operationTotal + operations;

        if (operations < operationMin) {
            operationMin = operations;
        }
        if (operations > operationMax) {
            operationMax = operations;
        }
    }
    fclose(input);
    free(testFile);

	operationMoy = operationTotal / nbValeurSearch;
    printf("Statistics :\n");
    printf("\tSize of the list : %d\n", skiplist_size(d));
    printf("Search %d values :\n", nbValeurSearch);
    printf("\tFound %d\n", valTrouve);
    printf("\tNot found %d\n", valPasTrouve);
    printf("\tMin number of operations : %d\n", operationMin);
    printf("\tMax number of operations : %d\n", operationMax);
    printf("\tMean number of operations : %d\n", operationMoy);

    skiplist_delete(&d);
}

/** Exercice 3.
 Programming and test of naïve search operator using iterators.
 */
void test_search_iterator(int num) {

	// Ouverture du fichier 
    SkipList* d = buildlist(num);
    char *testFile = gettestfilename("search", num);
    FILE *input = fopen(testFile, "r");
    if (input == NULL) {
        printf("Erreur d'ouverture du fichier %s\n", testFile);
        free(testFile);
        exit(1);
    }

    // initialisation des variables utiles pour l'affichage
    unsigned int nbValeurSearch = read_uint(input); //premiere valeur du fichier : uint indiquant le nombre de valeur a chercher
    unsigned int valTrouve = 0, valPasTrouve = 0, operationTotal = 0, operationMax = 0;
    unsigned int operationMin = skiplist_size(d);
    unsigned int operationMoy;

    // on cherche la valeur qu'on veut à l'aide de l'itérateur
    for (unsigned int i = 0; i < nbValeurSearch; i++) {
        int valASearch = read_int(input); // toute les autres : entier signé devant être cherché dans la liste
        unsigned int operations = 0; 
        bool valIterator = false;

        SkipListIterator* it = skiplist_iterator_create(d, FORWARD_ITERATOR);
        skiplist_iterator_begin(it);

		// utilisation de l'iterateur pour parcourir la SkipList
        while (skiplist_iterator_end(it) == false && valIterator == false) {
            operations++;
            if (skiplist_iterator_value(it) == valASearch) {
                valIterator = true;
            }
            skiplist_iterator_next(it);
        }
        skiplist_iterator_delete(&it);

        // permet l'affichage de valeur ->true/false
        if (valIterator == true) {
            printf("%d -> true\n", valASearch);
            valTrouve++;
        } else {
            printf("%d -> false\n", valASearch);
            valPasTrouve++;
        }

        // pour modifier les variables qui vont servir a afficher ce qu'on veut
        operationTotal = operationTotal + operations;
        if (operations < operationMin) {
            operationMin = operations;
        }
        if (operations > operationMax) {
            operationMax = operations;
        }
    }
    fclose(input);
    free(testFile);

    operationMoy = operationTotal / nbValeurSearch;
    printf("Statistics :\n");
    printf("\tSize of the list : %d\n", skiplist_size(d));
    printf("Search %d values :\n", nbValeurSearch);
    printf("\tFound %d\n", valTrouve);
    printf("\tNot found %d\n", valPasTrouve);
    printf("\tMin number of operations : %d\n", operationMin);
    printf("\tMax number of operations : %d\n", operationMax);
    printf("\tMean number of operations : %d\n", operationMoy);

    skiplist_delete(&d);
}

/** Exercice 4.
 Programming and test of skiplist remove operator.
 */
void test_remove(int num){

	// Ouverture du fichier
    SkipList* d = buildlist(num);
    char *testFile = gettestfilename("remove", num);
    FILE *input = fopen(testFile, "r");
    if (input == NULL) {
        printf("Erreur d'ouverture du fichier %s\n", testFile);
        free(testFile);
        exit(1);
    }

    unsigned int nbValeurRemove = read_uint(input); //premiere valeur du fichier : uint indiquant le nombre de valeur a chercher
    for (unsigned int i = 0; i < nbValeurRemove; i++) {
        int valASupprimer = read_int(input); // toute les autres : entier signé devant être cherché dans la liste
		skiplist_remove(d, valASupprimer);
    }
    fclose(input);
    free(testFile);

	// on affiche la liste dans le sens inverse à l'aide de l'itérateur
    printf("Skiplist (%d)\n", skiplist_size(d));
    SkipListIterator* it = skiplist_iterator_create(d, BACKWARD_ITERATOR);
    skiplist_iterator_begin(it);
    while (skiplist_iterator_end(it) == false) {
        printf("%d ", skiplist_iterator_value(it));
        skiplist_iterator_next(it);
    }
    printf("\n");

    skiplist_iterator_delete(&it);
    skiplist_delete(&d);
}


/** Function you ca use to generate dataset for testing.
 */
void generate(int nbvalues);


int main(int argc, const char *argv[]){
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}
	switch (argv[1][1]) {
		case 'c' :
			test_construction(atoi(argv[2]));
			break;
		case 's' :
			test_search(atoi(argv[2]));
			break;
		case 'i' :
			test_search_iterator(atoi(argv[2]));
			break;
		case 'r' :
			test_remove(atoi(argv[2]));
			break;
		case 'g' :
			generate(atoi(argv[2]));
			break;
		default :
			usage(argv[0]);
			return 1;
	}
	return 0;
}


/* Generates a set of test files for a given number of value. */

void generate(int nbvalues){
	FILE *output;
	int depth;
	int maxvalue;
	output = fopen("construct.txt", "w");
	srand(nbvalues);
	depth = rand()%16;
	maxvalue = rand()%10 * nbvalues;
	fprintf(output, "%d\n%d\n", depth, nbvalues);
	for (int i=0; i< nbvalues; ++i) {
		fprintf(output, "%d\n", rand()%maxvalue);
	}
	fclose(output);
	output = fopen("search.txt", "w");
	srand(rand());
	nbvalues *= depth/4;
	fprintf(output, "%d\n", nbvalues);
	for (int i=0; i< nbvalues; ++i) {
		fprintf(output, "%d\n", rand()%maxvalue);
	}
	fclose(output);
}


/** @} */

/**
 @mainpage
 @section TP45 Implantation du TAD Listes à raccourci.
 @subsection SkipLists SkipLists
 Les listes à raccourcis, nommées SkipLists, sont une alternative aux arbres de
 recherche équilibrés que nous verrons dans la suite du cours.
 
 Inventées par William Pugh en 1902, elles reposent sur une structure de données
 linéaire construite de manière probabiliste.
 Les opérations de dictionnaire définies sur les listes à raccourcis sont simples à
 programmer, élégantes, et l’aspect probabiliste permet de démontrer une complexité
 moyenne en O(log(N)) au lieu des O(N) inhérents aux structures linéaires.
 
 Toutefois, l’aspect probabiliste de la structure de données ne permet pas d’assurer
 la complexité en pire cas, comme on pourra le faire sur les arbres équilibrés, mais
 donne, en pratique, une structure extrêmement efficace (la probabilité de construire
 une structure non efficace étant très faible). Nous ne ferons pas de démonstration
 ici, et il ne vous est pas demandé d’en effectuer une, les étudiants curieux
 pourront se rapporter à leur cours de complexité et à l’analyse fournie dans
 l’article de William Pugh, accessible sur Moodle et sur la <a href="https://dl.acm.org/citation.cfm?id=78977"> bibliothèque ACM</a>.
 
 @note \anchor WP-skiplist-1990 William Pugh, <a href="https://dl.acm.org/citation.cfm?id=78977">Skip lists : a probabilistic alternative to balanced trees</a> in Communications of the ACM, June 1990, 33(6) 668-676
 
 @subsection Objectifs Objectifs du TP
 - Implantation du TAD SkipList
 - Implantation d'un itérateur sur une SkipList
 - Comparaison empirique des opérations de dictionnaires sur les SkipList et en utilisant un itérateur.
 
 */
