/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_TARGETING_MODELS_BEHAVIORAL_PURCHASE_INTENT_PURCHASE_INTENT_MODEL_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_TARGETING_MODELS_BEHAVIORAL_PURCHASE_INTENT_PURCHASE_INTENT_MODEL_H_

#include "brave/components/brave_ads/core/internal/ads/serving/targeting/models/model_interface.h"

namespace brave_ads::targeting::model {

class PurchaseIntent final : public ModelInterface {
 public:
  SegmentList GetSegments() const override;
};

}  // namespace brave_ads::targeting::model

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_TARGETING_MODELS_BEHAVIORAL_PURCHASE_INTENT_PURCHASE_INTENT_MODEL_H_
