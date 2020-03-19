//
// Created by karla on 06. 03. 2020..
//

#ifndef GP_SCHEDULING_PTNODE_H
#define GP_SCHEDULING_PTNODE_H

#include "../TaskEvalOp.h"

class ptNode : public Tree::Primitives::Primitive {
    public:
        ptNode() {
            nArguments_ = 0;
            name_ = "pt";
        }

        void execute(void* evalOp, Tree::Tree& tree) {
            // get pointer to simulator from the context
            TaskEvalOp* task = (TaskEvalOp*) state_->getContext()->environment;
            task->ptNode( evalOp );
        }
};



#endif //GP_SCHEDULING_PTNODE_H
