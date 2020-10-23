#ifndef TaskEvalOp_h
#define TaskEvalOp_h

#include <random>

#include "Task.h"
#include "Simulator.h"

class TaskEvalOp : public EvaluateOp
{
protected:
	StateP state_;

public:
	TaskEvalOp( int taskNo, bool use_predef_tasks, bool periodic, std::string filename ) : 
	EvaluateOp() {
		this->taskNo = taskNo;
		this->use_predef_tasks = use_predef_tasks;
		this->periodic = periodic;
		this->filename = filename;
	}
	FitnessP evaluate(IndividualP individual);
	bool initialize(StateP);

	struct task_ctx tctx;
	
    void AddNode( void *ctx );
    void ddNode( void *ctx );
    void DivNode( void *ctx );
    void MulNode( void *ctx );
    void NrNode( void *ctx );
    void PosNode( void *ctx );
    void ptNode( void *ctx );
    void SDNode( void *ctx );
    void SLNode( void *ctx );
    void SPrNode( void *ctx );
    void SubNode( void *ctx );
    void WNode( void *ctx );

    void set_filename( std::string filename ) {
    	this->filename = filename;
    }

private:
    double compute_mean_fitness( std::vector<double> values );

    std::vector<Task *> test_tasks;
    std::vector<Task *> pending_tasks;
    std::vector<Task *> processed_tasks;
	int taskNo;
	bool use_predef_tasks;
	bool periodic;
	double dd_range = 0.6;
    double dd_tightness = 0.6;
    std::vector<Task *> test_set;
    std::string filename;
};

#endif
