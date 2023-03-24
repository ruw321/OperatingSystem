#include "scheduler.h"

int main(int argc, char const *argv[])
{

    signal(SIGINT, SIG_IGN);  // Ctrl-C
    signal(SIGQUIT, SIG_IGN); /* Ctrl-\ */
    signal(SIGTSTP, SIG_IGN); // Ctrl-Z

    make_scheduler_context();
    set_alarm_handler();
    set_timer();

    swapcontext(&main_context, &scheduler_context);

    return 0;
}
