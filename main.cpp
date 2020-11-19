#include <ecf/ECF.h>
#include "TaskEvalOp.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include "UunifastCreator.h"
#include "Simulator.tpp"
#include "RTO.h"
#include "RLP.h"
#include "EDL.h"

void generate_csv(std::vector<double> results, std::vector<double> utils, std::string filename );
void test_utils_qos( Tree::Tree * heuristic );
void pareto_test( Tree::Tree * heuristic );
void test_schedule( Tree::Tree * heuristic );
void test_utils_wCPU( Tree::Tree *heuristic );

int main(int argc, char **argv)
{

	IndividualP ind;

    StateP state = static_cast<StateP> (new State);
    state->setEvalOp(static_cast<EvaluateOpP> (new TaskEvalOp(4, true, true, "./../test_inputs/test_1.txt" )));
	
//	StateP state = initialize_state();
	if(!state->initialize(2, argv))
		return 1;

	state->run();

//	std::vector<IndividualP> hof = state->getHoF()->getBest();
//	ind = hof[0];

    IndividualP best = state->getPopulation()->getLocalDeme()->front();

    double max = (static_pointer_cast<MOFitness> (best->getFitness()))->getValueOfObjective(0);

    std::string tree =  ((Tree::Tree *)best->getGenotype().get())->toString();

    for( IndividualP ind : *state->getPopulation()->getLocalDeme() ) {

        double fitness1 = (static_pointer_cast<MOFitness> (ind->getFitness()))->getValueOfObjective(0);

        if( fitness1 > max ) {
            max = fitness1;
            best = ind;
        }

        cout << (static_pointer_cast<MOFitness> (ind->getFitness()))->getValueOfObjective(0) << "\t" << (static_pointer_cast<MOFitness> (ind->getFitness()))->getValueOfObjective(1);
        cout << "\n";
    }

    cout << tree;

    cout << (static_pointer_cast<MOFitness> (best->getFitness()))->getValueOfObjective(0) << "\t" << (static_pointer_cast<MOFitness> (best->getFitness()))->getValueOfObjective(1);

	// ofstream best("../../test_outputs/best.txt");
	// best << ind->toString();
	// best.close();

    test_utils_qos( (Tree::Tree*) best->getGenotype().get() );
//    test_schedule( (Tree::Tree*) best->getGenotype().get() );
        test_utils_wCPU( (Tree::Tree*) best->getGenotype().get() );
//
	return 0;
}

void generate_csv(std::vector<double> results, std::vector<double> utils, std::string filename )
{
    std::string tmp = "./../../test_outputs/";
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

    for( size_t i=1; i<=14; i++ ) {
        utils.push_back( 0.90 + i * 0.05 );
    }

    std::vector<double> mean_qos;
    std::vector<double> mean_gini;
    std::vector<double> mean_skips;
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
            mean_qos.push_back( rlp->get_qos() );
            mean_skips.push_back( rlp->compute_mean_skip_factor());
            mean_gini.push_back( rlp->compute_gini_coeff() );
        }
    }

    generate_csv( mean_qos, actual_utils, "rlp_modified.csv" );
//    generate_csv( mean_gini, actual_utils, "modif_gini.csv" );
//    generate_csv( mean_skips, actual_utils, "modif_skips.csv" );
}

void test_utils_wCPU( Tree::Tree * heuristic )
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

    for( size_t i=1; i<=14; i++ ) {
        utils.push_back( 0.90 + i * 0.05 );
    }

    std::vector<double> wasted;
    std::vector<double> actual_utils;

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
            wasted.push_back( rlp->get_wasted_time() / rlp->get_finish_time() );
        }
    }

    generate_csv( wasted, actual_utils, "rlp_modified_wCPU.csv" );
//    generate_csv( mean_gini, actual_utils, "modif_gini.csv" );
//    generate_csv( mean_skips, actual_utils, "modif_skips.csv" );
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

    UunifastCreator *taskc = new UunifastCreator( 3, "./../test_inputs/test_1.txt", true, 20, 4, 2, 1 );
    Scheduler *sched = new Scheduler();

    RTO *rto = new RTO( taskc, 5, sched, 72, false );
    EDL *edl = new EDL( rto );
    RLP *rlp = new RLP( edl, 1 );

	std::vector<Task *> test_tasks;

    taskc->load_tasks(test_tasks);
    rto->set_pending( test_tasks );
    rto->compute_eq_utilization();
    rto->set_finish_time( taskc->get_hyperperiod() );
    edl->compute_static( test_tasks );
    rlp->set_waiting( test_tasks );
    rlp->display_sched = true;
    rlp->set_finish_time( taskc->get_hyperperiod() );
    rlp->set_heuristic( heuristic );
    rlp->simulate();
}
