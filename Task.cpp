//
// Created by karla on 06. 03. 2020..
//

#include "Task.h"

double Task::compute_tardiness()
{
    double completion_time = time_started + duration;
    return completion_time > due_date ? completion_time-due_date : 0;
}
