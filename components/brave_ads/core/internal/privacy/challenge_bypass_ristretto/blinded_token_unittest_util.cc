/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/privacy/challenge_bypass_ristretto/blinded_token_unittest_util.h"

#include "brave/components/brave_ads/core/internal/privacy/challenge_bypass_ristretto/blinded_token.h"
#include "brave/components/brave_ads/core/internal/privacy/challenge_bypass_ristretto/challenge_bypass_ristretto_unittest_constants.h"

namespace ads::privacy::cbr {

BlindedToken GetBlindedToken() {
  return BlindedToken(kBlindedTokenBase64);
}

std::vector<BlindedToken> GetBlindedTokens() {
  std::vector<BlindedToken> blinded_tokens;
  const BlindedToken blinded_token = GetBlindedToken();
  blinded_tokens.push_back(blinded_token);
  return blinded_tokens;
}

BlindedToken GetInvalidBlindedToken() {
  return BlindedToken(kInvalidBase64);
}

std::vector<BlindedToken> GetInvalidBlindedTokens() {
  std::vector<BlindedToken> blinded_tokens;
  const BlindedToken blinded_token = GetInvalidBlindedToken();
  blinded_tokens.push_back(blinded_token);
  return blinded_tokens;
}

}  // namespace ads::privacy::cbr
