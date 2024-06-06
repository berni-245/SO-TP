#include "pipes.h"
#include "semaphore.h"


//PRIMERA IMPLEMENTACIÓN DE PIPES
//Todos los pipes tienen nombre y funcionan con un semáforo producer, consumer.
//La idea es tener un arreglo circular sobre el que vayan iterando el índice de producer
//y el de consumer.


//TODO ver el tema de los strcat o con un strcpy o algún invento del estilo

#define MAX_PIPES 20


int pipe_size;
pipe_pos pipeArray[MAX_PIPES];

int check_pipe(int pipe) {
    return (pipe < 0 || pipe >= MAX_PIPES || !pipeArray[pipe].is_used);
}

void pipes_birth() {
    for (int i = 0; i < MAX_PIPES; i++) {
        pipeArray[i].is_used = 0;
    }
    pipe_size = 0;
}

int find_available_pipe() {
    if (pipe_size >= MAX_PIPES) {
        return -1;
    }
    for (int i = 0; i < MAX_PIPES; ++i) {
        if (!pipeArray[i].is_used) {
            return i;
        }
    }
    return -1;
}

int find_pipe(const char* name) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if (pipeArray[i].is_used && strcmp(pipeArray[i].pipe->name, name) == 0) {
            return i;
        }
    }
    return -1;
}
//Si no quiero asignar el proceso todavía le mando 0 y con el join lo puedo agregar
int create_pipe(char* name, int process_write, int process_read) {
    if (process_write < 0 || process_read < 0) {
        return -1;
    }
    if (find_pipe(name) != -1) {
        return -1;
    }
    int available_pipe = find_available_pipe();
    if (available_pipe == -1) {
        return -1;
    }
    pipeArray[available_pipe].is_used = 1;
    pipe_size++;
    pipeArray[available_pipe].pipe = malloc(sizeof(pipe_t));
    if (pipeArray[available_pipe].pipe == NULL) {
        pipeArray[available_pipe].is_used = 0;
        pipe_size--;
        return -1;
    }
    strcpy(pipeArray[available_pipe].pipe->name, name);
    pipeArray[available_pipe].pipe->producingIndex = 0;
    pipeArray[available_pipe].pipe->consumingIndex = 0;
    pipeArray[available_pipe].pipe->mutex = my_sem_init(strcat("pipe_m_", name), 1);
    pipeArray[available_pipe].pipe->empty = my_sem_init(strcat("pipe_e_", name), BUFFER_SIZE);
    pipeArray[available_pipe].pipe->full = my_sem_init(strcat("pipe_f_", name), 0);


    pipeArray[available_pipe].pipe->read = process_read;
    pipeArray[available_pipe].pipe->write = process_write;
    return available_pipe;
}

int join_pipe(const char* pipe_name, int process_write, int process_read) {
    int pipe_id = find_pipe(pipe_name);
    if (pipe_id == -1) {
        return -1;
    }
    if (pipeArray[pipe_id].pipe->write == 0 && process_write > 0) {
        pipeArray[pipe_id].pipe->write = process_write;
    } else if (process_write != 0) {
        return -1;
    }
    if (pipeArray[pipe_id].pipe->read == 0 && process_read > 0) {
        pipeArray[pipe_id].pipe->read = process_read;
    } else if (process_read != 0) {
        return -1;
    }
    return 1;
}

int write_pipe(int pipe, const char* info, int size) {
    if (check_pipe(pipe)) {
        return -1;
    }
    if (pipeArray[pipe].pipe->write != 0 && pipeArray[pipe].pipe->read != 0) {
        int pos = 0;
        while (pos < size) {
            my_sem_wait(pipeArray[pipe].pipe->empty);
            my_sem_wait(pipeArray[pipe].pipe->mutex);
            pipeArray[pipe].pipe->buffer[pipeArray[pipe].pipe->producingIndex] = info[pos++];
            pipeArray[pipe].pipe->producingIndex = (pipeArray[pipe].pipe->producingIndex + 1) % BUFFER_SIZE;
            my_sem_post(pipeArray[pipe].pipe->mutex);
            my_sem_post(pipeArray[pipe].pipe->full);
        }
        return pos;
    }
    return -1;
}

int read_pipe(int pipe, char* info, int size) {
    if (check_pipe(pipe)) {
        return -1;
    }
    if (pipeArray[pipe].pipe->write != 0 && pipeArray[pipe].pipe->read != 0) {
        int pos = 0;
        while (pos < size) {
            my_sem_wait(pipeArray[pipe].pipe->full);
            my_sem_wait(pipeArray[pipe].pipe->mutex);
            info[pos++] = pipeArray[pipe].pipe->buffer[pipeArray[pipe].pipe->consumingIndex];
            pipeArray[pipe].pipe->consumingIndex = (pipeArray[pipe].pipe->consumingIndex + 1) % BUFFER_SIZE;
            my_sem_post(pipeArray[pipe].pipe->mutex);
            my_sem_post(pipeArray[pipe].pipe->empty);
        }
        return pos;
    }
    return -1;
}

int destroy_pipe(int pipe) {
    if (check_pipe(pipe)) {
        return -1;
    }
    free(pipeArray[pipe].pipe->name);
    free(pipeArray[pipe].pipe);
    pipeArray[pipe].is_used = 0;
    pipe_size--;
    return 0;
}

