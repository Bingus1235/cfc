/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_COMMON_STRINGS_STRING_CONVERSIONS_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_COMMON_STRINGS_STRING_CONVERSIONS_UTIL_H_

#include <string>
#include <vector>

namespace ads {

std::string BoolToString(bool value);

std::vector<float> ConvertStringToVector(const std::string& string,
                                         const std::string& delimiter);
std::string ConvertVectorToString(const std::vector<float>& vector,
                                  const std::string& delimiter);

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_COMMON_STRINGS_STRING_CONVERSIONS_UTIL_H_
