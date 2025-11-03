#include <stdio.h>
#include "includes.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_pio_regs.h"

void Read_SW(void* pdata);

// variavles partagées
INT8U sw_value = 0;
INT8U key_value = 0;

void Read_Key(void* pdata){
    while (1)
    {
        INT8U key_value = IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE);
        OSTimeDlyHMSM(0, 0, 0, 500);
    }
}
int main(void){
    printf("Voulez-vous commencer une nouvelle partie ? ");
    if(key_value == 1){
        printf("Nouvelle partie commencée!\n");
    } else if (key_value == 2)
    {
        printf("Continuer la partie en cours.\n");
    }

    return 0;
}