//
// Created by Greg Niemann on 2019-01-12.
//

#include "config.h"

void Configuration::update(std::vector<Stage> newStages) {
    stages.resize(newStages.size());

    auto newStageIter = newStages.begin();
    for (auto&& stage: stages) {
        newStageIter->setPrevStatus(stage.getStatus());
        stage = *newStageIter;
        newStageIter++;
    }
}