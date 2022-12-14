#include "contr.h"
#include "so.h"

//cd ../../mnt/c/Users/guerr/OneDrive/SO/Trabalho-2---SO/
//cd ../../mnt/c/Users/Deivis\ Felipe/OneDrive/SO/Trabalho-2---SO/
//valgrind --track-origins=yes --log-file=filename ./teste
int main()
{
  contr_t *contr = contr_cria();
  so_t *so = so_cria(contr);
  contr_informa_so(contr, so);
  contr_laco(contr);
  contr_destroi(contr);
  return 0;
}
