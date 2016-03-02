#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_INVALID_SYMBOL_TRANSITION "Invalid symbol in transition"
#define ERROR_INVALID_SYMBOL_WORD       "Invalid symbol in word"
#define ERROR_INVALID_STATE             "Invalid state"
#define ERROR_NOT_DFA                   "Invalid DFA"

#define SEPARATORS " ;\r\n"

struct dfa_node_t;

typedef struct dfa_transition_t {
	char symbol;
	struct dfa_node_t *node;
	struct dfa_transition_t *next;
} dfa_transition_t;

typedef enum {
	NODE_SIMPLE,
	NODE_START,
	NODE_END,
	NODE_START_END,
} dfa_node_type_t;

typedef struct dfa_node_t {
	char *label;
	dfa_node_type_t type;
	dfa_transition_t *transitions;
	struct dfa_node_t *next;
} dfa_node_t;

void print_error(char *error)
{
	printf("%s ;\n", error);
}

dfa_node_t *dfa_node_new(dfa_node_type_t type, char *label)
{
	dfa_node_t *node = (dfa_node_t *)malloc(sizeof(dfa_node_t));
	node->label = label;
	node->type = type;
	node->transitions = NULL;
	node->next = NULL;
	return node;
}

void dfa_transition_clear(dfa_transition_t *node);

void dfa_node_free(dfa_node_t *node)
{
	free(node->label);
	dfa_transition_clear(node->transitions);
	free(node);
}

void dfa_clear(dfa_node_t *node)
{
	dfa_node_t *aux;
	while (node) {
		aux = node;
		node = node->next;
		dfa_node_free(aux);
	}
}

dfa_node_t *dfa_find_node(char *label, dfa_node_t *dfa)
{
	while (dfa) {
		if (!strcmp(label, dfa->label))
			return dfa;
		dfa = dfa->next;
	}
	return NULL;
}

dfa_node_t *dfa_find_node_by_type(dfa_node_type_t type, dfa_node_t *dfa)
{
	while (dfa) {
		if (dfa->type == type)
			return dfa;
		dfa = dfa->next;
	}
	return NULL;
}

dfa_transition_t *dfa_transition_new(char symbol, dfa_node_t *node)
{
	dfa_transition_t *transition = (dfa_transition_t *)malloc(sizeof(dfa_transition_t));
	transition->symbol = symbol;
	transition->node = node;
	return transition;
}

void dfa_transition_free(dfa_transition_t *transition)
{
	free(transition);
}

void dfa_transition_clear(dfa_transition_t *node)
{
	dfa_transition_t *aux;
	while (node) {
		aux = node;
		node = node->next;
		dfa_transition_free(aux);
	}
}

void dfa_transition_add(char *from, char *to, char symbol, dfa_node_t *dfa, int *error)
{
	dfa_node_t *from_node = dfa_find_node(from, dfa);
	dfa_node_t *to_node = dfa_find_node(to, dfa);

	if (!from_node || !to_node) {
		print_error(ERROR_INVALID_STATE);
		*error = 1;
		return;
	}

	dfa_transition_t *transition = dfa_transition_new(symbol, to_node);
	transition->next = from_node->transitions;
	from_node->transitions = transition;
}

void validate_dfa(dfa_node_t *dfa, char *alphabet, int *error)
{
	if (!dfa)
		goto error;

	int initial_state_found = 0;

	char *al = alphabet;
	while (*al) {
		dfa_node_t *node = dfa;
		while (node) {
			if (node->type == NODE_START || node->type == NODE_START_END)
				initial_state_found = 1;
			int tr_count = 0;
			dfa_transition_t *transition = node->transitions;

			while (transition) {
				if (transition->symbol == *al)
					++tr_count;
				transition = transition->next;
			}

			if (tr_count != 1)
				goto error;

			node = node->next;
		}
		if (!initial_state_found)
			goto error;
		++al;
	}

	return;
error:
	print_error(ERROR_NOT_DFA);
	*error = 1;
}

void parse_states(char *line, dfa_node_t **dfa)
{
	char *token = strtok(line, SEPARATORS);
	while (token) {
		dfa_node_t *node = dfa_node_new(NODE_SIMPLE, strdup(token));
		node->next = *dfa;
		*dfa = node;
		token = strtok(NULL, SEPARATORS);
	}
}

void remove_all_chars(char* str, char c)
{
	char *pr = str, *pw = str;
	while (*pr) {
		*pw = *pr++;
		pw += (*pw != c);
	}
	*pw = '\0';
}

