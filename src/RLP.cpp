#include <cassert>
#include "RLP.h"

static int compare_factors(const void *m1, const void *m2)
{
    if (*(int*)m1 > *(int*)m2) {
        return -1;
    }

    else if( (*(int*)m1 < *(int*)m2) ) {
        return 1;
    }

    else {
        return  0;
    }
}

void RLP::simulate()
{
//    edl->compute_EDL_deadline_vector();
//    edl->set_EDL_idle_time_vector();
    abs_time = 0;
    completed = 0;
    wasted_time = 0;
    missed_tasks = 0;
    blue_ready.clear();
    red_ready.clear();
    running = nullptr;

    assert(!waiting.empty());
//    std::string prefix = "./../test_outputs/";
//    FILE *fd = fopen( (prefix + "rlp_schedule1.txt").c_str(), "w+" );


    if( display_sched ) {

        fprintf( fd, "\\begin{figure}[ht]\n" );
        fprintf( fd, "\\begin{RTGrid}[width=10cm]{%lu}{%d}\n", waiting.size(), static_cast<int>(finish_time) );
        for( auto & element : waiting ) {
            fprintf( fd, "\\multido{\\n=0+%d}{%d}{\n", static_cast<int>(element->get_period()), static_cast<int>( finish_time / element->get_period() ) );
            fprintf( fd, "\t\\TaskArrDead{%d}{\\n}{%d}\n", element->get_id() + 1, static_cast<int>(element->get_period()) );
            fprintf( fd, "}\n" );
        }
    }

    for( auto & element : waiting ) {
        element->set_state( RED );      // first instance is always red
        element->set_arrival_time( 0 );     // all instances have release time = 0
        element->initialize_task();
        element->set_skip_factor(2);
        element->set_abs_dd();
//        fprintf( fd, "%lf %lf\n", element->get_period(), element->get_duration() );
    }

//    edl->compute_schedule( waiting, nullptr, 0 );
    edl->compute_static( waiting );
//    edl->compute_dynamic( 0, waiting, nullptr );

    while( abs_time < finish_time ) {
        algorithm(abs_time);
        abs_time += time_slice;
    }

    if( display_sched ) {
        fprintf( fd, "{%d}\n", static_cast<int>(abs_time) );
        fprintf( fd, "\\end{RTGrid}\n" );
        fprintf( fd, "\\end{figure}\n" );
        fclose(fd);
    }

    for( auto & element : waiting ) {
//		printf( "%d tardiness: %f\n", element->get_id(), element->get_tardiness() );
        element->skip_factors.push_back( element->get_curr_skip_value() );
    }
    for( auto & element : blue_ready ) {
//		printf( "%d tardiness: %f\n", element->get_id(), element->get_tardiness() );
        if( element->get_arrival_time() < abs_time ) {
//            missed_tasks++;
            element->skip_factors.push_back( element->get_curr_skip_value() );
        }
    }
    for( auto & element : red_ready ) {
//		printf( "%d tardiness: %f\n", element->get_id(), element->get_tardiness() );
        if( element->get_arrival_time() < abs_time ) {
//            missed_tasks++;
            element->skip_factors.push_back( element->get_curr_skip_value() );
        }
    }
    if( running ) {
//		printf( "%d tardiness: %f\n", running->get_id(), running->get_tardiness() );
        if( std::isgreater( running->get_remaining(), 0  )  ) {
            running->skip_factors.push_back( running->get_curr_skip_value() );
//            missed_tasks++;
        }
    }


    qos = static_cast<double>( completed ) / static_cast<double>( completed + missed_tasks );
    if( running ) {
        waiting.push_back( std::move( running ) );
    }
//    assert( waiting.size() + red_ready.size() + blue_ready.size() == 6 );
    if( !red_ready.empty() ) {
        std::copy( red_ready.begin(), red_ready.end(), std::back_inserter( waiting ) );
    }
    if( !blue_ready.empty() ) {
        std::copy( blue_ready.begin(), blue_ready.end(), std::back_inserter( waiting ) );
    }

    assert( !waiting.empty() );

//    for( auto & element : waiting ) {
//        if( element->get_curr_skip_value() >= 2 )
//            element->skip_factors.push_back( element->get_curr_skip_value() );
//    }
    compute_qos();
}

