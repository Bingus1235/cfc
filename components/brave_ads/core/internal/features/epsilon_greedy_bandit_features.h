/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FEATURES_EPSILON_GREEDY_BANDIT_FEATURES_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FEATURES_EPSILON_GREEDY_BANDIT_FEATURES_H_

#include "base/feature_list.h"

namespace ads::targeting::features {

BASE_DECLARE_FEATURE(kEpsilonGreedyBandit);

bool IsEpsilonGreedyBanditEnabled();

double GetEpsilonGreedyBanditEpsilonValue();

}  // namespace ads::targeting::features

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FEATURES_EPSILON_GREEDY_BANDIT_FEATURES_H_
