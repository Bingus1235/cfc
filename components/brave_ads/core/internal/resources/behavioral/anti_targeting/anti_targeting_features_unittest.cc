/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/resources/behavioral/anti_targeting/anti_targeting_features.h"

#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace brave_ads::resource::features {

TEST(BatAdsAntiTargetingFeaturesTest, IsAntiTargetingEnabled) {
  // Arrange

  // Act

  // Assert
  EXPECT_TRUE(IsAntiTargetingEnabled());
}

TEST(BatAdsAntiTargetingFeaturesTest, GetAntiTargetingResourceVersion) {
  // Arrange

  // Act

  // Assert
  EXPECT_EQ(1, GetAntiTargetingResourceVersion());
}

}  // namespace brave_ads::resource::features
