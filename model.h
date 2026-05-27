#ifndef MODEL_H
#define MODEL_H

/* типы животных */
#define ANIMAL_NONE    0
#define ANIMAL_RABBIT  1
#define ANIMAL_WOLF_M  2
#define ANIMAL_WOLF_F  3
#define ANIMAL_CAT     100

typedef struct animal_s { /* структура, описывающая животное */
  struct animal_s *next;  /* указатель на следующую структуру в списке */
  struct animal_s *prev;  /* указатель на предыдущую структуру в списке */
  int x;         /* координаты животного */
  int y;
  int type;       /* тип животного */
  int vitality;   /* очки жизненной силы */
  int age;        /* возраст животного */
} animal_t;

typedef struct animal_list_s { /* структура списка */
  animal_t *first;  /* указатель на первую структуру в списке */
  animal_t *last;   /* указатель на последнюю структуру в списке */
} animal_list_t;

extern animal_list_t TheList; /* переменная-список животных */

extern int CntAnimals;  /* количество животных */
extern int StepCounter; /* номер шага моделирования */

void model_init (void);
void model_finish(void);
void model (void);
int is_model_finished(void);

#endif
