#include <stdio.h>
#include "config.h"
#include "console.h"
#include "model.h"

/*******************************************************************************
  void view_all (void)

  Функция визуализации модели на текущем шаге.
  Визуализация осуществляется в FrameBuffer, не сразу в консоль.

  Это сделано потому что
  в процессе работы этой функции происходит очистка и новое заполнение,
  поэтому, чтоб ничего не мерцало и не дергалось, работа идет с FrameBuffer-ом,
  а не сразу с консолью. FrameBuffer выводится в консоль позже, уже полностью
  сформированный и при помощи отдельной функции.

  Функция проходит по списку животных и визуализирует каждое во FrameBuffer
  в зависимости от типа и координат.
*******************************************************************************/
void view_all (void)
{
  int x, y;
  /* Очистка поля во FrameBuffer-e (заполнение точками всех полей) */
  for (y = 1; y < FSIZE+1; y++) {
    for (x = 1; x < 2*FSIZE+1; x+=2) { /* по X масштаб x2 - через один. */
       //pset(x, y, ' ', 0);
       pset(x+1, y, '.', TCLR_BBLUE);
    }
  }
  /* проход по списку животных */
  animal_t *an = TheList.first; /* берется первое животное в списке */
  while (an != NULL) {          /* цикл пока список не закончится */
    /* в зависимости от типа берется символ и цвет */
    int ch, clr;
    if (an->type == ANIMAL_RABBIT) {
      ch = 'R';
      clr = TCLR_BGREEN;
    }
    else if (an->type == ANIMAL_WOLF_M) {
      ch = 'W';
      if (an->vitality < 3) {
        clr = BCLR_BRED; /* волк при смерти будет отображаться на красном фоне */
      }
      else {
        clr = TCLR_BRED;
        if (an->age < 2) {
          clr = TCLR_YELLOW;  /* волчата отображаются желтым */
        }
      }
    }
    else if (an->type == ANIMAL_WOLF_F) {
      ch = 'w';
      if (an->vitality < 3) {
        /* волчица при смерти будет отображаться на фиолетовом фоне */
        clr = BCLR_BMAGENTA;
      }
      else {
        clr = TCLR_BMAGENTA;
        if (an->age < 2) {
          clr = TCLR_YELLOW;  /* волчата отображаются желтым */
        }
      }
    }
    else {
      /* на всякий случай, хотя и не должно быть незнакомых животных в списке */
      ch = '.';
      clr = TCLR_BBLUE;
    }

    /* вывод символа и цвета FrameBuffer по координатам животного
     * (по X масштаб x2) */
    pset(an->x * 2, an->y, ch, clr);
    /* переход к следующему животному в списке */
    an = an->next;
  }
  /* вывод текущей статистики снизу от игрового поля */
  cursor_position(0, FSIZE+2);
  printf("Animals = %d, step = %d       ", CntAnimals, StepCounter);
  cursor_position(0, FSIZE+4);
  printf("FSIZE = %d, Chance = %d, Vitality = %d", FSIZE, chance, vitality);
}