#ifndef STATE_NAME_H
#define STATE_NAME_H

#include "downward/task_proxy.h"
#include "downward/tasks/root_task.h"

#include <format>
#include <string>

namespace probfd {
template <typename State>
inline std::string state_name(State s)
{
    throw std::runtime_error("State name not implemented");
}

inline std::string state_name(downward::State s) {
    s.unpack();
    std::string name;
    for (const auto& [var, val] : std::views::enumerate(s.get_unpacked_values())) {;
        name += std::format("{}; ", s.get_task().get_variables()[var].get_fact(val).get_name());
    }
    return name;
}

template <typename Action>
inline std::string action_name(const downward::PlanningTaskProxy& task, Action a)
{
    throw std::runtime_error("Action name not implemented");
}

inline std::string action_name(const downward::PlanningTaskProxy& task, downward::OperatorID op_id)
{
    return task.get_partial_operators()[op_id.get_index()].get_name();
}
}
#endif //STATE_NAME_H
