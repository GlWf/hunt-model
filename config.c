#include <stdio.h>
#include <string.h>
#include "config.h"

int FSIZE = 20;
int chance;
int vitality;

static int input_int (char *prompt,
                      int min_value,
                      int max_value,
                      int default_value);

/*******************************************************************************
  static int input_int (char *prompt, int min_value,
                        int max_value, int default_value)

  Функция ввода с клавиатуры одного числового параметра.

  Параметры:
    prompt - строка запроса пользователя.
    min_value - минимальное допустимое значение введенного числа.
    max_value - максимальное допустимое значение введенного числа.
    default_value - значение по умолчанию.

  Функция выводит запрос и считывает ввод пользователя.
   - Если ничего не введено (пользователь просто нажал ENTER), то возвращается
     значение по умолчанию.
   - Если введено не число или число вне допустимого диапазона, то возвращается
     значение по умолчанию.
   - Возвращается введенное пользователем число.

*******************************************************************************/
static int input_int (char *prompt, int min_value,
                      int max_value, int default_value)
{
  int inval;
  char input_string[32];

  printf("%s %d...%d [default %d]: ", prompt, min_value,
                                      max_value, default_value);
  fflush(stdout);
  fgets(input_string, sizeof(input_string), stdin);
  if (strlen(input_string) < 2) {
    printf ("  empty input, use default: %d\n", default_value);
    return default_value;
  }
  sscanf(input_string, "%d\n", &inval);
  //printf("inval = %d (%u)\n", inval, strlen(input_string));
  /* проверка диапазона, если inval туда входит, то вернуть inval */
  if ((inval >= min_value) && (inval <= max_value)) {
    return inval;
  }
  else {
    /* если введенное значение НЕ входит в заданный диапазон, то вернуть
     * значение по умолчанию */
    printf ("  input %d is outside range [min, max], use default: %d\n",
                                                     inval, default_value);
    return default_value;
  }
}

/*******************************************************************************
  void input_parameters(void)

  Функция ввода с клавиатуры параметров моделирования.
*******************************************************************************/
void input_parameters(void)
{
  printf ("\n");
  FSIZE    = input_int ("Enter game field size", MIN_FSIZE, MAX_FSIZE, 20);
  chance   = input_int ("Enter born chance of a rabbit", 1, 100, 20);
  vitality = input_int ("Enter the vitality of a wolf", 10, 100, 35);
}
