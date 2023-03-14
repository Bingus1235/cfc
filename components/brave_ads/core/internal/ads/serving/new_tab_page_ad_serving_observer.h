/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_NEW_TAB_PAGE_AD_SERVING_OBSERVER_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_NEW_TAB_PAGE_AD_SERVING_OBSERVER_H_

#include "base/observer_list_types.h"
#include "brave/components/brave_ads/core/internal/segments/segment_alias.h"

namespace brave_ads {

struct NewTabPageAdInfo;

namespace new_tab_page_ads {

class ServingObserver : public base::CheckedObserver {
 public:
  // Invoked when an opportunity arises to serve a new tab page ad for the
  // |segments|.
  virtual void OnOpportunityAroseToServeNewTabPageAd(
      const SegmentList& segments) {}

  // Invoked when a new tab page ad is served.
  virtual void OnDidServeNewTabPageAd(const NewTabPageAdInfo& ad) {}

  // Invoked when a new tab page ad fails to serve.
  virtual void OnFailedToServeNewTabPageAd() {}
};

}  // namespace new_tab_page_ads
}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ADS_SERVING_NEW_TAB_PAGE_AD_SERVING_OBSERVER_H_
