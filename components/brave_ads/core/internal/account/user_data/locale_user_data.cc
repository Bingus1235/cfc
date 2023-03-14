/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/account/user_data/locale_user_data.h"

#include <string>

#include "brave/components/brave_ads/common/interfaces/ads.mojom.h"
#include "brave/components/brave_ads/core/build_channel.h"
#include "brave/components/brave_ads/core/internal/privacy/locale/country_code_util.h"
#include "brave/components/l10n/common/locale_util.h"

namespace brave_ads::user_data {

namespace {

constexpr char kCountryCodeKey[] = "countryCode";
constexpr char kOtherCountryCode[] = "??";

}  // namespace

base::Value::Dict GetLocale() {
  base::Value::Dict user_data;

  if (!BuildChannel().is_release) {
    return user_data;
  }

  const std::string country_code = brave_l10n::GetDefaultISOCountryCodeString();

  if (privacy::locale::IsCountryCodeMemberOfAnonymitySet(country_code)) {
    user_data.Set(kCountryCodeKey, country_code);
  } else {
    if (privacy::locale::ShouldClassifyCountryCodeAsOther(country_code)) {
      user_data.Set(kCountryCodeKey, kOtherCountryCode);
    }
  }

  return user_data;
}

}  // namespace brave_ads::user_data
