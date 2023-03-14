/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_USER_DATA_CONVERSION_USER_DATA_UTIL_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_USER_DATA_CONVERSION_USER_DATA_UTIL_H_

#include "third_party/abseil-cpp/absl/types/optional.h"

namespace brave_ads {

namespace security {
struct VerifiableConversionEnvelopeInfo;
}  // namespace security

struct ConversionQueueItemInfo;

namespace user_data {

absl::optional<security::VerifiableConversionEnvelopeInfo> GetEnvelope(
    const ConversionQueueItemInfo& conversion_queue_item);

}  // namespace user_data
}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_USER_DATA_CONVERSION_USER_DATA_UTIL_H_
