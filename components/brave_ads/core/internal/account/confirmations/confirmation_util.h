/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_CONFIRMATIONS_CONFIRMATION_UTIL_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_CONFIRMATIONS_CONFIRMATION_UTIL_H_

#include <string>

#include "base/values.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace base {
class Time;
}  // namespace base

namespace brave_ads {

namespace privacy {
class TokenGeneratorInterface;
}  // namespace privacy

class AdType;
class ConfirmationType;
struct ConfirmationInfo;

absl::optional<ConfirmationInfo> CreateConfirmation(
    privacy::TokenGeneratorInterface* token_generator,
    base::Time created_at,
    const std::string& transaction_id,
    const std::string& creative_instance_id,
    const ConfirmationType& confirmation_type,
    const AdType& ad_type,
    base::Value::Dict user_data);

bool IsValid(const ConfirmationInfo& confirmation);

void ResetConfirmations();

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_CONFIRMATIONS_CONFIRMATION_UTIL_H_
