/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_PRIVACY_P2A_IMPRESSIONS_P2A_IMPRESSION_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_PRIVACY_P2A_IMPRESSIONS_P2A_IMPRESSION_H_

#include <string>

namespace brave_ads {

class AdType;
struct AdInfo;

namespace privacy::p2a {

std::string GetAdImpressionNameForAdType(const AdType& ad_type);

void RecordAdImpression(const AdInfo& ad);

}  // namespace privacy::p2a
}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_PRIVACY_P2A_IMPRESSIONS_P2A_IMPRESSION_H_
