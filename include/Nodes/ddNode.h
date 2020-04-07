//
// Created by karla on 06. 03. 2020..
//

#ifndef GP_SCHEDULING_DDNODE_H
#define GP_SCHEDULING_DDNODE_H

#include "../TaskEvalOp.h"

class ddNode : public Tree::Primitives::Primitive {
    public:
        ddNode() {
            nArguments_ = 0;
            name_ = "dd";
        }

        void execute(void* evalOp, Tree::Tree& ) {
            // get pointer to simulator from the context
            TaskEvalOp* task = (TaskEvalOp*) state_->getContext()->environment;
            task->ddNode( evalOp );
        }
};

#endif //GP_SCHEDULING_DDNODE_H
