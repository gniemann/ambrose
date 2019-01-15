//
// Created by Greg Niemann on 2019-01-12.
//

#include "config.h"

void Configuration::update(std::vector<Stage> newStages) {
    hasChanged.resize(newStages.size());
    stages.resize(newStages.size());

    auto newStageIter = newStages.begin();
    auto hasChangedIter = hasChanged.begin();

    for (auto&& stage: stages) {
        *hasChangedIter = stage != *newStageIter;
        stage = *newStageIter;
        hasChangedIter++;
        newStageIter++;
    }
}