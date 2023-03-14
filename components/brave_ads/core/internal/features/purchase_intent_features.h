/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FEATURES_PURCHASE_INTENT_FEATURES_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FEATURES_PURCHASE_INTENT_FEATURES_H_

#include <cstdint>

#include "base/feature_list.h"

namespace base {
class TimeDelta;
}  // namespace base

namespace brave_ads::targeting::features {

BASE_DECLARE_FEATURE(kPurchaseIntent);

bool IsPurchaseIntentEnabled();

uint16_t GetPurchaseIntentThreshold();

base::TimeDelta GetPurchaseIntentTimeWindow();

int GetPurchaseIntentResourceVersion();

}  // namespace brave_ads::targeting::features

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FEATURES_PURCHASE_INTENT_FEATURES_H_
