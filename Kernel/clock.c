#include <clock.h>

// BORRAR ESTO EN CASO DE DEJARLO ASÍ
// Quería hacer algo similar a un ADT de time pero sin malloc, calloc, está difícil.

// También intenté un struct sin el timeInString y queríendo hacer un método toString donde tenía
// de parámetro un Time, peeeero nuevamente sin malloc y calloc no puedo devolver un string, salvo que lo
// pase de parámetro el string que devuelvo... pero siento que es más engorroso porque el usuario tendría que saber la 
// dimensión de ese string que pasar de parámetro
void getCurrentTime(Time* toReturn){
    toReturn->hours = getCurrentHours();
    toReturn->minutes = getCurrentMinutes();
    toReturn->seconds = getCurrentSeconds();
    
    (toReturn->timeInString)[TIME_STR_LEN-1] = 0;
    (toReturn->timeInString)[0] = ((toReturn->hours) / 10) + '0';
    (toReturn->timeInString)[1] = ((toReturn->hours) % 10) + '0';
    (toReturn->timeInString)[2] = ':';
    (toReturn->timeInString)[3] = ((toReturn->minutes) / 10) + '0';
    (toReturn->timeInString)[4] = ((toReturn->minutes) % 10) + '0';
    (toReturn->timeInString)[5] = ':';
    (toReturn->timeInString)[6] = ((toReturn->seconds) / 10) + '0';
    (toReturn->timeInString)[7] = ((toReturn->seconds) % 10) + '0';
}

int compare(Time t1, Time t2){
    int cmp = t1.hours - t2.hours;
    if(cmp == 0){
        cmp = t1.minutes - t2.minutes;
        if(cmp == 0){
            cmp = t1.seconds - t2.seconds;
        }
    }
    return cmp;
}

void sleep(int seconds){
    Time secInTime = {seconds/3600, (seconds/60)%60, seconds%60, ""};
    sleepWithTime(secInTime);
}

void sleepWithTime(Time time){
    Time endTime; getCurrentTime(&endTime);
    sumTimes(&endTime, time);

    Time currentTime; getCurrentTime(&currentTime);
    while(compare(currentTime, endTime) < 0){
        getCurrentTime(&currentTime);
    }
}

void sumTimes(Time * t1, Time t2){
    t1->seconds += t2.seconds;
    t1->minutes += (t2.minutes + (t1->seconds)/60);
    t1->hours += (t2.hours + (t1->minutes)/60);
    t1->minutes = (t1->minutes) % 60;
    t1->seconds = (t1->seconds) % 60;
}