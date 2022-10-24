#include "alarm.h"

Alarm* alr;

// Alarm function handler
void alarmHandler(int signal){
    alr->alarmEnabled = FALSE;
    alr->alarmRang = TRUE;
    alr->alarmCount++;
}

void resetAlarm(){
    alr->alarmEnabled = FALSE;
    alr->alarmCount = 0;
    alr->alarmRang = FALSE;
}

int setAlarm(int time){
    (void)signal(SIGALRM, alarmHandler);
    if (alr->alarmEnabled == FALSE)
        {
            alarm(time); // Set alarm to be triggered in 3s
            alr->alarmEnabled = TRUE;
        }
    return 0;
}

void stopAlarm() {
	struct sigaction action;
	action.sa_handler = NULL;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGALRM, &action, NULL);
	alarm(0);
}