void RLP::algorithm( double current_time )
{
//    printf("current time: %f\n", current_time);
    bool running_changed = false;
    bool taskStarted = false;

    // check if running task is going to miss deadline
    if( running ) {
        if( current_time == running->get_abs_due_date() and
            std::isgreater( running->get_remaining()-time_slice, 0  )  ) {
            wasted_time += running->get_duration() - running->get_remaining();
            assert( wasted_time <= finish_time );
            running->inc_instance();
            running->skip_factors.push_back( running->get_curr_skip_value() );
            running->set_curr_skip_value(1);
            running->set_state( RED );
            running->set_arrival_time();
            running->reset_remaining();
            running->reset_skip_value();
            if( display_sched ) {
                fprintf( fd, "{%d}\n", static_cast<int>(abs_time) );
            }
            running->update_params();
            waiting.push_back( running );
            running = nullptr;
            missed_tasks++;
        }
    }
    // check blue ready in order to abort tasks
    std::vector<Task *>::iterator it;
    it = blue_ready.begin();
    while( it != blue_ready.end() ) {
        if ((*it)->is_next_instance(current_time)) {
            missed_tasks++;
            (*it)->skip_factors.push_back( (*it)->get_curr_skip_value() );
            (*it)->set_curr_skip_value(1);
            (*it)->set_state(RED);
            (*it)->inc_instance();
            (*it)->reset_remaining();
            (*it)->set_arrival_time();
            (*it)->update_params();
            waiting.push_back(std::move(*it));
            it = blue_ready.erase(it);
        } else {
            it++;
        }
    }

    it = red_ready.begin();
    while( it != red_ready.end() ) {
        if ((*it)->is_next_instance(current_time)) {
            missed_tasks++;
            (*it)->set_curr_skip_value(1);
            (*it)->set_state(RED);
            (*it)->inc_instance();
            (*it)->reset_remaining();
            (*it)->set_arrival_time();
            (*it)->update_params();
            waiting.push_back(std::move(*it));
            it = red_ready.erase(it);
        } else {
            it++;
        }
    }

    blue_appeared = false;

    std::sort(waiting.begin(), waiting.end(),
              [](const Task *a, const Task *b)
                      -> bool { return a->get_arrival_time() < b->get_arrival_time(); });
    it = waiting.begin();
    while( it != waiting.end() ) {
        if ((*it)->get_arrival_time() > current_time) {
            break;
        }
        if ((*it)->get_state() == RED) {
            red_ready.push_back(*it);
            it = waiting.erase(it);
        } else {
            blue_ready.push_back(*it);
            it = waiting.erase(it);
            if( blue_ready.size() == 1 )
                blue_appeared = true;
        }
    }

    // idle time -> suspend red task
//    if( !blue_ready.empty() and availability ) {
//        it = red_ready.begin();
//        while( it != red_ready.end() ) {
//            waiting.push_back( *it );
//            red_ready.erase( it );
//        }
//    }
    if( !red_ready.empty() ) {
        std::sort(red_ready.begin(), red_ready.end(),
                  [](const Task *a, const Task *b)
                          -> bool { return a->get_abs_due_date() < b->get_abs_due_date(); });
    }
    if( !blue_ready.empty() ) {
        it = blue_ready.begin();
        while( it != blue_ready.end() ) {
            double priority;
            update_state( *it, blue_ready );
            heuristic->execute( &priority );
            (*it)->set_priority( priority );
            it++;
        }
        std::sort(blue_ready.begin(), blue_ready.end(),
                  [](const Task *a, const Task *b)
                          -> bool { return a->get_priority() < b->get_priority(); });
    }

    bool availability = false;

    if( running ) {
        if( running->isFinished() ) {
            if( display_sched ) {
                fprintf( fd, "{%d}\n", static_cast<int>(abs_time) );
            }
            completed++;
            if( running->get_state() == BLUE && !blue_ready.empty() ) {
                if( !red_ready.empty() ) {
                    edl->compute_dynamic( current_time, red_ready, nullptr);
                }
                else {
                    availability = true;
                }
            }
            running->reset_remaining();
            running->inc_instance();
            running->update_rb_params();
            running->inc_skip_value();
//            printf( "time : %f\ttask %d is finished\n", current_time, running->get_id() );
            if( running->isReady( current_time ) ) {
                if( running->get_state() == BLUE ) {
                    blue_ready.push_back( running );
                    it = blue_ready.begin();
                    while( it != blue_ready.end() ) {
                        double priority;
                        update_state( *it, blue_ready );
                        heuristic->execute( &priority );
                        (*it)->set_priority( priority );
                        it++;
                    }
                    std::sort(blue_ready.begin(), blue_ready.end(),
                              [](const Task *a, const Task *b)
                                      -> bool { return a->get_priority() < b->get_priority(); });
                }
                else {
                    red_ready.push_back( running );
                    std::sort(red_ready.begin(), red_ready.end(),
                              [](const Task *a, const Task *b)
                                      -> bool { return a->get_abs_due_date() < b->get_abs_due_date(); });
                }
            }
            else
                waiting.push_back(running);
            running = nullptr;
        }
    }

    if( blue_appeared ) {
        if( running ) {
            if( running->get_state() == RED ) {
                edl->compute_dynamic(current_time, red_ready, running);
            }
        }
        else {
            if( !red_ready.empty() ) {
                edl->compute_dynamic( current_time, red_ready, nullptr);
            }
        }
    }

    if( !running ) {
        if( blue_ready.empty() ) {
            if( !red_ready.empty() ) {
                running = std::move( red_ready[0] );
                red_ready.erase( red_ready.begin() );
//                printf( "time : %f\tred instance of task %d is running, %f remaining\n", current_time, running->get_id(), running->get_remaining() );
            }
            else {
                running = nullptr;
            }
        }
        else {
            availability = edl->compute_availability( current_time );
            if( red_ready.empty() ) {
                availability = true;
            }
            if( availability ) {
                if( blue_ready.size() > 1 ) {
                    break_dd_tie( blue_ready );
                }
                running = std::move( blue_ready[0] );

                blue_ready.erase( blue_ready.begin() );
//                printf( "time : %f\tblue instance of task %d is running, %f remaining\n", current_time, running->get_id(), running->get_remaining() );
            }
            else if (red_ready.size()) {   // schedule red by EDL
                assert(red_ready.begin() != red_ready.end());
                running = red_ready[0];
                red_ready.erase( red_ready.begin() );
//                printf( "time : %f\tred instance of task %d is running, %f remaining\n", current_time, running->get_id(), running->get_remaining() );
            }
        }
        if(running) {
            taskStarted = true;
        }
    }
    else {
        edl->compute_availability(current_time);
        if (running->get_state() == RED and availability and !blue_ready.empty()) {
//            running->update_remaining();
            running_changed = true;
            red_ready.push_back(running);
            running = std::move(blue_ready[0]);
            blue_ready.erase(blue_ready.begin());
//            printf( "time : %f\tblue instance of task %d is running, %f remaining\n", current_time, running->get_id(), running->get_remaining() );

        } else {
            if (running->get_state() == BLUE) {
                if (!red_ready.empty()) {
                    edl->compute_availability(current_time);
                    if (!availability) {
                        running->update_remaining();
                        running_changed = true;
                        blue_ready.push_back(running);
                        running = std::move(red_ready[0]);
                        assert(red_ready.size());
                        red_ready.erase(red_ready.begin());
//                    printf( "time : %f\tred instance of task %d is running, %f remaining\n", current_time, running->get_id(), running->get_remaining() );

                    }
                }
                if (blue_ready.front()->get_priority() < running->get_priority() && !running_changed) {
                    running_changed = true;
                    running->update_remaining();
                    blue_ready.push_back(running);
                    running = blue_ready[0];
                    blue_ready.erase( blue_ready.begin() );
                }
            }
        }
        if (!running_changed) {
            running->update_remaining();
        }
    }
    if(running) {
        if( display_sched ) {
            if( running_changed ) {
                fprintf( fd, "{%d}\n", static_cast<int>(abs_time) );
                fprintf( fd, "\\TaskExecution[color=%s]{%d}{%d}", (running->get_state() == RED ? "red" : "blue"), running->get_id() + 1, static_cast<int>(abs_time) );
            }
            if( taskStarted ) {
                fprintf( fd, "\\TaskExecution[color=%s]{%d}{%d}", (running->get_state() == RED ? "red" : "blue"), running->get_id() + 1, static_cast<int>(abs_time) );
            }
        }
    }

//    printf( "task %d is running, %f remaining\n", running->get_id(), running->get_remaining() );
}

