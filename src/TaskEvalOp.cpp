#include <cmath>
#include <ecf/ECF.h>
#include "TaskEvalOp.h"
#include "Simulator.h"
#include "Simulator.tpp"
#include "UunifastCreator.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
#include "RTO.h"
#include "EDL.h"
#include "RLP.h"
using namespace std;

// TODO staviti u ctx i time

/**
 * \brief Initialize the simulator, read environments from input file.
 */
bool TaskEvalOp::initialize(StateP state)
{
    // Scheduler *sched = new Scheduler();
    // UunifastCreator *tc = new UunifastCreator( task_number, "./uunifast_tasks.txt", 10, 1, 1 );
	state_ = state;
	// set the environment pointer to this object (so Tree elements can access the simulator)
	state->getContext()->environment = this;

    // if( use_predef_tasks ) {
    //     tc->load_tasks( test_set );
    // }
    // else {
    //     tc->create_test_set( test_set );
    // }
	return true;
}

/**
 * \brief	Evaluation function, simulates ant movement and counts the eaten food.
 */
FitnessP TaskEvalOp::evaluate(IndividualP individual)
{
    double twt = 0;
    double abs_time = 0;

    Scheduler *sched = new Scheduler();
    UunifastCreator *tc = new UunifastCreator( 3, filename, true, 10, 1, 1, 1 );

    RTO *rto = new RTO( tc, 5, sched, 72, false );
    EDL *edl = new EDL( rto );
    RLP *rlp = new RLP( edl, 1 );

    std::copy( test_set.begin(), test_set.end(), std::back_inserter( tctx.pending ) );

    // get tree from the individual
    Tree::Tree* tree = (Tree::Tree*) individual->getGenotype().get();
    MOFitnessP fitness = static_cast<MOFitnessP> (new MOFitness);
    // MOFitness fitness;
    // FitnessP fitness (new FitnessMin);

    if( periodic ) {
        tc->load_tasks( test_tasks );
        tc->compute_hyperperiod( test_tasks );

        Simulator<Tree::Tree*> *simulator = new Simulator<Tree::Tree*>( 1, 10, tc, sched, true, false );
        simulator->set_heuristic( tree );

        std::vector<double> skip;
        std::vector<double> gini;

        std::vector<double> qos;
        std::vector<double> wasted;

        std::vector<double> utils = { 0.90, 1, 1.1, 1.2, 1.3, 1.4 };

         for( size_t i = 0; i<10; i++) {
             for( size_t j=0; j<utils.size(); j++ ) {
                 tc->set_overload( utils[j] );
                 tc->set_task_number( 6 );
                 tc->create_test_set( test_tasks );
                 tc->compute_hyperperiod( test_tasks );

                 simulator->set_pending( test_tasks );
                simulator->set_finish_time( tc->get_hyperperiod() );
                simulator->run();
                simulator->compute_mean_skip_factor();

//                 rto->set_pending( test_tasks );
//                 rto->compute_eq_utilization();
//                 rto->set_finish_time( tc->get_hyperperiod() );
//                 edl->compute_static( test_tasks );
//                 rlp->set_waiting( test_tasks );
//                 rlp->set_finish_time( tc->get_hyperperiod() );
//                 rlp->set_heuristic( tree );
//                 rlp->simulate();

                 qos.push_back( simulator->get_qos() );
                 wasted.push_back( simulator->get_time_wasted() / simulator->get_finish_time() );
//                    skip.push_back( simulator->compute_skip_fitness() );
//                    gini.push_back( simulator->compute_gini_coeff() );
             }
         }

//        tc->compute_hyperperiod(test_tasks);
//        simulator->set_pending( test_tasks );
//        simulator->set_finish_time( tc->get_hyperperiod() );
//        simulator->run();
//        simulator->compute_mean_skip_factor();
//
//        fitness->push_back( (FitnessP) new FitnessMin );
//        fitness->back()->setValue( - simulator->compute_skip_fitness() );
//        fitness->push_back( (FitnessP) new FitnessMin );
//        fitness->back()->setValue( - simulator->compute_gini_coeff() );

         fitness->push_back( (FitnessP) new FitnessMax );
         fitness->back()->setValue( compute_mean_fitness( qos ) );
         fitness->push_back( (FitnessP) new FitnessMin );
         fitness->back()->setValue( compute_mean_fitness( wasted ) );
    }

    else {
        while( tctx.pending.size() > 0 ) {
            for( size_t i=0; i<tctx.pending.size(); i++ ) {
                tctx.task = tctx.pending[i];
                tree->execute( &tctx );
            }

            std::sort( tctx.pending.begin(), tctx.pending.end(), 
            []( const Task *a, const Task *b ) -> bool { return a->get_priority() < b->get_priority(); });

            tctx.pending[0]->set_time_started( abs_time );
            twt += tctx.pending[0]->compute_tardiness( abs_time ) * tctx.pending[0]->get_weight();

            abs_time+=tctx.pending[0]->get_duration();

            tctx.processed.push_back( tctx.pending[0] );
            tctx.pending.erase( tctx.pending.begin() );
        }
        fitness->setValue(twt);
    }
	return fitness;
}

void TaskEvalOp::ddNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);


	ctx_->task->set_priority( static_cast<double>(ctx_->task->get_abs_due_date() ));

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority > 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::NrNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);


	ctx_->task->set_priority( static_cast<double>(ctx_->pending.size() ));

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::ptNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);

	ctx_->task->set_priority( static_cast<double>(ctx_->task->get_duration() ));

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::SDNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);
    double sum = 0;

	for( auto & element : ctx_->pending ) {
	    sum+=element->get_abs_due_date();
	}

	for( auto & element : ctx_->processed ) {
	    sum+=element->get_abs_due_date();
	}
	ctx_->task->set_priority( sum );

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::SLNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
	ctx_->task->set_priority( static_cast<double>(std::max( ctx_->task->get_abs_due_date() - ctx_->task->get_duration() - ctx_->task->get_time_started(), 0 )));

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::SPrNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);

    double sum = 0;
    for( auto & element : ctx_->pending ) {
        sum += element->get_duration();
    }

    ctx_->task->set_priority( sum );

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::WNode( void *ctx )
{

	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
	ctx_->task->set_priority( ctx_->task->get_weight() );

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::rtNode( void *ctx )
{
    struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);


    ctx_->task->set_priority( static_cast<double>(ctx_->task->get_remaining() ));

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority > 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::skipNode( void *ctx )
{
    struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);


    ctx_->task->set_priority(static_cast<double>( ctx_->task->get_curr_skip_value() ));

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority > 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::QoSNode( void *ctx )
{
    struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);


    ctx_->task->set_priority(static_cast<double>( ctx_->task->get_completed() ) / static_cast<double>( ctx_->task->get_released() ) );

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority > 0 && ctx_->task->priority < 1000);
}

double TaskEvalOp::compute_mean_fitness( std::vector<double> values )
{
    double sum = 0;
    for( size_t i=0; i<values.size(); i++ ) {
        sum += values[i];
        assert( values[i]== values[i] );
    }
    return sum / values.size();
}

