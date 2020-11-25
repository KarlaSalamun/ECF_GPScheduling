//
// Created by karla on 15. 04. 2020..
//
#include <cmath>
#include <algorithm>
#include <cassert>
#include "BWP.h"

void BWP::simulate( double time_slice )
{
    std::string prefix = "./../../test_outputs/";
    FILE *fd = fopen( (prefix + "bwp_schedule.txt").c_str(), "w+" );

    for( auto & element : pending ) {
        element->initialize_task();
        element->set_skip_factor( 2 );
        fprintf( fd, "%lf %lf\n", element->get_period(), element->get_duration() );
    }
    all_tasks = 0;
    bwp_completed = 0;
    bwp_missed = 0;

    wasted = 0;

    Task *running = nullptr;				// TODO: ovo je leak
    abs_time = 0;
    red.clear();
    blue.clear();
    while( abs_time <= finish_time ) {
//        printf( "\ntime: %f\n", abs_time );

        if( running ) {
            if( abs_time == running->get_abs_due_date() and
                std::isgreater( running->get_remaining(), 0  )  ) {
                wasted += running->get_duration() - running->get_remaining();
                running->inc_instance();
                running->set_curr_skip_value(1);
                running->set_state( RED );
                running->update_params();
                running->reset_remaining();
                pending.push_back( std::move( running ) );
                running = nullptr;
//                printf( "MISS\n" );
                bwp_missed++;
            }
        }
        // check blue ready in order to abort tasks
        std::vector<Task *>::iterator it;
        it = blue.begin();
        while( it != blue.end() ) {
            if ((*it)->is_next_instance(abs_time)) {
                (*it)->set_curr_skip_value(1);
                (*it)->set_state(RED);
                (*it)->inc_instance();
                (*it)->update_params();
                pending.push_back(std::move(*it));
                it = blue.erase(it);
//                printf( "SKIP\n" );
                bwp_missed++;
            } else {
                it++;
            }
        }

        it = pending.begin();
        std::sort(pending.begin(), pending.end(),
                  [](const Task *a, const Task *b)
                          -> bool { return a->get_arrival_time() < b->get_arrival_time(); });

        while( it != pending.end() ) {
            if ((*it)->get_arrival_time() > abs_time) {
                break;
            }
            if ((*it)->get_state() == RED) {
                red.push_back(*it);
                it = pending.erase(it);
            } else {
                blue.push_back(*it);
                it = pending.erase(it);
            }
        }

        if( !red.empty() ) {
//            printf( "scheduling red tasks : " );
//            for( auto & element : red ) {
//                printf( "%d ", element->get_id() );
//            }
//            printf( "\n" );

            for( size_t i=0; i<red.size(); i++ ) {
                red[i]->update_priority(abs_time);
            }
            if( running ) {
                if( running->get_state() == BLUE ) {    // suspend blue task
                    blue.push_back( std::move(running) );
                    running = nullptr;
                }
                else {
                    running->update_priority( abs_time );
                }
            }
            // printf( "%d: %f\n", ready[i]->id, ready[i]->priority );
            sched->schedule_next( red, running, abs_time );
        }
        else if (!blue.empty() ) {
            if( running == nullptr  || running->get_state() == BLUE ) {
//                printf( "scheduling blue tasks : " );
//                for( auto & element : blue ) {
//                    printf( "%d", element->get_id() );
//                }
//                printf( "\n" );

                it = blue.begin();
                while( it != blue.end() ) {
                    double priority;
                    update_state( *it, blue );
                    heuristic->execute( &priority );
                    (*it)->set_priority( priority );
                    it++;
                }

                if( running ) {
                    double priority;
                    update_state( running, blue );
                    heuristic->execute( &priority );
                    running->set_priority( priority );
                }
                sched->schedule_next( blue, running, abs_time );
            }
        }

        if(running) {
            fprintf( fd, "%lf\t%d\t%s\n", abs_time, running->get_id(), running->get_state() == RED ? "red" : "blue" );
        }
        else {
            fprintf( fd, "%lf\t null\n", abs_time );
        }
        abs_time += time_slice;

//        printf( "task %d is running, %f remaining\n", running->get_id(), running->get_remaining() );

        if( running ) {
            if( running->isFinished() ) {
//                printf( "task %d is finished!\n", running->get_id() );
                running->update_tardiness( abs_time );
                running->reset_remaining();
                running->inc_instance();
                running->update_rb_params();
                pending.push_back( std::move( running ) );
                running = nullptr;
                bwp_completed++;
            }
            else {
                running->update_remaining();
//				printf("remaining time: %f\n", running->get_remaining() );
            }
        }
    }
    qos = static_cast<double> (bwp_completed) / static_cast<double>( bwp_completed + bwp_missed );
    fclose(fd);
//    printf( "qos: %f\n", get_qos() );
}

void BWP::update_state( Task *current, std::vector<Task *> ready_tasks ) {
    double due_date = static_cast<double>(current->get_abs_due_date());
    double remaining_no = static_cast<double>(ready_tasks.size());
    double spr = 0;
    double sd = 0;
    for( auto & element : ready_tasks ) {
        if( element->get_id() == current->get_id() ) {
            continue;
        }
        spr += static_cast<double>(element->get_duration());
        sd += static_cast<double>(element->get_abs_due_date());
    }
    double pt = static_cast<double>(current->get_duration());
    double sl = static_cast<double>(std::max( current->get_abs_due_date() - current->get_duration() - current->get_time_started(), 0));
    double w = static_cast<double>(current->get_weight());
    double rt = static_cast<double>(current->get_remaining());
    double skip = static_cast<double>(current->get_curr_skip_value());
    double QoS = static_cast<double>(current->get_completed()) / static_cast<double>(current->get_released());

    heuristic->setTerminalValue( "dd", &due_date);
    heuristic->setTerminalValue( "nr", &remaining_no );
    heuristic->setTerminalValue( "spr", &spr );
    heuristic->setTerminalValue( "pt", &pt );
    heuristic->setTerminalValue( "sd", &sd );
    heuristic->setTerminalValue( "sl", &sl );
    heuristic->setTerminalValue( "w", &w );
    heuristic->setTerminalValue( "rt", &rt );
    heuristic->setTerminalValue( "skip", &skip );
    heuristic->setTerminalValue( "QoS", &QoS );
}