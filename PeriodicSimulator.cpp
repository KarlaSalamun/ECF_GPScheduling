//
// Created by karla on 01. 04. 2020..
//
#include <cassert>
#include "PeriodicSimulator.h"
#include "Scheduler.h"

void PeriodicSimulator::initialize()
{
    // TaskCreator *tc = new TaskCreator();
    // tc->set_task_number( 10 );
    // tc->create_periodic_test_set( test_tasks );
    for( auto & element : test_tasks ) {
        element->arrival_time = element->phase;
        element->set_abs_dd();
        // printf( "%d: arrival %f\n", element->id, element->arrival_time );
    }
}

void PeriodicSimulator::run()
{
    pending = test_tasks;
    Scheduler *sched = new Scheduler( heuristic );      // TODO maknuti ovo
    Task_p *running = nullptr;				// TODO: ovo je leak
    while( abs_time < finish_time ) {
        std::vector<Task_p *>::iterator it = pending.begin();

        while( it != pending.end() ) {

            if ( (*it)->isReady( abs_time ) ) {
                (*it)->inc_instance();
//                printf( "task %d is ready!\n", (*it)->id );
                ready.push_back( std::move( *it ) );
                it = pending.erase( it );
            }
            else {
                it++;
            }
        }

        assert( pending.size() <= test_tasks.size() );

        if( !ready.empty() ) {
            sched->schedule_next( pending, ready, running, abs_time );
        }
        assert( pending.size() <= test_tasks.size() );

        it = ready.begin();
        while( it != ready.end() ) {
            // next arrival time is less than current time -> missed task, new instance appeared
            if( islessequal( (*it)->phase + ( (*it)->instance - 1 ) * (*it)->period, abs_time + time_slice ) ) {
//                printf("Missed task %d\n", (*it)->id );
                pending.push_back( *it );
                total_tardiness += (*it)->period;
                it = ready.erase( it );
                // update_params( (*it), abs_time + time_slice );

                missed++;
            }
            else {
                it++;
            }
        }
        assert( pending.size() <= test_tasks.size() );


        abs_time += time_slice;
//        printf( "\ntime: %f\n\n", abs_time );

        if( running ) {
            // update_params( running, abs_time );
            if( running->isFinished( abs_time ) ) {
//                printf( "task %d is finished!\n", running->id );
                running->update_tardiness( abs_time );
                pending.push_back( std::move( running ) );
                running = nullptr;
            }
        }
        assert( pending.size() <= test_tasks.size() );


        for( auto & element : pending ) {
            if( element->phase + element->instance * element->period >= abs_time ) {
                update_params( element, abs_time );
            }

        }
    }
    for( auto & element : pending ){
        total_tardiness += element->tardiness;
    }
    for( auto & element : ready ){
        total_tardiness += element->tardiness;
    }
    if( running ) {
        total_tardiness += running->tardiness;
    }

//    printf("missed tasks: %d\n", missed);
}

void PeriodicSimulator::update_params( Task_p *&task, double time )
{
    // task->update_tardiness( time );
    task->set_arrival_time();
    task->set_abs_dd();
}

void PeriodicSimulator::set_test_tasks( std::vector<Task_p *> tasks )
{
    this->test_tasks = tasks;
}

void PeriodicSimulator::reset_params()
{
    for( auto & element : test_tasks ) {
        element->instance = 1;
        element->isPreempted = false;
        element->arrival_time = element->phase;
        element->set_abs_dd();
        element->tardiness = 0;
    }
}