//
// Created by karla on 06. 03. 2020..
//

#ifndef GP_SCHEDULING_NRNODE_H
#define GP_SCHEDULING_NRNODE_H

class NrNode : public Tree::Primitives::Primitive {
    public:
        NrNode() {
            nArguments_ = 0;
            name_ = "Nr";
        }

        void execute(void* evalOp, Tree::Tree& tree) {
            // get pointer to simulator from the context
            TaskEvalOp* task = (TaskEvalOp*) state_->getContext()->environment;
            task->NrNode( evalOp );
        }
};



#endif //GP_SCHEDULING_NRNODE_H
