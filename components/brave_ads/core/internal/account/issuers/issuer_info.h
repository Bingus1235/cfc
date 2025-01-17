/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_ISSUERS_ISSUER_INFO_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_ISSUERS_ISSUER_INFO_H_

#include <vector>

#include "brave/components/brave_ads/core/internal/account/issuers/issuer_types.h"
#include "brave/components/brave_ads/core/internal/account/issuers/public_key_alias.h"

namespace ads {

struct IssuerInfo final {
  IssuerInfo();

  IssuerInfo(const IssuerInfo& other);
  IssuerInfo& operator=(const IssuerInfo& other);

  IssuerInfo(IssuerInfo&& other) noexcept;
  IssuerInfo& operator=(IssuerInfo&& other) noexcept;

  ~IssuerInfo();

  bool operator==(const IssuerInfo& other) const;
  bool operator!=(const IssuerInfo& other) const;

  IssuerType type = IssuerType::kUndefined;
  PublicKeyMap public_keys;
};

using IssuerList = std::vector<IssuerInfo>;

}  // namespace ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_CORE_INTERNAL_ACCOUNT_ISSUERS_ISSUER_INFO_H_
