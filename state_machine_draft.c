#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum STATE{
    WAIT_DIAL,
    WELCOME,
    PW_VAL,
    BLOCK, 
    MENU,
    MSG_PLAYBACK,
    PW_CHANGE,
}STATE;

typedef enum OPTS{
    PLAYBACK_MAILBOX,
    CHANGE_PASSWORD,
}OPTS;

/*  SUPPORT FUNCTIONS  */

bool wait_tone(){
    //ESPERAR POR UN TONO VALIDO
    return true;
}
bool validation(int password){
    //RECIBIR PW Y VALIDAR
    return true;
}
OPTS read_opt(){
    //LEER OPCION
    return PLAYBACK_MAILBOX;
}
int get_new_password(){
    int password = 0x0000;
    return password;
}
void password_playback(int password){

}
/*  STATE MACHINE  */
int controller(/*argumentos?*/){
    static int password = 0x0000;
    static STATE curr_state;
    static STATE next_state = WAIT_DIAL;
    while(true){
        curr_state = next_state;
        switch(curr_state){
            
            case WAIT_DIAL:
                bool valid = wait_tone();
                next_state = (valid == true) ? WELCOME : WAIT_DIAL;
                break;
            
            case WELCOME:
                //REPRODUCIR MENSAJE BIENVENIDA
                next_state = PW_VAL;
                break;
            
            case PW_VAL:
                bool valid_pw = validation(password); 
                next_state = (valid_pw == true) ? MENU : BLOCK;
                break;
            
            case BLOCK:
                //REPRODUCIR MENSAJE BLOQUEO
                next_state = PW_VAL;
                break;
            
            case MENU:
                //MENU MESSAGE
                OPTS opt = read_opt();
                 next_state = (opt == PLAYBACK_MAILBOX) ? MSG_PLAYBACK : PW_CHANGE;
                break;
            
            case MSG_PLAYBACK:
                //REPRODUCIR MENSAJES
                return 0;
                
            case PW_CHANGE:
                password = get_new_password();
                password_playback(password);
                return 0;
        }
    }

}

/*  MAIN  */

int main(){
    controller();
    printf("hola\n");
    return 0;
}
