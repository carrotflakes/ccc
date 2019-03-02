#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "ccc.h"

Vector *tokens;
int pos = 0;

void push_token(int ty, char *input, int val) {
  Token *token = malloc(sizeof(Token));
  token->ty = ty;
  token->val = val;
  token->input = input;
  vec_push(tokens, token);
}

void push_token_ident(char *input, char *ident) {
  Token *token = malloc(sizeof(Token));
  token->ty = TK_IDENT;
  token->ident = ident;
  token->input = input;
  vec_push(tokens, token);
}

Token *get_token(int pos) {
  if (tokens->len <= pos)
    return NULL;
  return tokens->data[pos];
}

void tokenize(char *p) {
  tokens = new_vector();

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '='|| *p == ';') {
      push_token(*p, p, 0);
      p++;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      int len = 0;
      while (('a' <= p[len] && p[len] <= 'z') ||
             ('0' <= p[len] && p[len] <= '9') ||
             p[len] == '_')
        len++;
      char *ident = malloc(sizeof(char) * len + 1);
      strncpy(ident, p, len);
      ident[len] = 0;
      push_token_ident(p, ident);
      p += len;
      continue;
    }

    if (isdigit(*p)) {
      push_token(TK_NUM, p, strtol(p, &p, 10));
      continue;
    }
    fprintf(stderr, "Tokenize failed: %s\n", p);
    exit(1);
  }

  push_token(TK_EOF, p, 0);
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_ident(char *name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  return node;
}

int consume(int ty) {
  Token *token = get_token(pos);
  if (token == NULL || token->ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *stmt();
Node *assign();
Node *add();
Node *mul();
Node *term();

Node *stmt() {
  Node *node = assign();

  if (!consume(';'))
    error("expects ';' but found: %s", get_token(pos)->input);
  return node;
}

Node *assign() {
  Node *node = add();

  if (consume('='))
    return new_node('=', node, assign());
  else
    return node;
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = term();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, term());
    else if (consume('/'))
      node = new_node('/', node, term());
    else
      return node;
  }
}

Node *term() {
  if (consume('(')) {
    Node *node = add();
    if (!consume(')'))
      error("unclosed parenthesis: %s", get_token(pos)->input);
    return node;
  }

  if (get_token(pos)->ty == TK_NUM)
    return new_node_num(get_token(pos++)->val);
  if (get_token(pos)->ty == TK_IDENT)
    return new_node_ident(get_token(pos++)->ident);

  error("expects number or parenthesis but found: %s", get_token(pos)->input);
}

Node **program() {
  Node **code = malloc(sizeof(Node *) * 100);
  int i = 0;
  while (get_token(pos)->ty != TK_EOF) {
    code[i++] = stmt();
  }
  code[i] = NULL;
  return code;
}
