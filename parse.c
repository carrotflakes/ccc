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

    if (strncmp(p, "==", 2) == 0) {
      push_token(TK_EQ, p, 0);
      p += 2;
      continue;
    }
    if (strncmp(p, "!=", 2) == 0) {
      push_token(TK_NEQ, p, 0);
      p += 2;
      continue;
    }
    if (strncmp(p, ">=", 2) == 0) {
      push_token(TK_GE, p, 0);
      p += 2;
      continue;
    }
    if (strncmp(p, "<=", 2) == 0) {
      push_token(TK_LE, p, 0);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '='|| *p == ';' ||
        *p == '>' || *p == '<' || *p == '{' || *p == '}') {
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
      
      if (strncmp(p, "if", 2) == 0) {
        push_token(TK_IF, p, 0);
      } else if (strncmp(p, "else", 4) == 0) {
        push_token(TK_ELSE, p, 0);
      } else if (strncmp(p, "while", 5) == 0) {
        push_token(TK_WHILE, p, 0);
      } else if (strncmp(p, "for", 3) == 0) {
        push_token(TK_FOR, p, 0);
      } else if (strncmp(p, "return", 6) == 0) {
        push_token(TK_RETURN, p, 0);
      } else {
        char *ident = malloc(sizeof(char) * len + 1);
        strncpy(ident, p, len);
        ident[len] = 0;
        push_token_ident(p, ident);
      }

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
  node->body.ope.lhs = lhs;
  node->body.ope.rhs = rhs;
  return node;
}

Node *new_node_if(Node *cond, Node *then, Node *els) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IF;
  node->body.if_stmt.cond = cond;
  node->body.if_stmt.then = then;
  node->body.if_stmt.els = els;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->body.val = val;
  return node;
}

Node *new_node_ident(char *name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->body.ident = name;
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
Node *eq();
Node *rel();
Node *add();
Node *mul();
Node *unary();
Node *term();

Node *stmt() {
  if (consume('{')) {
    Node *node = assign();

    if (!consume('}'))
      error("expects '}' but found: %s", get_token(pos)->input);
    return node;
  }

  if (consume(TK_RETURN)) {
    Node *node = assign();

    if (!consume(';'))
      error("expects ';' but found: %s", get_token(pos)->input);
    return new_node(ND_RETURN, node, NULL);
  }

  if (consume(TK_IF)) {
    Node *cond = assign();
    Node *then = stmt();
    Node *els = NULL;
    if (consume(TK_ELSE))
      els = stmt();
    return new_node_if(cond, then, els);
  }

  Node *node = assign();

  if (!consume(';'))
    error("expects ';' but found: %s", get_token(pos)->input);
  return node;
}

Node *assign() {
  Node *node = eq();

  if (consume('='))
    return new_node('=', node, assign());
  else
    return node;
}

Node *eq() {
  Node *node = rel();

  if (consume(TK_EQ))
    node = new_node(ND_EQ, node, rel());
  else if (consume(TK_NEQ))
    node = new_node(ND_NEQ, node, rel());
  else
    return node;
}

Node *rel() {
  Node *node = add();

  if (consume('<'))
    node = new_node(ND_LT, node, add());
  else if (consume('>'))
    node = new_node(ND_GT, node, add());
  else if (consume(TK_LE))
    node = new_node(ND_LE, node, add());
  else if (consume(TK_GE))
    node = new_node(ND_GE, node, add());
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
  Node *node = unary();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, unary());
    else if (consume('/'))
      node = new_node('/', node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume('+'))
    return term();
  if (consume('-'))
    return new_node('-', new_node_num(0), term());
  return term();
}

Node *term() {
  if (consume('(')) {
    Node *node = eq();
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
