//
// Created by karla on 06. 03. 2020..
//

#ifndef GP_SCHEDULING_WNODE_H
#define GP_SCHEDULING_WNODE_H

class WNode : public Tree::Primitives::Primitive {
    public:
        WNode() {
            nArguments_ = 0;
            name_ = "W";
        }

        void execute(void* evalOp, Tree::Tree& tree) {
            // get pointer to simulator from the context
            TaskEvalOp* task = (TaskEvalOp*) state_->getContext()->environment;
            task->WNode( evalOp );
        }
};



#endif //GP_SCHEDULING_WNODE_H
