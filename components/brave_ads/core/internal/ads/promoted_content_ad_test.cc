/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/ad_type.h"
#include "brave/components/brave_ads/core/confirmation_type.h"
#include "brave/components/brave_ads/core/internal/account/transactions/transactions_unittest_util.h"
#include "brave/components/brave_ads/core/internal/ads/ad_events/ad_event_unittest_util.h"
#include "brave/components/brave_ads/core/internal/ads/serving/permission_rules/permission_rules_unittest_util.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_base.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_mock_util.h"
#include "brave/components/brave_ads/core/internal/history/history_unittest_util.h"
#include "net/http/http_status_code.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace brave_ads {

namespace {

constexpr char kPlacementId[] = "f0948316-df6f-4e31-814d-d0b5f2a1f28c";
constexpr char kCreativeInstanceIdId[] = "75d4cbac-b661-4126-9ccb-7bbb6ee56ef3";

}  // namespace

class BatAdsPromotedContentAdIntegrationTest : public UnitTestBase {
 protected:
  void SetUp() override {
    UnitTestBase::SetUpForTesting(/*is_integration_test*/ true);

    ForcePermissionRulesForTesting();
  }

  void SetUpMocks() override {
    const URLResponseMap url_responses = {
        {"/v9/catalog", {{net::HTTP_OK, "/catalog.json"}}}};
    MockUrlResponses(ads_client_mock_, url_responses);
  }
};

TEST_F(BatAdsPromotedContentAdIntegrationTest, TriggerViewedEvent) {
  // Arrange
  GetAds()->TriggerPromotedContentAdEvent(
      kPlacementId, kCreativeInstanceIdId,
      mojom::PromotedContentAdEventType::kServed);

  // Act
  GetAds()->TriggerPromotedContentAdEvent(
      kPlacementId, kCreativeInstanceIdId,
      mojom::PromotedContentAdEventType::kViewed);

  // Assert
  EXPECT_EQ(1, GetAdEventCount(AdType::kPromotedContentAd,
                               ConfirmationType::kServed));
  EXPECT_EQ(1, GetAdEventCount(AdType::kPromotedContentAd,
                               ConfirmationType::kViewed));
  EXPECT_EQ(1, GetHistoryItemCount());
  EXPECT_EQ(1, GetTransactionCount());
}

TEST_F(BatAdsPromotedContentAdIntegrationTest, TriggerClickedEvent) {
  // Arrange
  GetAds()->TriggerPromotedContentAdEvent(
      kPlacementId, kCreativeInstanceIdId,
      mojom::PromotedContentAdEventType::kServed);
  GetAds()->TriggerPromotedContentAdEvent(
      kPlacementId, kCreativeInstanceIdId,
      mojom::PromotedContentAdEventType::kViewed);

  // Act
  GetAds()->TriggerPromotedContentAdEvent(
      kPlacementId, kCreativeInstanceIdId,
      mojom::PromotedContentAdEventType::kClicked);

  // Assert
  EXPECT_EQ(1, GetAdEventCount(AdType::kPromotedContentAd,
                               ConfirmationType::kServed));
  EXPECT_EQ(1, GetAdEventCount(AdType::kPromotedContentAd,
                               ConfirmationType::kViewed));
  EXPECT_EQ(1, GetAdEventCount(AdType::kPromotedContentAd,
                               ConfirmationType::kClicked));
  EXPECT_EQ(2, GetHistoryItemCount());
  EXPECT_EQ(2, GetTransactionCount());
}

}  // namespace brave_ads
