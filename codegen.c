#include <stdio.h>
#include <stdlib.h>
#include "map.h"
#include "ccc.h"

void error(int i) {
  fprintf(stderr, "Unexpected token: %s\n", get_token(i)->input);
  exit(1);
}

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT)
    error("invalid lvalue");

  int offset = ('z' - node->body.ident[0] + 1) * 8;
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", offset);
  printf("  push rax\n");
}

int label_index = 0;

void gen(Node *node) {
  switch (node->ty) {
  case ND_NUM:
    printf("  push %d\n", node->body.val);
    return;
  case ND_IDENT:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case '=':
    gen_lval(node->body.ope.lhs);
    gen(node->body.ope.rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  case ND_IF:
    gen(node->body.if_stmt.cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");

    if (node->body.if_stmt.els) {
      int lelse_index = label_index++;
      int lend_index = label_index++;
      printf("  je .Lelse%d\n", lelse_index);
      gen(node->body.if_stmt.then);
      printf("  jmp .Lend%d\n", lend_index);
      printf(".Lelse%d:\n", lelse_index);
      gen(node->body.if_stmt.els);
      printf(".Lend%d:\n", lend_index);
    } else {
      int lend_index = label_index++;
      printf("  je .Lend%d\n", lend_index);
      gen(node->body.if_stmt.then);
      printf(".Lend%d:\n", lend_index);
    }
    return;
  case ND_RETURN:
    gen(node->body.ope.lhs);
    return;
  }

  gen(node->body.ope.lhs);
  gen(node->body.ope.rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NEQ:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_GT:
    printf("  cmp rax, rdi\n");
    printf("  setg al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_GE:
    printf("  cmp rax, rdi\n");
    printf("  setge al\n");
    printf("  movzb rax, al\n");
    break;
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
  default:
    fprintf(stderr, "invalid node type: %d\n", node->ty);
    exit(1);
  }

  printf("  push rax\n");
}

void collect_vars(Map *vars, Node *node) {
  switch (node->ty) {
  case ND_NUM:
    return;
  case ND_IDENT:
    if (map_get(vars, node->body.ident) == NULL) {
      int *v = malloc(sizeof(int));
      *v = map_size(vars);
      map_put(vars, node->body.ident, v);
    }
    return;
  case ND_EQ:
  case ND_NEQ:
  case ND_GT:
  case ND_GE:
  case ND_LT:
  case ND_LE:
  case '=':
  case '+':
  case '-':
  case '*':
  case '/':
    collect_vars(vars, node->body.ope.lhs);
    collect_vars(vars, node->body.ope.rhs);
    return;
  case ND_IF:
    collect_vars(vars, node->body.if_stmt.cond);
    collect_vars(vars, node->body.if_stmt.then);
    if (node->body.if_stmt.els)
      collect_vars(vars, node->body.if_stmt.els);
    return;
  case ND_RETURN:
    collect_vars(vars, node->body.ope.lhs);
    return;
  default:
    fprintf(stderr, "invalid node type: %d\n", node->ty);
    exit(1);
  }
}

void gen_all(Node **nodes) {
  Map *vars = new_map();

  for (int i = 0; nodes[i]; i++) {
    collect_vars(vars, nodes[i]);
  }
  fprintf(stderr, "vars: %d\n", map_size(vars));

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  for (int i = 0; nodes[i]; i++) {
    gen(nodes[i]);
    printf("  pop rax\n");
  }

  // epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}
