#include <stdio.h>
#include "includes.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_pio_regs.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task_key_stk[TASK_STACKSIZE];
OS_STK    task_game_stk[TASK_STACKSIZE];

#define TASK_KEY_PRIORITY 2
#define TASK_GAME_PRIORITY 3


void Read_SW(void* pdata);
void startGame(void);

// variavles partagées
INT8U sw_value = 0;
INT8U key_value = 0;

void startGame(void){
    if(sw_value == 1){
        printf("Nouvelle partie commencée!\n");
    } else if (sw_value == 2)
    {
        printf("Continuer la partie en cours.\n");
    }
}

void Read_Key(void* pdata){
    while (1)
    {
        do {
            key_value = IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE);
        } while (key_value == 3);
        
        printf("Bouton pressé: KEY[%d]\n", key_value);
        
        do {
            OSTimeDlyHMSM(0, 0, 0, 50);
        } while (IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE) != 3);
    }
}

int main(void){
    OSInit();
    OSTaskCreateExt(Read_Key,
                    NULL,
                    (void *)&task_key_stk[TASK_STACKSIZE-1],
                    TASK_KEY_PRIORITY,
                    TASK_KEY_PRIORITY,
                    task_key_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);
    OSTaskCreateExt(startGame,
                    NULL,
                    (void *)&task_game_stk[TASK_STACKSIZE-1],
                    TASK_GAME_PRIORITY,
                    TASK_GAME_PRIORITY,
                    task_game_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);
    printf("Voulez-vous commencer une nouvelle partie ? ");
    OSStart();
    return 0;
}