void clear_string(char *str)
{
	remove_all_chars(str, ' ');
	remove_all_chars(str, ';');
	remove_all_chars(str, '\r');
	remove_all_chars(str, '\n');
}

char *parse_alphabet(char *line)
{
	clear_string(line);
	return line;
}

void parse_transition(char *line, dfa_node_t *dfa, char *alphabet, int *error)
{
	char *from = strtok(line, SEPARATORS);
	char *to = strtok(NULL, SEPARATORS);
	char *symbol = strtok(NULL, SEPARATORS);

	if (!from || !to || !symbol) {
		print_error(ERROR_INVALID_STATE);
		*error = 1;
		return;
	}

	if (!strchr(alphabet, symbol[0])) {
		print_error(ERROR_INVALID_SYMBOL_TRANSITION);
		*error = 1;
		return;
	}

	dfa_transition_add(from, to, symbol[0], dfa, error);
}

void parse_initial_state(char *line, dfa_node_t *dfa, int *error)
{
	char *token = strtok(line, SEPARATORS);
	dfa_node_t *node = dfa_find_node(token, dfa);
	if (!node) {
		print_error(ERROR_INVALID_STATE);
		*error = 1;
		return;
	}
	node->type = NODE_START;
}

void parse_finish_states(char *line, dfa_node_t *dfa, int *error)
{
	char *token = strtok(line, SEPARATORS);
	while (token) {
		dfa_node_t *node = dfa_find_node(token, dfa);
		if (!node) {
			print_error(ERROR_INVALID_STATE);
			*error = 1;
			return;
		}

		if (node->type == NODE_START)
		    node->type = NODE_START_END;
		else
		    node->type = NODE_END;
		token = strtok(NULL, SEPARATORS);
	}
}

char *validate_word(char *line, char *alphabet, int *error)
{
	int i;
	size_t len = strlen(line);

	for (i = 0; i < len; ++i) {
		if (!strchr(alphabet, line[i])) {
			print_error(ERROR_INVALID_SYMBOL_WORD);
			*error = 1;
			break;
		}
	}

	return line;
}

int check_word(char *alphabet, dfa_node_t *dfa, char *word)
{
	dfa_node_t *current_node = dfa_find_node_by_type(NODE_START, dfa);
	if (!current_node)
		current_node = dfa_find_node_by_type(NODE_START_END, dfa);

	char current_symbol;
	while (*word) {
		current_symbol = *word;
		dfa_transition_t *transition = current_node->transitions;

		int transition_found = 0;
		while (transition) {
			if (transition->symbol == current_symbol) {
				transition_found = 1;
				current_node = transition->node;
				break;
			}
			transition = transition->next;
		}

		if (!transition_found)
			break;
		++word;
	}

	if (!*word && (current_node->type == NODE_END || current_node->type == NODE_START_END))
		return 1;
	return 0;
}

int main(int argc, char*argv[])
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	dfa_node_t *dfa = NULL;
	int error = 0;

	read = getline(&line, &len, stdin);
	if (!read)
		return 2;
	parse_states(line, &dfa);

	read = getline(&line, &len, stdin);
	if (!read)
		return 3;
	char *alphabet = strdup(parse_alphabet(line));

	while ((read = getline(&line, &len, stdin)) != -1) {
		if (line[0] == '#')
			break;
		parse_transition(line, dfa, alphabet, &error);
		if (error)
			goto cleanup;
	}

	read = getline(&line, &len, stdin);
	if (!read)
		return 4;
	parse_initial_state(line, dfa, &error);
	if (error)
		goto cleanup;

	read = getline(&line, &len, stdin);
	if (!read)
		return 5;
	parse_finish_states(line, dfa, &error);
	if (error)
		goto cleanup;

	validate_dfa(dfa, alphabet, &error);
	if (error)
		goto cleanup;

	read = getline(&line, &len, stdin);
	if (!read)
		return 6;

	char *token = strtok(line, SEPARATORS);
	if (!token) {
		dfa_node_t *node = dfa_find_node_by_type(NODE_START_END, dfa);
		printf(" %s ;\n", node != 0 ? "accepted" : "rejected");
	}

	while (token) {
		error = 0;
		char *word = validate_word(token, alphabet, &error);
		if (!error) {
			int ok = check_word(alphabet, dfa, word);
			printf("%s %s ;\n", word, ok ? "accepted" : "rejected");
		}
		token = strtok(NULL, SEPARATORS);
	}

cleanup:
	free(alphabet);
	free(line);
	dfa_clear(dfa);

	return 0;
}
