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
    UunifastCreator *tc = new UunifastCreator( taskNo, "./../../test_inputs/task_set.txt", 10, 1, 1 );

    std::copy( test_set.begin(), test_set.end(), std::back_inserter( tctx.pending ) );

    // get tree from the individual
    Tree::Tree* tree = (Tree::Tree*) individual->getGenotype().get();
    FitnessP fitness (new FitnessMin);

    if( periodic ) {
        Simulator<Tree::Tree *> *simulator = new Simulator<Tree::Tree *>( 0.1, 1000, tc, sched, true );
        simulator->load();
        simulator->set_heuristic( tree );

        simulator->run();

        fitness->setValue( simulator->get_total_tardiness() );
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

            tctx.ready.push_back( tctx.pending[0] );
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


	ctx_->task->set_priority( ctx_->task->get_abs_due_date() );

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority > 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::NrNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);


	ctx_->task->set_priority( ctx_->pending.size() );	

    // printf("%lf\n", ctx_->task->priority);
    // assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::ptNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);

	ctx_->task->set_priority( ctx_->task->get_duration() );

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

	for( auto & element : ctx_->ready ) {
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
	ctx_->task->set_priority( std::max( ctx_->task->get_abs_due_date() - ctx_->task->get_duration() - ctx_->task->get_time_started(), 0. ));

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
