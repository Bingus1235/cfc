/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_COMMON_METRICS_FIELD_TRIAL_PARAMS_UTIL_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_COMMON_METRICS_FIELD_TRIAL_PARAMS_UTIL_H_

#include <string>

#include "base/feature_list.h"

namespace base {
class TimeDelta;
}  // namespace base

namespace brave_ads {

std::string GetFieldTrialParamByFeatureAsString(
    const base::Feature& feature,
    const std::string& param_name,
    const std::string& default_value);

base::TimeDelta GetFieldTrialParamByFeatureAsTimeDelta(
    const base::Feature& feature,
    const std::string& param_name,
    base::TimeDelta default_value);

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_COMMON_METRICS_FIELD_TRIAL_PARAMS_UTIL_H_
