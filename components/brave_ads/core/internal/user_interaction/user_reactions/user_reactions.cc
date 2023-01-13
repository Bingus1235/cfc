/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/user_interaction/user_reactions/user_reactions.h"

#include "brave/components/brave_ads/core/ad_content_info.h"
#include "brave/components/brave_ads/core/confirmation_type.h"
#include "brave/components/brave_ads/core/internal/account/account.h"
#include "brave/components/brave_ads/core/internal/history/history_manager.h"

namespace ads {

UserReactions::UserReactions(Account* account) : account_(account) {
  DCHECK(account_);

  HistoryManager::GetInstance()->AddObserver(this);
}

UserReactions::~UserReactions() {
  HistoryManager::GetInstance()->RemoveObserver(this);
}

///////////////////////////////////////////////////////////////////////////////

// TODO(tmancey): Change ad_content.brand to segment
void UserReactions::OnDidLikeAd(const AdContentInfo& ad_content) {
  account_->Deposit(ad_content.creative_instance_id, ad_content.type,
                    ad_content.brand, ConfirmationType::kUpvoted);
}

// TODO(tmancey): Change ad_content.brand to segment
void UserReactions::OnDidDislikeAd(const AdContentInfo& ad_content) {
  account_->Deposit(ad_content.creative_instance_id, ad_content.type,
                    ad_content.brand, ConfirmationType::kDownvoted);
}

// TODO(tmancey): Change ad_content.brand to segment
void UserReactions::OnDidMarkAdAsInappropriate(
    const AdContentInfo& ad_content) {
  account_->Deposit(ad_content.creative_instance_id, ad_content.type,
                    ad_content.brand, ConfirmationType::kFlagged);
}

// TODO(tmancey): Change ad_content.brand to segment
void UserReactions::OnDidSaveAd(const AdContentInfo& ad_content) {
  account_->Deposit(ad_content.creative_instance_id, ad_content.type,
                    ad_content.brand, ConfirmationType::kSaved);
}

}  // namespace ads
