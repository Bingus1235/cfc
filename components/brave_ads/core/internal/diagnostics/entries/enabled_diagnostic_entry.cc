/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/diagnostics/entries/enabled_diagnostic_entry.h"

#include "brave/components/brave_ads/common/pref_names.h"
#include "brave/components/brave_ads/core/internal/ads_client_helper.h"
#include "brave/components/brave_ads/core/internal/common/strings/string_conversions_util.h"

namespace brave_ads {

namespace {
constexpr char kName[] = "Enabled";
}  // namespace

DiagnosticEntryType EnabledDiagnosticEntry::GetType() const {
  return DiagnosticEntryType::kEnabled;
}

std::string EnabledDiagnosticEntry::GetName() const {
  return kName;
}

std::string EnabledDiagnosticEntry::GetValue() const {
  const bool is_enabled =
      AdsClientHelper::GetInstance()->GetBooleanPref(prefs::kEnabled);
  return BoolToString(is_enabled);
}

}  // namespace brave_ads
