//
// Created by karla on 06. 03. 2020..
//

#ifndef GP_SCHEDULING_ADDNODE_H
#define GP_SCHEDULING_ADDNODE_H

#include "../TaskEvalOp.h"

class AddNode : public Tree::Primitives::Primitive {
    public:
        AddNode() {
            nArguments_ = 2;
            name_ = "Add";
        }

        void execute(void* evalOp, Tree::Tree& tree) {
            struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(evalOp);
            double tmp_priority;

            getNextArgument(ctx_, tree);
            tmp_priority = ctx_->task->priority;
             
            getNextArgument(ctx_, tree);
            ctx_->task->priority += tmp_priority;
        }
};


#endif //GP_SCHEDULING_ADDNODE_H
