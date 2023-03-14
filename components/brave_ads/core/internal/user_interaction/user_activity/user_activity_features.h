/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_USER_INTERACTION_USER_ACTIVITY_USER_ACTIVITY_FEATURES_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_USER_INTERACTION_USER_ACTIVITY_USER_ACTIVITY_FEATURES_H_

#include <string>

#include "base/feature_list.h"

namespace base {
class TimeDelta;
}  // namespace base

namespace brave_ads::user_activity::features {

BASE_DECLARE_FEATURE(kFeature);

bool IsEnabled();

std::string GetTriggers();
base::TimeDelta GetTimeWindow();
double GetThreshold();

base::TimeDelta GetIdleTimeThreshold();
base::TimeDelta GetMaximumIdleTime();

bool ShouldDetectScreenWasLocked();

}  // namespace brave_ads::user_activity::features

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_USER_INTERACTION_USER_ACTIVITY_USER_ACTIVITY_FEATURES_H_
