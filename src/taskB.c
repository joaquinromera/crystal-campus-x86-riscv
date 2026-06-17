/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
*/

#include "syscall.h" 

void task() {
    
    uint32_t id = syscall_getId();

    if( FALSE ) {
        
        // Tarea 1
        // se mueve en distintas direcciones
        // permite diferenciar si estamos
        // haciendo bien los movimientos
        for(int i=0;i<10; i++)
            syscall_move(Forward);
        for(int i=0;i<5; i++)
            syscall_move(Right);
        for(int i=0;i<5; i++)
            syscall_move(Forward);
        for(int i=0;i<5; i++)
            syscall_move(Left);    
        for(int i=0;i<3; i++)
            syscall_move(Back);
        while(1) { __asm __volatile("mov $2, %%eax":::"eax"); }
        
    } else if( id == 5 ){
        
        // Tarea 5
        // genera un page fault,
        // permite determinar si estamos
        // manejando bien el control de 
        // errores.
        syscall_move(Forward);
        syscall_move(Forward);
        syscall_move(Forward);
        syscall_move(Forward);
        syscall_move(Forward);
        syscall_move(Right);
        syscall_move(Right);
        syscall_move(Right);
        return; 
        
    } else {
        
        // Tarea i
        // se mueve hasta juntar algo y luego
        // regresa. Permite ver si estamos
        // capturando bien los cristales y
        // contando puntos correctamente
        while(1) {
            int32_t cont=1;
            int32_t count=0;
            int32_t crystals=0;
            while(cont) {
                syscall_move(Forward);
                count++;
                int32_t a = syscall_take();
                if(a != -1) crystals = crystals + a;
                if(crystals >= 15) cont=0;
                if(count == 50) cont=0;
            }
            for(int i=0;i<count; i++) {
                syscall_move(Back);
            }
            syscall_move(Right);
        }
    }
}
