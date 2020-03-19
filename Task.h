//
// Created by karla on 06. 03. 2020..
//

#ifndef GP_SCHEDULING_TASK_H
#define GP_SCHEDULING_TASK_H


class Task {
    public:
        int duration;
        double due_date;
        double weight;
        double priority;
        double time_started;
        double compute_tardiness();

        Task( double duration, double weight, double due_date ) :
            duration( duration ), due_date( due_date ), weight( weight ) {}
        Task() {}
        ~Task() {}

        Task& operator = ( Task&& other )
        {
            if( &other == this ) {
                return *this;
            }
            this->priority = other.priority;
            this->due_date = other.due_date;
            this->weight = other.weight;
            this->duration = other.duration;
            this->time_started = other.time_started;
            return *this;
        }

        Task( const Task& obj )
        {
            this->priority = obj.priority;
            this->due_date = obj.due_date;
            this->weight = obj.weight;
            this->duration = obj.duration;
            this->time_started = obj.time_started;
        }

        Task&operator = ( const Task& obj )
        {
            if ( &obj == this ) {
                return *this;
            }
            this->priority = obj.priority;
            this->due_date = obj.due_date;
            this->weight = obj.weight;
            this->duration = obj.duration;
            this->time_started = obj.time_started;

            return *this;
        }
};


#endif //GP_SCHEDULING_TASK_H
