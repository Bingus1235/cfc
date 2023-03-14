/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_RESOURCES_BEHAVIORAL_PURCHASE_INTENT_PURCHASE_INTENT_SITE_INFO_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_RESOURCES_BEHAVIORAL_PURCHASE_INTENT_PURCHASE_INTENT_SITE_INFO_H_

#include <cstdint>

#include "brave/components/brave_ads/core/internal/segments/segment_alias.h"
#include "url/gurl.h"

namespace brave_ads::targeting {

struct PurchaseIntentSiteInfo final {
  PurchaseIntentSiteInfo();
  PurchaseIntentSiteInfo(SegmentList segments,
                         GURL url_netloc,
                         uint16_t weight);

  PurchaseIntentSiteInfo(const PurchaseIntentSiteInfo& other);
  PurchaseIntentSiteInfo& operator=(const PurchaseIntentSiteInfo& other);

  PurchaseIntentSiteInfo(PurchaseIntentSiteInfo&& other) noexcept;
  PurchaseIntentSiteInfo& operator=(PurchaseIntentSiteInfo&& other) noexcept;

  ~PurchaseIntentSiteInfo();

  bool operator==(const PurchaseIntentSiteInfo& other) const;
  bool operator!=(const PurchaseIntentSiteInfo& other) const;

  SegmentList segments;
  GURL url_netloc;
  uint16_t weight = 0;
};

}  // namespace brave_ads::targeting

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_RESOURCES_BEHAVIORAL_PURCHASE_INTENT_PURCHASE_INTENT_SITE_INFO_H_
