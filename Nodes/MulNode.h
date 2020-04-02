//
// Created by karla on 06. 03. 2020..
//

#ifndef GP_SCHEDULING_MULNODE_H
#define GP_SCHEDULING_MULNODE_H

#include "../TaskEvalOp.h"

class MulNode : public Tree::Primitives::Primitive {
    public:
        MulNode() {
            nArguments_ = 2;
            name_ = "Mul";
        }

        void execute(void* evalOp, Tree::Tree& tree) {
            struct task_ctx *ctx_ = reinterpret_cast<struct task_ctx *>(evalOp);
            double tmp_priority;

            getNextArgument(ctx_, tree);
            tmp_priority = ctx_->task->priority;
             
            getNextArgument(ctx_, tree);
            // printf("%lf * %lf\n", ctx_->task->priority, tmp_priority);
            ctx_->task->priority *= tmp_priority;
            // assert(ctx_->task->priority >= 0 && ctx_->task->priority < 10000);
        }
    };


#endif //GP_SCHEDULING_MULNODE_H
