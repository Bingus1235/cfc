/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_PERMISSION_RULES_PERMISSION_RULE_FEATURES_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_PERMISSION_RULES_PERMISSION_RULE_FEATURES_H_

#include "base/feature_list.h"

namespace brave_ads::permission_rules::features {

BASE_DECLARE_FEATURE(kFeature);

bool IsEnabled();

bool ShouldOnlyServeAdsInWindowedMode();

bool ShouldOnlyServeAdsWithValidInternetConnection();

bool ShouldOnlyServeAdsIfMediaIsNotPlaying();

bool ShouldOnlyServeAdsIfBrowserIsActive();

}  // namespace brave_ads::permission_rules::features

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_PERMISSION_RULES_PERMISSION_RULE_FEATURES_H_
