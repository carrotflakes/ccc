#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

enum {
  TK_NUM = 256,
  TK_EOF,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

enum {
  ND_NUM = 256,
};

typedef struct {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
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
  return tokens->data[pos];
}

void tokenize(char *p) {
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
      push_token(*p, p, 0);
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
  node->val =val;
  return node;
}

int consume(int ty) {
  if (get_token(pos)->ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *mul();
Node *term();

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

  error("expects number or parenthesis but found: %s", get_token(pos)->input);
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
  Node *node = add();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
