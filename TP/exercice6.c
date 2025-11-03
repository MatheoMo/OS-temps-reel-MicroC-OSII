#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "includes.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_pio_regs.h"

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];
OS_STK    task_key_stk[TASK_STACKSIZE];

#define TASK1_PRIORITY 2
#define TASK2_PRIORITY 3
#define TASK_KEY_PRIORITY 4

void Task1_StartGame(void* pdata);
void Task2_GuessNumber(void* pdata);
void Read_Key(void* pdata);

// Variables partagées
volatile INT8U key_value = 3;
volatile INT16U sw_value = 0;
volatile INT16U secret_number = 0;
volatile INT8U game_active = 0;
INT8U err;

// Sémaphores
OS_EVENT *KeyPressSem;
OS_EVENT *GameStartSem;

void Read_Key(void* pdata){
    while (1)
    {
        do {
            key_value = IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE);
            OSTimeDlyHMSM(0, 0, 0, 10);
        } while (key_value == 3);
        
        if (key_value == 2 && !game_active)
        {
            OSSemPost(KeyPressSem);
        }
        
        do {
            OSTimeDlyHMSM(0, 0, 0, 50);
        } while (IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE) != 3);
    }
}

void Task1_StartGame(void* pdata){
    printf("Voulez-vous commencer une nouvelle partie ?\n");
    printf("Appuyez sur KEY[0] pour commencer\n\n");
    
    while (1)
    {
        OSSemPend(KeyPressSem, 0, &err);
        
        printf("\n==> Nouvelle partie commencée!\n");
        
        srand(OSTimeGet());
        secret_number = rand() % 1001;
        printf("Un nombre secret entre 0 et 1000 a été généré...\n");
        printf("Devinez le nombre en utilisant les switches!\n\n");

        game_active = 1;
        
        OSSemPost(GameStartSem);
    }
}

void Task2_GuessNumber(void* pdata){
    INT16U user_guess;
    INT16U attempts = 0;
    
    while (1)
    {
        OSSemPend(GameStartSem, 0, &err);
        
        attempts = 0;
        printf("\nDébut du jeu\n");
        
        while (game_active)
        {
            sw_value = IORD_ALTERA_AVALON_PIO_DATA(SW_BASE) & 0x3FF;
            
            do {
                key_value = IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE);
                OSTimeDlyHMSM(0, 0, 0, 10);
            } while (key_value == 3);
            
            if (key_value == 2)
            {
                user_guess = sw_value;
                attempts++;
                
                printf("Tentative #%d : Vous proposez %d\n", attempts, user_guess);
                
                if (user_guess < secret_number)
                {
                    printf("Trop petit!\n\n");
                    IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0x01);
                }
                else if (user_guess > secret_number)
                {
                    printf("Trop grand!\n\n");
                    IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0x02);
                }
                else
                {
                    printf("\n\n Victoire ! \n");
                    printf("Vous avez trouvé le nombre %d en %d tentatives!\n\n", secret_number, attempts);
                    
                    for (int i = 0; i < 5; i++)
                    {
                        IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0xFF);
                        OSTimeDlyHMSM(0, 0, 0, 200);
                        IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0x00);
                        OSTimeDlyHMSM(0, 0, 0, 200);
                    }
                    
                    game_active = 0;
                    printf("Appuyez sur KEY[0] pour une nouvelle partie.\n\n");
                    OSSemPost(KeyPressSem);
                }
            }
            
            do {
                OSTimeDlyHMSM(0, 0, 0, 50);
            } while (IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE) != 3);
        }
    }
}

int main(void){
    OSInit();
    
    KeyPressSem = OSSemCreate(0);
    GameStartSem = OSSemCreate(0);

    OSTaskCreateExt(Read_Key,
                    NULL,
                    (void *)&task_key_stk[TASK_STACKSIZE-1],
                    TASK_KEY_PRIORITY,
                    TASK_KEY_PRIORITY,
                    task_key_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);
    
    OSTaskCreateExt(Task1_StartGame,
                    NULL,
                    (void *)&task1_stk[TASK_STACKSIZE-1],
                    TASK1_PRIORITY,
                    TASK1_PRIORITY,
                    task1_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);
    
    OSTaskCreateExt(Task2_GuessNumber,
                    NULL,
                    (void *)&task2_stk[TASK_STACKSIZE-1],
                    TASK2_PRIORITY,
                    TASK2_PRIORITY,
                    task2_stk,
                    TASK_STACKSIZE,
                    NULL,
                    0);

    OSStart();
    return 0;
}