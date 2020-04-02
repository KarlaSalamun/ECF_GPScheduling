//
// Created by karla on 06. 03. 2020..
//

#ifndef GP_SCHEDULING_POSNODE_H
#define GP_SCHEDULING_POSNODE_H

#include "../TaskEvalOp.h"

class PosNode : public Tree::Primitives::Primitive {
    public:
        PosNode() {
            nArguments_ = 1;
            name_ = "Pos";
        }

        void execute(void* evalOp, Tree::Tree& tree) {
            struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(evalOp);

            getNextArgument(ctx_, tree);
            ctx_->task->priority = std::max( ctx_->task->priority, 0. );
            // assert(ctx_->task->priority > -1000 && ctx_->task->priority < 10000);
        }
};


#endif //GP_SCHEDULING_POSNODE_H
