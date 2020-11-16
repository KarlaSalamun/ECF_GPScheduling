#include <ecf/ECF.h>
#include "TaskEvalOp.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include "Nodes/AddNode.h"
#include "Nodes/ddNode.h"
#include "Nodes/DivNode.h"
#include "Nodes/MulNode.h"
#include "Nodes/NrNode.h"
#include "Nodes/PosNode.h"
#include "Nodes/ptNode.h"
#include "Nodes/SDNode.h"
#include "Nodes/SLNode.h"
#include "Nodes/SPrNode.h"
#include "Nodes/SubNode.h"
#include "Nodes/WNode.h"
#include "UunifastCreator.h"
#include "Simulator.tpp"
#include "RTO.h"
#include "RLP.h"
#include "EDL.h"

StateP initialize_state();
void generate_csv(std::vector<double> results, std::vector<double> utils, std::string filename );
void test_utils_qos( Tree::Tree * heuristic );
void pareto_test( Tree::Tree * heuristic );
void test_schedule( Tree::Tree * heuristic );

int main(int argc, char **argv)
{

	IndividualP ind;
	
	StateP state = initialize_state();
	if(!state->initialize(2, argv))
		return 1;

	state->run();

	std::vector<IndividualP> hof = state->getHoF()->getBest();
	ind = hof[0];

	FitnessP fitness (new FitnessMin);
	fitness = state->getEvalOp()->evaluate(ind);
	
	std::vector<double> utils;
	std::vector<double> results;

	// ofstream best("../../test_outputs/best.txt");
	// best << ind->toString();
	// best.close();

//	test_schedule( (Tree::Tree*) ind->getGenotype().get() );
    test_utils_qos( (Tree::Tree*) ind->getGenotype().get() );

	return 0;
}

StateP initialize_state() {
	StateP state (new State);

	state->setEvalOp(new TaskEvalOp( 4, true, true, "./../test_inputs/test_1.txt" ) );
	
	TreeP tree (new Tree::Tree);
	
	Tree::PrimitiveP add (new AddNode);
	add->setName( "add" );
	tree->addFunction(add);
	Tree::PrimitiveP div (new DivNode);
	div->setName( "div" );
	tree->addFunction(div);
	Tree::PrimitiveP mul (new MulNode);
	mul->setName( "mul" );
	tree->addFunction(mul);
	Tree::PrimitiveP sub (new SubNode);
	sub->setName( "sub" );
	tree->addFunction(sub);
	Tree::PrimitiveP dd (new ddNode);
	dd->setName( "dd" );
	tree->addTerminal(dd);
	Tree::PrimitiveP nr (new NrNode);
	nr->setName( "nr" );
	tree->addTerminal(nr);
	Tree::PrimitiveP pos (new PosNode);
	pos->setName( "positive" );
	tree->addFunction(pos);
	Tree::PrimitiveP pt (new ptNode);
	pt->setName( "pt" );
	tree->addTerminal(pt);
	Tree::PrimitiveP sd (new SDNode);
	sd->setName( "sd" );
	tree->addTerminal(sd);
	Tree::PrimitiveP sl (new SLNode);
	sl->setName( "sl" );
	tree->addTerminal(sl);
	Tree::PrimitiveP spr (new SPrNode);
	spr->setName( "spr" );
	tree->addTerminal(spr);
	Tree::PrimitiveP w (new WNode);
	w->setName( "w" );
	tree->addTerminal(w);
	state->addGenotype(tree);

	return state;
}

void generate_csv(std::vector<double> results, std::vector<double> utils, std::string filename )
{
    std::string tmp = "./../test_outputs/";
    FILE *fd = fopen( (tmp + filename).c_str(), "w+" );
    for( size_t i=0; i<results.size(); i++ ) {
        fprintf( fd, "%lf,%lf\n", utils[i], results[i] );
    }
    fclose( fd );
}

