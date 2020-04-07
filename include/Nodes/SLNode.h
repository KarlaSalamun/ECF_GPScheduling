//
// Created by karla on 06. 03. 2020..
//

#include "../TaskEvalOp.h"

#ifndef GP_SCHEDULING_SLNODE_H
#define GP_SCHEDULING_SLNODE_H

#include "../TaskEvalOp.h"

#include <cstdio>

class SLNode : public Tree::Primitives::Primitive {
    public:
        SLNode() {
            nArguments_ = 0;
            name_ = "SL";
        }

        void execute(void* evalOp, Tree::Tree& ) {
            // get pointer to simulator from the context
            TaskEvalOp* task = (TaskEvalOp*) state_->getContext()->environment;
            task->SLNode( evalOp );
        }
};

#endif //GP_SCHEDULING_SLNODE_H
