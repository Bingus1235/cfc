/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_ELIGIBLE_ADS_PACING_PACING_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_ELIGIBLE_ADS_PACING_PACING_H_

#include <iterator>

#include "base/ranges/algorithm.h"
#include "brave/components/brave_ads/core/internal/ads/serving/eligible_ads/pacing/pacing_util.h"

namespace brave_ads {

struct CreativeAdInfo;

template <typename T>
T PaceCreativeAds(const T& creative_ads) {
  if (creative_ads.empty()) {
    return {};
  }

  T paced_creative_ads;

  base::ranges::copy_if(creative_ads, std::back_inserter(paced_creative_ads),
                        [](const CreativeAdInfo& creative_ad) {
                          return !ShouldPaceAd(creative_ad);
                        });

  return paced_creative_ads;
}

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_ELIGIBLE_ADS_PACING_PACING_H_
