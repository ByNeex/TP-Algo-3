// Need this to use the getline C function on Linux. Works without this on MacOs. Not tested on Windows.
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>

#include "token.h"
#include "queue.h"
#include "stack.h"


/** 
 * Utilities function to print the token queues
 */
void print_token(const void* e, void* user_param);
void print_queue(FILE* f, Queue* q);



/** 
 * Function to be written by students
 */

/* ------------------------------ Exercice 1 ------------------------------ */
bool isSymbol(char c){
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '(' || c == ')';
}

Queue* stringToTokenQueue(const char* expression){
	Queue* tokenQueue = create_queue();
	const char* curpos = expression;
	while(*curpos != '\0'){
		while(*curpos == ' ' || *curpos == '\n'){
			curpos++;
		}

		if (isSymbol(*curpos)){
			Token *tokenSymbol = create_token_from_string(curpos, 1);
			queue_push(tokenQueue, tokenSymbol);
			curpos++;
		} else if (isdigit(*curpos)){
			int lgDigit = 1;
			while(isdigit(*(curpos + lgDigit))){
				lgDigit++;
			}
			Token *tokenSymbol = create_token_from_string(curpos, lgDigit);
			queue_push(tokenQueue, tokenSymbol);
			curpos = curpos + lgDigit;
		} else{
			curpos++;
		}
	}
	return tokenQueue;
}

/* ------------------------------ Exercice 2 ------------------------------ */

Queue* shuntingYard(Queue* infix){
	Queue* postfix = create_queue();
	Stack* operateurStack = create_stack(0);

	while(!queue_empty(infix)){
		Token* token = (Token*) queue_top(infix);
		queue_pop(infix);

		if(token_is_number(token)){
			queue_push(postfix, token);
		}else if (token_is_operator(token)){
			while(!stack_empty(operateurStack) && token_is_operator((Token*)stack_top(operateurStack)) 
			&& ((token_operator_priority((Token*)stack_top(operateurStack)) > token_operator_priority(token)) 
			||(
				token_operator_priority((Token*)stack_top(operateurStack)) == token_operator_priority(token) 
				&&
				token_operator_leftAssociative(token)))
				){

				queue_push(postfix, stack_top(operateurStack));
				stack_pop(operateurStack);
			}
			stack_push(operateurStack, token);

		} else if (token_is_parenthesis(token) && token_parenthesis(token) == '('){
			stack_push(operateurStack, token);

		} else if (token_is_parenthesis(token) && token_parenthesis(token) == ')'){

			while(!stack_empty(operateurStack) && 
			(!token_is_parenthesis((Token*)stack_top(operateurStack)) || token_parenthesis((Token*)stack_top(operateurStack)) != '(')){
				queue_push(postfix, stack_top(operateurStack));
				stack_pop(operateurStack);
			}
			Token* t = (Token*)stack_top(operateurStack);
			delete_token(&t);
			stack_pop(operateurStack);
			delete_token(&token);
		}
	}
	while (!stack_empty(operateurStack)){
		queue_push(postfix, stack_top(operateurStack));
		stack_pop(operateurStack);
	}
	delete_stack(&operateurStack);
	return postfix;
}

/* ------------------------------ Exercice 3 ------------------------------ */

Token* evaluateOperator(Token* arg1, Token* op, Token* arg2){
	float resultat = 0;

	if(token_operator(op) == '+'){
		resultat = token_value(arg1) + token_value(arg2);
	} else if (token_operator(op) == '-'){
		resultat = token_value(arg1) - token_value(arg2);
	} else if (token_operator(op) == '*'){
		resultat = token_value(arg1) * token_value(arg2);
	} else if (token_operator(op) == '/'){
		if(token_value(arg2) == 0){
			fprintf(stderr, "Division par 0 impossible !\n");
		}
		resultat = token_value(arg1) / token_value(arg2);
	} else if (token_operator(op) == '^'){
		resultat = pow(token_value(arg1), token_value(arg2));
	} else{
		fprintf(stderr, "Opérateur inconnue !\n");
	}

	return create_token_from_value(resultat);
}

float evaluateExpression(Queue* postfix){
	Stack* stack = create_stack(0);
	float resultatFinale = 0;
	while(!queue_empty(postfix)){
		Token* token = (Token*)queue_top(postfix);
		queue_pop(postfix);
		if (token_is_operator(token)){
			Token* op2 = (Token*)stack_top(stack); 
			stack_pop(stack);
            Token* op1 = (Token*)stack_top(stack); 
			stack_pop(stack);

			Token* resultat = evaluateOperator(op1, token, op2);
			stack_push(stack, resultat);

			delete_token(&op1);
			delete_token(&op2);
			delete_token(&token);
		} else if(token_is_number(token)){
			stack_push(stack, token);
		}
	}
	Token* derResultatToken = (Token*)stack_top(stack);
	resultatFinale = token_value(derResultatToken);

	delete_token(&derResultatToken);
	delete_stack(&stack);

	return resultatFinale;
}

/* ------------------------------ Fonction finale ------------------------------ */

void computeExpressions(FILE* input) {
	char* ligne = NULL;
	size_t tailleInit = 0;
	ssize_t tailleLue = 0; // ssize pour avoir des valeurs négatives
	Queue* tokenQueue;
	Queue* postfix;
	float resultat;
	while((tailleLue = getline(&ligne, &tailleInit, input)) != -1){
		printf("Input    : %s", ligne);
		tokenQueue = stringToTokenQueue(ligne);

		printf("Infix    : ");
		print_queue(stdout, tokenQueue);
		printf("\n");

		postfix = shuntingYard(tokenQueue);
		printf("Postfix  : ");	
		print_queue(stdout, postfix);
		printf("\n");

		resultat = evaluateExpression(postfix);
		printf("Evaluate : ");
		printf("%f\n", resultat);
		printf("\n");
		
		while(!queue_empty(postfix)){
			Token* token = (Token*) queue_top(postfix);
			delete_token(&token);
			queue_pop(postfix);
		}
		delete_queue(&postfix);

		while(!queue_empty(tokenQueue)){
			Token* token = (Token*) queue_top(tokenQueue);
			delete_token(&token);
			queue_pop(tokenQueue);
		}
		delete_queue(&tokenQueue);
	}
	free(ligne);
}


/** Main function for testing.
 * The main function expects one parameter that is the file where expressions to translate are
 * to be read.
 *
 * This file must contain a valid expression on each line
 *
 */
int main(int argc, char** argv){
	if (argc<2) {
		fprintf(stderr,"usage : %s filename\n", argv[0]);
		return 1;
	}
	
	FILE* input = fopen(argv[1], "r");

	if ( !input ) {
		perror(argv[1]);
		return 1;
	}

	computeExpressions(input);

	fclose(input);
	return 0;
}
 
void print_token(const void* e, void* user_param) {
	FILE* f = (FILE*)user_param;
	Token* t = (Token*)e;
	token_dump(f, t);


}

void print_queue(FILE* f, Queue* q) {
	fprintf(f, "(%d) --  ", queue_size(q));
	queue_map(q, print_token, f);
}
