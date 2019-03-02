#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_EOF,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

enum {
  ND_NUM = 256,
  ND_IDENT,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  char name;
} Node;

Vector *tokens;
int pos = 0;

void push_token(int ty, char *input, int val) {
  Token *token = malloc(sizeof(Token));
  token->ty = ty;
  token->val = val;
  token->input = input;
  vec_push(tokens, token);
}

Token *get_token(int pos) {
  if (tokens->len <= pos)
    return NULL;
  return tokens->data[pos];
}

void tokenize(char *p) {
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';') {
      push_token(*p, p, 0);
      p++;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      push_token(TK_IDENT, p, 0);
      p++;
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

Node *new_node_ident(char name) {
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
    return new_node_ident(get_token(pos++)->val);

  error("expects number or parenthesis but found: %s", get_token(pos)->input);
}

Node *code[100];

void program() {
  int i = 0;
  while (get_token(pos)->ty != TK_EOF) {
    code[i++] = stmt();
  }
  code[i] = NULL;
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
    break;
  }

  printf("  push rax\n");
}

void error(int i) {
  fprintf(stderr, "Unexpected token: %s\n", get_token(i)->input);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments!\n");
    return 1;
  }

  tokens = new_vector();

  tokenize(argv[1]);
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(code[0]);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
