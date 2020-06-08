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

StateP initialize_state();
void generate_csv(std::vector<double> results, std::vector<double> utils, std::string filename );
void test_utils_qos( Tree::Tree * heuristic );

int main(int argc, char **argv)
{

	IndividualP ind;
	// FILE *fd = fopen( "../../test_outputs/ecf_results.csv", "w+" );
	// assert(fd);
	
	StateP state = initialize_state();
	if(!state->initialize(argc, argv))
		return 1;

	state->run();

	std::vector<IndividualP> hof = state->getHoF()->getBest();
	ind = hof[0];

	FitnessP fitness (new FitnessMin);
	fitness = state->getEvalOp()->evaluate(ind);

		// fprintf( fd, "%d,%lf\n", i+1, fitness->getValue() );
	
	std::vector<double> utils;
	std::vector<double> results;

	TaskEvalOp *eval = new TaskEvalOp( 4, true, true, "../../teat_inputs/100.txt" );

	// for( int overload = 90; overload <= 160; overload = overload + 5 ) {
 //        std::string tmp = "../../test_inputs/" + std::to_string( overload ) + ".txt";
 //        // StateP state = initialize_state( tmp );
 //        eval->set_filename( tmp );
 //        fitness = eval->evaluate(ind);

 //        printf( "qos: %lf\n", fitness->getValue());
 //        utils.push_back( overload / 100. );
 //        results.push_back( fitness->getValue() );
 //    }

 //    generate_csv( results, utils, "ecf.csv" );

	// fclose(fd);

	// also, simulate best evolved ant on a (different) test trail!
	// std::cout << "\nBest ant's performance on test trail(s):" << std::endl;
	// AntEvalOp* evalOp = new AntEvalOp;

	// substitute test trails for learning trails (defined in config file):
	// state->getRegistry()->modifyEntry("learning_trails", state->getRegistry()->getEntry("test_trails"));
	// evalOp->initialize(state);
	// evalOp->evaluate(ind);


	// optional: write best individual to 'best.txt'
	ofstream best("../../test_outputs/best.txt");
	best << ind->toString();
	best.close();

	test_utils_qos( (Tree::Tree*) ind->getGenotype().get() );

    // optional: read individual from 'best.txt' (for subsequent simulation)
	// XMLNode xInd = XMLNode::parseFile("./best.txt", "Individual");
	// IndividualP ind1 = (IndividualP) new Individual(state);
	// ind1->read(xInd); 


	return 0;
}

StateP initialize_state() {
	StateP state (new State);

	// crate evaluation operator
	state->setEvalOp(new TaskEvalOp( 4, true, true, "../../test_inputs/120.txt" ) );
	
	// create tree genotype
	TreeP tree (new Tree::Tree);
	
	// create new functions and add them to function set 
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
	// register genotype with our primitives
	state->addGenotype(tree);
	// initialize and start evaluation

	return state;
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
    UunifastCreator *taskc = new UunifastCreator( 3, "./../../test_inputs/test_1.txt", true, 20, 4, 2, 1 );
    Scheduler *sched = new Scheduler();
    Simulator<Tree::Tree *> *sim = new Simulator<Tree::Tree *>( 1, taskc->get_hyperperiod(), taskc, sched, true, false );

    sim->set_heuristic( heuristic );
    std::vector<double> utils;
    std::vector<double> results;

    std::vector<Task *> test_tasks;

    for( size_t i=0; i<=14; i++ ) {
        utils.push_back( 0.90 + i * 0.05 );
    }

    std::vector<double> mean_qos;
    std::vector<double> actual_utils;

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
            actual_utils.push_back( tmp_util );
            sim->set_pending(test_tasks);
            sim->set_finish_time(taskc->get_hyperperiod());
            sim->run();
            mean_qos.push_back( sim->get_qos() );
        }
    }

    generate_csv( mean_qos, actual_utils, "ecf.csv" );
}