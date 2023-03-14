/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/catalog/campaign/creative_set/creative/new_tab_page_ad/catalog_creative_new_tab_page_ad_info.h"

namespace brave_ads {

bool CatalogCreativeNewTabPageAdInfo::operator==(
    const CatalogCreativeNewTabPageAdInfo& other) const {
  return CatalogCreativeInfo::operator==(other) && payload == other.payload;
}

bool CatalogCreativeNewTabPageAdInfo::operator!=(
    const CatalogCreativeNewTabPageAdInfo& other) const {
  return !(*this == other);
}

}  // namespace brave_ads
