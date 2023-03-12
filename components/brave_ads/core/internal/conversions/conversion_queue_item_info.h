/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_CONVERSIONS_CONVERSION_QUEUE_ITEM_INFO_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_CONVERSIONS_CONVERSION_QUEUE_ITEM_INFO_H_

#include <string>
#include <vector>

#include "base/time/time.h"
#include "brave/components/brave_ads/core/ad_type.h"

namespace ads {

struct ConversionQueueItemInfo final {
  ConversionQueueItemInfo();

  ConversionQueueItemInfo(const ConversionQueueItemInfo& other);
  ConversionQueueItemInfo& operator=(const ConversionQueueItemInfo& other);

  ConversionQueueItemInfo(ConversionQueueItemInfo&& other) noexcept;
  ConversionQueueItemInfo& operator=(ConversionQueueItemInfo&& other) noexcept;

  ~ConversionQueueItemInfo();

  bool operator==(const ConversionQueueItemInfo& other) const;
  bool operator!=(const ConversionQueueItemInfo& other) const;

  AdType ad_type = AdType::kUndefined;
  std::string creative_instance_id;
  std::string creative_set_id;
  std::string campaign_id;
  std::string advertiser_id;
  std::string segment;
  std::string conversion_id;
  std::string advertiser_public_key;
  base::Time process_at;
  bool was_processed = false;

  bool IsValid() const;
};

using ConversionQueueItemList = std::vector<ConversionQueueItemInfo>;

}  // namespace ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_CONVERSIONS_CONVERSION_QUEUE_ITEM_INFO_H_