void test_utils_qos( Tree::Tree * heuristic )
{
    UunifastCreator *taskc = new UunifastCreator( 3, "./../test_inputs/test_1.txt", true, 20, 4, 2, 1 );
    Scheduler *sched = new Scheduler();
//    Simulator<Tree::Tree *> *sim = new Simulator<Tree::Tree *>( 1, taskc->get_hyperperiod(), taskc, sched, true, false );

    RTO *rto = new RTO( taskc, 5, sched, 72, false );
    EDL *edl = new EDL( rto );
    RLP *rlp = new RLP( edl, 1 );

    std::vector<double> utils;
    std::vector<double> results;

    std::vector<Task *> test_tasks;

    for( size_t i=0; i<=14; i++ ) {
        utils.push_back( 0.90 + i * 0.05 );
    }

    std::vector<double> mean_qos;
    std::vector<double> actual_utils;
    std::vector<double> low_qos;

    for( size_t i=0; i<utils.size(); i++ ) {
        taskc->set_overload(utils[i]);
        taskc->set_task_number(6);
        for (size_t j = 0; j < 100; j++) {
            do {
                taskc->create_test_set(test_tasks);
                taskc->compute_hyperperiod( test_tasks );
            } while( taskc->get_hyperperiod() > 10000 );
            double tmp_util = 0;
            for( auto & element : test_tasks ) {
                tmp_util += static_cast<double>( element->get_duration() ) / static_cast<double>( element->get_period() ) ;
            }
            actual_utils.push_back( tmp_util );
            rto->set_pending( test_tasks );
            rto->compute_eq_utilization();
            rto->set_finish_time( taskc->get_hyperperiod() );
            edl->compute_static( test_tasks );
            rlp->set_waiting( test_tasks );
            rlp->set_finish_time( taskc->get_hyperperiod() );
            rlp->set_heuristic( heuristic );
            rlp->simulate();
            mean_qos.push_back( rlp->get_qos());
        }
    }

    generate_csv( mean_qos, actual_utils, "rlp_modified.csv" );
}

void pareto_test( Tree::Tree * heuristic )
{
    UunifastCreator *taskc = new UunifastCreator( 3, "./../test_inputs/test_1.txt", true, 20, 4, 2, 1 );
    Scheduler *sched = new Scheduler();
    Simulator<Tree::Tree *> *sim = new Simulator<Tree::Tree *>( 1, taskc->get_hyperperiod(), taskc, sched, true, false );

    sim->set_heuristic( heuristic );
    std::vector<double> utils;
    std::vector<double> results;

    std::vector<Task *> test_tasks;

    for( size_t i=0; i<=14; i++ ) {
        utils.push_back( 0.90 + i * 0.05 );
    }

    std::vector<double> gini;
    std::vector<double> skip;

    for( size_t i=0; i<utils.size(); i++ ) {
        taskc->set_overload(utils[i]);
        taskc->set_task_number(6);
//        double sum = 0;
        for (size_t j = 0; j < 100; j++) {
            do {
                taskc->create_test_set(test_tasks);
                taskc->compute_hyperperiod( test_tasks );
            } while( taskc->get_hyperperiod() > 10000 );
            double tmp_util = 0;
            for( auto & element : test_tasks ) {
                tmp_util += static_cast<double>( element->get_duration() ) / static_cast<double>( element->get_period() ) ;
            }
            if( tmp_util >= 0.9 && tmp_util <= 1.60 ) {
                sim->set_pending(test_tasks);
                sim->set_finish_time(taskc->get_hyperperiod());
                sim->run();
                sim->compute_mean_skip_factor();
                skip.push_back( sim->compute_skip_fitness() );
                gini.push_back( sim->compute_gini_coeff() );
            }
        }
    }
    generate_csv( gini, skip, "pareto_ecf.csv" );
}

void test_schedule( Tree::Tree * heuristic ) {

	std::vector<Task *> test_tasks;

	UunifastCreator *taskc = new UunifastCreator( 3, "./../test_inputs/test_1.txt", true, 20, 4, 2, 1 );
    Scheduler *sched = new Scheduler();
    Simulator<Tree::Tree *> *sim = new Simulator<Tree::Tree *>( 1, taskc->get_hyperperiod(), taskc, sched, true, false );

    taskc->load_tasks(test_tasks);
    sim->set_display();
    sim->set_heuristic(heuristic);
    sim->set_pending(test_tasks);
    taskc->compute_hyperperiod( test_tasks );
    sim->set_finish_time(taskc->get_hyperperiod());
    sim->run();

}
