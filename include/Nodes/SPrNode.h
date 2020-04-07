//
// Created by karla on 06. 03. 2020..
//

#ifndef GP_SCHEDULING_SPRNODE_H
#define GP_SCHEDULING_SPRNODE_H

#include "../TaskEvalOp.h"

class SPrNode : public Tree::Primitives::Primitive {
    public:
        SPrNode() {
            nArguments_ = 0;
            name_ = "SPr";
        }

        void execute(void* evalOp, Tree::Tree& ) {
            // get pointer to simulator from the context
            TaskEvalOp* task = (TaskEvalOp*) state_->getContext()->environment;
            task->SPrNode( evalOp );
        }
};



#endif //GP_SCHEDULING_SPRNODE_H
