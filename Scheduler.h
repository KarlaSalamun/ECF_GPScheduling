//
// Created by karla on 01. 04. 2020..
//

#ifndef GP_SCHEDULING_SCHEDULER_H
#define GP_SCHEDULING_SCHEDULER_H

#include <vector>
#include <ecf/ECF.h>
#include "Task_p.h"

struct taskp_ctx {
	Task_p *task;
	std::vector<Task_p *> pending;
	std::vector<Task_p *> ready;
};

class Scheduler {
public:
    Scheduler( Tree::Tree* heuristic ) : heuristic( heuristic ) {}
    virtual void schedule_next( std::vector<Task_p *> &pending, std::vector<Task_p *> &ready, Task_p *&running, double time );

private:
	struct taskp_ctx tctx;
	Tree::Tree* heuristic;
    virtual void set_start( Task_p *&task, double time );
    virtual void set_end( Task_p *&task, double time );
    virtual bool preempt( const Task_p *t1, const Task_p *t2 );
};


#endif //GP_SCHEDULING_SCHEDULER_H
