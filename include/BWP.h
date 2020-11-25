//
// Created by karla on 15. 04. 2020..
//

#ifndef FM_SCHEDULING_BWP_H
#define FM_SCHEDULING_BWP_H

#include <ecf/ECF.h>
#include "RTO.h"

class BWP : public RTO {
public:
    BWP( int task_number, UunifastCreator *creator, Scheduler *sched, double finish_time, bool display_sched ) :
        RTO( creator, task_number, sched, finish_time, display_sched) {}
    virtual void simulate( double time_slice );

    double get_qos() {
        return qos;
    }

    double get_wasted() {
        return wasted;
    }

    void set_heuristic( Tree::Tree *heuristic ) {
        this->heuristic = heuristic;
    }

    void update_state( Task *current, std::vector<Task *> ready_tasks );

private:
    Tree::Tree *heuristic;
    size_t bwp_completed;
    size_t bwp_missed;
    double qos;
    double wasted;
};

#endif //FM_SCHEDULING_BWP_H
