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

int main(int argc, char **argv)
{
	StateP state (new State);

	// crate evaluation operator
	state->setEvalOp(new TaskEvalOp( 12 ) );
	
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
	if(!state->initialize(argc, argv))
		return 1;
	state->run();

	// after the evolution: show best evolved ant's behaviour on learning trails
	/*
	std::vector<IndividualP> hof = state->getHoF()->getBest();
	IndividualP ind = hof[0];
	std::cout << ind->toString();
	std::cout << "\nBest ant's performance on learning traIndividualPil(s):" << std::endl;

	// optional: show movements step by step (interactive)
	//AntEvalOp::step = 1;

	state->getEvalOp()->evaluate(ind);

	// also, simulate best evolved ant on a (different) test trail!
	std::cout << "\nBest ant's performance on test trail(s):" << std::endl;
	AntEvalOp* evalOp = new AntEvalOp;

	// substitute test trails for learning trails (defined in config file):
	state->getRegistry()->modifyEntry("learning_trails", state->getRegistry()->getEntry("test_trails"));
	evalOp->initialize(state);
	evalOp->evaluate(ind);


	// optional: write best individual to 'best.txt'
//	ofstream best("./best.txt");
//	best << ind->toString();
//	best.close();

    // optional: read individual from 'best.txt' (for subsequent simulation)
//	XMLNode xInd = XMLNode::parseFile("./best.txt", "Individual");
//	IndividualP ind = (IndividualP) new Individual(state);
//	ind->read(xInd); 
*/
	return 0;
}
