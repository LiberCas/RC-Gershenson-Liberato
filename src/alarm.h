#ifndef _ALARM_H_
#define _ALARM_H_

#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#define FALSE 0
#define TRUE 1

typedef struct {
    int alarmEnabled;
    int alarmCount;
    int alarmRang;
}Alarm;

void alarmHandler(int signal);
void resetAlarm();
int setAlarm(int time);
void stopAlarm();

#endif 
