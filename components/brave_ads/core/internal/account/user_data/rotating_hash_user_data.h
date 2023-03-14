/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_USER_DATA_ROTATING_HASH_USER_DATA_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_USER_DATA_ROTATING_HASH_USER_DATA_H_

#include <string>

#include "base/values.h"

namespace brave_ads::user_data {

base::Value::Dict GetRotatingHash(const std::string& creative_instance_id);

}  // namespace brave_ads::user_data

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_USER_DATA_ROTATING_HASH_USER_DATA_H_
