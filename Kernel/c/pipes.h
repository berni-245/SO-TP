#ifndef TP_ARQUI_2024_PIPES_H
#define TP_ARQUI_2024_PIPES_H

#define BUFFER_SIZE 4096

typedef struct {
    char* name;
    char buffer[BUFFER_SIZE];
    int producingIndex;
    int consumingIndex;
    int mutex;
    int empty;
    int full;
    int read;
    int write;
} pipe_t;

typedef struct {
    int is_used;
    pipe_t* pipe;
} pipe_pos;

//Se llama al iniciar el programa para inicializar todos los pipes
void pipes_birth();
//Permite crear un nuevo pipe (se necesita que se envíe un nombre ÚNICO, si no se le quieren adsignar procesos TODAVÍA
//se envía 0 al process que no tengo (si no le mando el pid), con el join se puede agregar
int create_pipe(char* name, int process_write, int process_read);
//Se asigna el proc read y/o write a un pipe, si no se envían procesos o ya tengo procesos asignados devuelve -1
int join_pipe(const char* pipe_name, int process_write, int process_read);
//permite escribir en un pipe, funciona como la función write (en lo posible)
int write_pipe(int pipe, const char* info, int size);
//permite leer de un pipe (igual que antes funciona como un read)
int read_pipe(int pipe, char* info, int size);
//permite eliminar un pipe para que vuelva a estar disponible
int destroy_pipe(int pipe);





#endif //TP_ARQUI_2024_PIPES_H
