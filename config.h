#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#define MIN_FSIZE      5
#define MAX_FSIZE      70

/* ограничение на количество очков у волка в процессе моделирования */
#define MAX_WOLF_VITALITY  300

extern int FSIZE;
extern int chance;
extern int vitality;

void input_parameters(void);

#endif
