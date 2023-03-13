/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/conversions/conversions_features.h"

#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace brave_ads::features {

TEST(BatAdsConversionsFeaturesTest, ConversionsEnabled) {
  // Arrange

  // Act

  // Assert
  EXPECT_TRUE(IsConversionsEnabled());
}

TEST(BatAdsConversionsFeaturesTest, ConversionsResourceVersion) {
  // Arrange

  // Act

  // Assert
  EXPECT_EQ(1, GetConversionsResourceVersion());
}

TEST(BatAdsConversionsFeaturesTest, DefaultConversionIdPattern) {
  // Arrange

  // Act

  // Assert
  const std::string expected_pattern =
      "<meta.*name=\"ad-conversion-id\".*content=\"([-a-zA-Z0-9]*)\".*>";
  EXPECT_EQ(expected_pattern, GetDefaultConversionIdPattern());
}

}  // namespace brave_ads::features
