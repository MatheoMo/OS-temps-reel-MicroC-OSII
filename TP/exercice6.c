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
void startGame(void* pdata);

volatile INT8U key_value = 3;
INT8U err;

OS_EVENT *KeyPressSem;

void startGame(void* pdata){
    printf("Voulez-vous commencer une nouvelle partie ? \n");
    printf("KEY[0] = Nouvelle partie, KEY[1] = Continuer\n\n");
    
    while (1)
    {
        OSSemPend(KeyPressSem, 0, &err);
        
        if (key_value == 2)  // KEY[0] pressé
        {
            printf("\n==> Nouvelle partie commencée!\n\n");
        }
        else if (key_value == 1)  // KEY[1] pressé
        {
            printf("\n==> Continuer la partie en cours.\n\n");
        }
    }
}

void Read_Key(void* pdata){
    while (1)
    {
        do {
            key_value = IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE);
            OSTimeDlyHMSM(0, 0, 0, 10);
        } while (key_value == 3);
        
        printf("Bouton pressé: KEY[%d]\n", key_value);
        
        OSSemPost(KeyPressSem);
        
        do {
            OSTimeDlyHMSM(0, 0, 0, 50);
        } while (IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE) != 3);
    }
}

int main(void){
    OSInit();
    
    KeyPressSem = OSSemCreate(0);
    
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
    
    OSStart();
    return 0;
}