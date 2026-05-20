#include <stdio.h>
#include "console.h"
#include <string.h>
#include "config.h"
#include "model.h"
#include "view.h"

int AutoMode = 0;

/*------------------------------------------------------------------------*/
int main()
{
    srand(GetTickCount());
    console_title ("Hunt emulation");
    input_parameters();

    console_init (FSIZE*2+2, FSIZE+2); /* инициализация консоли */

    cursor_parameters (75, 0);         /* выключение курсора */
    clear();                           /* очистка FrameBuffer */
    console_clear ();

    /* рамка вокруг игрового поля во FrameBuffer */
    hline(0, 0, FSIZE*2+1, '-', TCLR_GREY);
    hline(FSIZE+1, 0, FSIZE*2+1, '-', TCLR_GREY);
    vline(0, 0, FSIZE+1, '|', TCLR_GREY);
    vline(2*FSIZE+1, 0, FSIZE+1, '|', TCLR_GREY);

    /* инициализация модели */
    model_init();
    /* начальное добавление животных на поле */
    int i, cnt;
    cnt = (rand() % (FSIZE * 2)) + 1; /* случайное количество животных */
    for (i = 0; i < cnt; i++) {
      add_animal_at_random_place (ANIMAL_RABBIT);
    }
    cnt = (rand() % FSIZE) + 1; /* случайное количество животных */
    for (i = 0; i < cnt; i++) {
      add_animal_at_random_place (ANIMAL_WOLF_M);
    }
    cnt = (rand() % FSIZE) + 1; /* случайное количество животных */
    for (i = 0; i < cnt; i++) {
      add_animal_at_random_place (ANIMAL_WOLF_F);
    }

    cursor_position(0, FSIZE+6);
    printf("press SPACE to step\n"
           "press Q or ESCAPE to exit\n"
           "press A to turn on automatic mode");

    view_all();
    redraw();

    while (1)  {
      int key = kbhit (); /* считывание нажатой клавиши без ожидания */

      if ((key == 0x51) || (key == VK_ESCAPE))  { /* клавиша ESC или Q */
        break;  /* выход из программы */
      }
      else if (key == 32) { /* клавиша ПРОБЕЛ - ручной пошаговый режим */
        AutoMode = 0;       /* остановка автоматического режима */
        model();            /* моделирование одного шага */
        view_all();   /* визуализация модели на текущем шаге в FrameBuffer */
        redraw();     /* вывод FrameBuffer в консоль */
        continue;
      }
      else if (key == 0x41) { /* клавиша A */
        AutoMode = 1;         /* запуск автоматического режима */
      }

      if (AutoMode) {
        model();      /* моделирование одного шага в автоматическом режиме */
        view_all();   /* визуализация модели на текущем шаге в FrameBuffer */
        redraw();     /* вывод FrameBuffer в консоль */
        /* если моделирование завершено, то выключить автоматический режим */
        if (is_model_finished()) {
          AutoMode = 0;
        }
        Sleep (50);   /* задержка 50 мс перед следующим шагом */
      }
    }
    model_finish ();
    console_finish();
    return 0;
}
