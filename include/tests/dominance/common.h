#ifndef COMMON_H
#define COMMON_H

#include <gtest/gtest.h>
#include "probfd/dominance/fact_names.h"
#include "probfd/dominance/fts_task.h"
#include "probfd/dominance/labelled_transition_system.h"
#include "probfd/dominance/manual_factor_dominance_relation.h"
#include "probfd/dominance/manual_label_outcome_relation.h"
#include "probfd/dominance/strong_types.h"

using namespace probfd::dominance;

#define FTSTASK(...) \
    auto fn = std::make_shared<NoFactNames>(); \
    auto fvn = std::make_shared<FactValueNames>(fn, -1); \
    FTSTask fts_task(__VA_ARGS__, std::nullopt); \
    LabelOutcomeMap lom(fts_task);

#define LTS(...) \
    LabelledTransitionSystem(__VA_ARGS__, fvn)

#define COSTS(...) \
    {__VA_ARGS__}

#define OUTCOMES(...) \
    {__VA_ARGS__}

#define TR(...) \
    {__VA_ARGS__}




#endif //COMMON_H
