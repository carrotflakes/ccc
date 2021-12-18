#ifndef CCC_H
#define CCC_H

enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_EQ,
  TK_NEQ,
  TK_EOF,
  TK_GE,
  TK_LE,
};

typedef struct {
  int ty;
  int val;
  char *ident;
  char *input;
} Token;

enum {
  ND_NUM = 256,
  ND_IDENT,
  ND_EQ,
  ND_NEQ,
  ND_EOF,
  ND_GT,
  ND_GE,
  ND_LT,
  ND_LE,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  char *name;
} Node;

Token *get_token(int pos);
void tokenize(char *p);
Node **program();

void gen_all(Node **node);

#endif
