/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/privacy/challenge_bypass_ristretto/token_util.h"

#include "brave/components/brave_ads/core/internal/privacy/challenge_bypass_ristretto/token.h"
#include "brave/components/brave_ads/core/internal/privacy/challenge_bypass_ristretto/token_unittest_util.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace brave_ads::privacy::cbr {

TEST(BatAdsTokenUtilTest, TokensToRawTokens) {
  // Arrange
  const std::vector<Token> tokens = GetTokens();

  // Act
  const std::vector<challenge_bypass_ristretto::Token> raw_tokens =
      ToRawTokens(tokens);

  // Assert
  std::vector<challenge_bypass_ristretto::Token> expected_raw_tokens;
  expected_raw_tokens.reserve(tokens.size());
  for (const auto& token : tokens) {
    expected_raw_tokens.push_back(token.get());
  }

  EXPECT_EQ(expected_raw_tokens, raw_tokens);
}

TEST(BatAdsTokenUtilTest, EmptyTokensToRawTokens) {
  // Arrange
  const std::vector<Token> tokens;

  // Act
  const std::vector<challenge_bypass_ristretto::Token> raw_tokens =
      ToRawTokens(tokens);

  // Assert
  EXPECT_TRUE(raw_tokens.empty());
}

}  // namespace brave_ads::privacy::cbr
