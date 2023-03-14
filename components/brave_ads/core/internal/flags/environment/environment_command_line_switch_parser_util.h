/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FLAGS_ENVIRONMENT_ENVIRONMENT_COMMAND_LINE_SWITCH_PARSER_UTIL_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FLAGS_ENVIRONMENT_ENVIRONMENT_COMMAND_LINE_SWITCH_PARSER_UTIL_H_

#include "brave/components/brave_ads/core/internal/flags/environment/environment_types.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace brave_ads {

absl::optional<EnvironmentType> ParseEnvironmentCommandLineSwitch();

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_FLAGS_ENVIRONMENT_ENVIRONMENT_COMMAND_LINE_SWITCH_PARSER_UTIL_H_
