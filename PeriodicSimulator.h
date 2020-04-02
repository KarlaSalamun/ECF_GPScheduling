//
// Created by karla on 01. 04. 2020..
//

#ifndef GP_SCHEDULING_PERIODICSIMULATOR_H
#define GP_SCHEDULING_PERIODICSIMULATOR_H

#include <cstdio>
#include <vector>
#include <ecf/ECF.h>
#include "math.h"
#include "Task_p.h"

class PeriodicSimulator {
public:
    PeriodicSimulator( double time_slice, double finish_time, Tree::Tree* heuristic ) :
        time_slice( time_slice ), finish_time( finish_time ), heuristic( heuristic )
    {
        abs_time = 0;
        missed = 0;
    }
    void run();
    void initialize();
    void update_params( Task_p *&task, double time );
    std::vector<Task_p *> test_tasks;
    int missed;
    double total_tardiness;
    void set_test_tasks( std::vector<Task_p *> tasks );
    void reset_params();

private:
    std::vector<Task_p *> pending;
    std::vector<Task_p *> ready;
    double time_slice;
    double finish_time;
    Tree::Tree* heuristic;
    double abs_time;
};


#endif //GP_SCHEDULING_PERIODICSIMULATOR_H
