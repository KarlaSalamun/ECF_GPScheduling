//
// Created by karla on 01. 04. 2020..
//

#ifndef GP_SCHEDULING_TASK_P_H
#define GP_SCHEDULING_TASK_P_H

#include "Task.h"

class Task_p : public Task  {
    public:
        int id;
        double phase;
        int instance;
        double period;
        double arrival_time;
        bool isPreempted;
        double tardiness;
        double time_ended;

        double compute_tardiness();
        void set_abs_dd();
        bool isReady( double time );
        void inc_instance();
        bool isFinished( double time );
        void update_tardiness( double time );
        void update_params();
        void set_arrival_time();
        void set_remaining( double time );

        Task_p( double phase, int instance, double period ) :
                phase( phase ), instance( instance ), period( period )
        {
            tardiness = 0;
            isPreempted = false;
            due_date = period;
            weight = 1;
        }
        Task_p() {
            tardiness = 0;
            isPreempted = false;
            due_date = period;
            weight = 1;
        }
        ~Task_p() {}
private:
    double abs_due_date;
    double remaining;
};


#endif //GP_SCHEDULING_TASK_P_H