// if multiple tasks in ready list have the same abs due date, schedule the one with earliest release time
void RLP::break_dd_tie( std::vector<Task *> &tasks )
{
    for( size_t i=1; i<tasks.size(); i++ ) {
        if( fabs( tasks[i]->get_abs_due_date() - tasks[0]->get_abs_due_date() ) < 0.001 ) {
            if( std::isless( tasks[i]->get_arrival_time(), tasks[0]->get_arrival_time() ) ) {
                std::iter_swap( tasks.begin(), tasks.begin() + i );
            }
        }
    }
}

void RLP::compute_qos()
{
    this->qos = static_cast<double>(completed) / static_cast<double>( completed + missed_tasks );
}

double RLP::compute_mean_skip_factor()
{
    double sum = 0;
    int tasks = 0;
    for( auto & element : waiting ) {
        qsort( element->skip_factors.data(), element->skip_factors.size(), sizeof(int), compare_factors );
        sum += element->get_weight() * element->compute_mean_skip_factor();
        tasks++;
    }
    return sum / static_cast<double>( tasks );
}

double RLP::compute_gini_coeff()
{
    double sum = 0;
    for( size_t  i=0; i<waiting.size(); i++) {
        for( size_t  j=0; j<waiting.size(); j++) {
            sum += fabs( waiting[i]->compute_mean_skip_factor() - waiting[j]->compute_mean_skip_factor() );
        }
    }
    double mean_skip_factor = compute_mean_skip_factor();
    sum /= ( 2 * pow(static_cast<double>(waiting.size()), 2 ) * mean_skip_factor );
    return sum;
}

double RLP::compute_skip_fitness()
{
    double sum = 0;
    int tasks = 0;
    for( auto & element : waiting ) {
        qsort( element->skip_factors.data(), element->skip_factors.size(), sizeof(int), compare_factors );
        sum += element->get_weight() * element->compute_mean_skip_factor() / ( element->skip_factors[0] * element->skip_factors.size() );
        tasks++;
    }
    assert( sum / static_cast<double>( tasks ) <= 1 );
    return sum / static_cast<double>( tasks );
}

void RLP::update_state( Task *current, std::vector<Task *> ready_tasks ) {
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