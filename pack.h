#define op_NOP 1
#define op_DAT 0
#define op_MOV 2
#define op_ADD 3
#define op_SUB 4
#define op_MUL 5
#define op_DIV 6
#define op_MOD 7
#define op_JMP 8
#define op_JMZ 9
#define op_JMN 10
#define op_DJN 11
#define op_SPL 12
#define op_CMP 13
#define op_SEQ 14
#define op_SNE 15
#define op_SLT 16
#define op_LDP 17
#define op_STP 18
#define op_IMM 0
// #
#define op_DIR 2
// $
#define op_A_IND 3
// *
#define op_B_IND 4
// @
#define op_A_IND_PREDEC 5
// {
#define op_B_IND_PREDEC 6
// <
#define op_A_IND_POSTINC 7
// }
#define op_B_IND_POSTINC 1
// >
#define op_A 0
#define op_B 1
#define op_AB 2
#define op_BA 3
#define op_F 4
#define op_X 5
#define op_I 6

int str_to_code(char *str);
void get_default_mod(struct unpacked_op_mem *op);
void pack(struct unpacked_op_mem *in,void *out);
void unpack(int ip,struct unpacked_op_mem *out);
void pack2mem(int ip,struct unpacked_op_mem *in);
