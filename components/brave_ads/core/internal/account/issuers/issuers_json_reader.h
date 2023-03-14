/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_ISSUERS_ISSUERS_JSON_READER_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_ISSUERS_ISSUERS_JSON_READER_H_

#include <string>

#include "third_party/abseil-cpp/absl/types/optional.h"

namespace brave_ads {

struct IssuersInfo;

namespace json::reader {

absl::optional<IssuersInfo> ReadIssuers(const std::string& json);

}  // namespace json::reader
}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_ISSUERS_ISSUERS_JSON_READER_H_
