//
// Created by karla on 06. 03. 2020..
//

#ifndef GP_SCHEDULING_SDNODE_H
#define GP_SCHEDULING_SDNODE_H

#include "../TaskEvalOp.h"

class SDNode : public Tree::Primitives::Primitive {
    public:
        SDNode() {
            nArguments_ = 0;
            name_ = "SD";
        }

        void execute(void* evalOp, Tree::Tree& tree) {
            // get pointer to simulator from the context
            TaskEvalOp* task = (TaskEvalOp*) state_->getContext()->environment;
            task->SDNode( evalOp );
        }
};


#endif //GP_SCHEDULING_SDNODE_H
