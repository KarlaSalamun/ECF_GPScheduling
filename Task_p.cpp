//
// Created by karla on 01. 04. 2020..
//

#include "Task_p.h"
#include <cmath>
#include <math.h>
#include <cstdio>

double Task_p::compute_tardiness()
{
    time_ended = time_started + duration;
    return time_ended > abs_due_date ? time_ended-abs_due_date : 0;
}

void Task_p::set_arrival_time()
{
    arrival_time = phase + ( instance - 1 ) * period;
}

void Task_p::set_abs_dd()
{
    abs_due_date = phase + ( instance - 1 ) * period + due_date;
}

void Task_p::update_params()
{
    arrival_time = phase + ( instance - 1 ) * period;
    abs_due_date = phase + ( instance - 1 ) * period;
}

bool Task_p::isReady( double time )
{
    return fabs( time - arrival_time ) < 0.001;
}

bool Task_p::isFinished( double time )
{
    if( isPreempted ) {
        if( isgreaterequal( time, time_started + remaining ) ) {
            isPreempted = false;
            return true;
        }
    }
    return isgreaterequal( time,  time_started + duration );
}

void Task_p::update_tardiness( double time )
{
    if( time > abs_due_date ) {
//        printf("yo");
        tardiness += time - abs_due_date;
    }
}

void Task_p::inc_instance()
{
    instance++;
}

void Task_p::set_remaining( double time )
{
    remaining = time_started + duration - time;
}

