#include <cmath>
#include <ecf/ECF.h>
#include "TaskEvalOp.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>
using namespace std;


struct {
    bool operator()(Task *a, Task *b) const
    {
        return a->priority < b->priority;
    }
} customLess;


// TODO staviti u ctx i time

/**
 * \brief Initialize the simulator, read environments from input file.
 */
bool TaskEvalOp::initialize(StateP state)
{
	state_ = state;
	// set the environment pointer to this object (so Tree elements can access the simulator)
	state->getContext()->environment = this;

    test_set.assign( taskNo, 0 );

    if( use_predef_tasks ) {
        FILE *fd = fopen( "task_set.txt", "r+" );
        if( periodic ) {
        	for( int i=0; i<test_set.size(); i++ ) {
        		Task_p *curr_task = new Task_p();
        		curr_task->time_started = 0;
        		fscanf( fd, "%d", &curr_task->id );
            	fscanf( fd, "%lf", &curr_task->phase );
            	fscanf( fd, "%lf", &curr_task->period );
            	fscanf( fd, "%lf", &curr_task->duration );
            	fscanf( fd, "%lf", &curr_task->due_date );
            	test_set[i]  = curr_task;
        	}
        }
        // TODO napisati funkciju koja prima deskriptor i ucitava paramse
        // overridati u inheritanom razredu!
        else {
	        for( int i=0; i<test_set.size(); i++ ) {
	            test_set[i] = std::move( new Task() );
	            test_set[i]->time_started = 0;
	            fscanf( fd, "%lf", &test_set[i]->due_date );
	            fscanf( fd, "%lf", &test_set[i]->weight );
	            fscanf( fd, "%d", &test_set[i]->duration );
	        }
	    }
        return true;
    }

    // TODO: generiranje periodickih taskova 

	double sum_1 = 0;
    double sum_2 = 0;

    for( int i=0; i<test_set.size(); i++ ) {
        test_set[i] = std::move( new Task() );
        test_set[i]->time_started = 0;
        test_set[i]->duration = 1 + rand() % 100; // integer value from 1 to 100
        test_set[i]->weight = static_cast<double>( 1 + rand() % 100 ) / 100.; // 0.01 to 1 with 0.01 increment
        sum_1 += test_set[i]->duration * ( 1 - dd_tightness - dd_range/2. );
        sum_2 += test_set[i]->duration * ( 1 - dd_tightness + dd_range/2. );
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis( sum_1, sum_2 );

    for( int i=0; i<test_set.size(); i++ ) {
//        test_tasks[i]->due_date = sum_1 + static_cast<double>( rand() / static_cast<double>(RAND_MAX) ) * ( sum_2 - sum_1 );
        test_set[i]->due_date = dis(gen);
    }
	return true;
}

/**
 * \brief	Evaluation function, simulates ant movement and counts the eaten food.
 */
FitnessP TaskEvalOp::evaluate(IndividualP individual)
{
    double twt = 0;
    double time = 0;

    std::copy( test_set.begin(), test_set.end(), std::back_inserter( tctx.pending ) );

    // get tree from the individual
    Tree::Tree* tree = (Tree::Tree*) individual->getGenotype().get();
    FitnessP fitness (new FitnessMin);

    if( periodic ) {
        PeriodicSimulator *simulator = new PeriodicSimulator( 0.1, 1000, tree );
        simulator->set_test_tasks(reinterpret_cast<const std::vector<Task_p *> &>( test_set ) );

        simulator->reset_params();

        simulator->total_tardiness = 0;
        simulator->missed = 0;
        simulator->run();

        fitness->setValue( simulator->total_tardiness );
    }

    else {
        while( tctx.pending.size() > 0 ) {
            for( int i=0; i<tctx.pending.size(); i++ ) {
                tctx.task = tctx.pending[i];
                tree->execute( &tctx );
            }

            std::sort( tctx.pending.begin(), tctx.pending.end(), customLess );

            tctx.pending[0]->time_started = time;
            twt += tctx.pending[0]->compute_tardiness() * tctx.pending[0]->weight;

            time+=tctx.pending[0]->duration;

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
    assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);


	ctx_->task->priority = ctx_->task->due_date;

    // printf("%lf\n", ctx_->task->priority);
    assert(ctx_->task->priority > 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::NrNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
    assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);


	ctx_->task->priority = ctx_->pending.size();	

    // printf("%lf\n", ctx_->task->priority);
    assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::ptNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
    assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);

	ctx_->task->priority = ctx_->task->duration;

    // printf("%lf\n", ctx_->task->priority);
    assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::SDNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    assert( ctx_->task->duration >= 0 && ctx_->task->duration < reinterpret_cast<Task_p *>(ctx_->task)->period);
    double sum = 0;

	for( auto & element : ctx_->pending ) {
	    sum+=element->due_date;
	}

	for( auto & element : ctx_->processed ) {
	    sum+=element->due_date;
	}
	ctx_->task->priority = sum;

    // printf("%lf\n", ctx_->task->priority);
    assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::SLNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
	ctx_->task->priority = std::max( ctx_->task->due_date - ctx_->task->duration - ctx_->task->time_started, 0. );

    // printf("%lf\n", ctx_->task->priority);
    assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::SPrNode( void *ctx )
{
	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);

    double sum = 0;
    for( auto & element : ctx_->pending ) {
        sum += element->duration;
    }

    ctx_->task->priority = sum;

    // printf("%lf\n", ctx_->task->priority);
    assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}

void TaskEvalOp::WNode( void *ctx )
{

	struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(ctx);
    // assert( ctx_->task->duration >= 0 && ctx_->task->duration < ctx_->task->period);
	ctx_->task->priority = ctx_->task->weight;

    // printf("%lf\n", ctx_->task->priority);
    assert(ctx_->task->priority >= 0 && ctx_->task->priority < 1000);
}
