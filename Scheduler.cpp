//
// Created by karla on 01. 04. 2020..
//

#include "Scheduler.h"
#include <algorithm>
#include <cstdio>
#include "Scheduler.h"

struct {
    bool operator()(Task_p *a, Task_p *b) const
    {
        return a->priority < b->priority;
    }
} customLess;

void Scheduler::schedule_next( std::vector<Task_p *> &pending, std::vector<Task_p *> &ready, Task_p *&running, double time )
{
    tctx.ready.clear();
    tctx.pending.clear();

    std::copy( pending.begin(), pending.end(), std::back_inserter( tctx.pending ) );
    std::copy( ready.begin(), ready.end(), std::back_inserter( tctx.ready ) );

        // printf( "pending size : %d\n", pending.size() );

    for( auto & element : ready ) {         // TODO: staviti pointer na vektore u tctx 
        // printf("%f %f\n", element->period, element->duration);
        assert( element->duration >= 0 && element->duration < element->period);

        tctx.task = element;
        heuristic->execute( &tctx );
        element->priority = tctx.task->priority;
        // printf( "%lf\n", element->priority );            // TODO priority je float
       
    }

    std::sort( ready.begin(), ready.end(), customLess );

    if( running ) {
        if ( preempt( ready[0], running ) ) {
//            printf( "task %d is preempted, ", running->id );
            running->isPreempted = true;
            running->set_remaining( time );
//            printf( "remaining time %f\n", running->remaining );
            std::swap( ready[0], running );
            set_start( running, time );
        }
    }
    else {
        running = std::move( ready[0] );
        set_start( running, time );
        // running->set_remaining( time );
        ready.erase( ready.begin() );
    }

//    printf( "task %d is running!\n", running->id );
}

void Scheduler::set_start( Task_p *&task, double time )
{
    task->time_started = time;
}

void Scheduler::set_end( Task_p *&task, double time )
{
    task->time_ended = time;
}

bool Scheduler::preempt( const Task_p *t1, const Task_p *t2 )
{
    return t1->priority < t2->priority;
}
