/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/account/confirmations/confirmations.h"

#include <utility>

#include "base/functional/bind.h"
#include "base/time/time.h"
#include "brave/components/brave_ads/common/pref_names.h"
#include "brave/components/brave_ads/core/internal/account/confirmations/confirmation_user_data_builder.h"
#include "brave/components/brave_ads/core/internal/account/confirmations/confirmation_util.h"
#include "brave/components/brave_ads/core/internal/account/transactions/transaction_info.h"
#include "brave/components/brave_ads/core/internal/account/utility/redeem_unblinded_token/redeem_unblinded_token.h"
#include "brave/components/brave_ads/core/internal/ads_client_helper.h"
#include "brave/components/brave_ads/core/internal/common/logging_util.h"
#include "brave/components/brave_ads/core/internal/common/time/time_formatting_util.h"
#include "brave/components/brave_ads/core/internal/deprecated/confirmations/confirmation_state_manager.h"
#include "brave/components/brave_ads/core/internal/privacy/tokens/token_generator_interface.h"
#include "brave/components/brave_ads/core/internal/privacy/tokens/unblinded_payment_tokens/unblinded_payment_token_info.h"
#include "brave/components/brave_ads/core/internal/privacy/tokens/unblinded_payment_tokens/unblinded_payment_token_util.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace ads {

namespace {

constexpr base::TimeDelta kRetryAfter = base::Seconds(15);

void AppendToRetryQueue(const ConfirmationInfo& confirmation) {
  DCHECK(IsValid(confirmation));

  ConfirmationStateManager::GetInstance()->AppendFailedConfirmation(
      confirmation);
  ConfirmationStateManager::GetInstance()->Save();

  BLOG(1, "Added " << confirmation.type << " confirmation for "
                   << confirmation.ad_type << " with transaction id "
                   << confirmation.transaction_id
                   << " and creative instance id "
                   << confirmation.creative_instance_id
                   << " to the confirmations queue");
}

void RemoveFromRetryQueue(const ConfirmationInfo& confirmation) {
  DCHECK(IsValid(confirmation));

  if (!ConfirmationStateManager::GetInstance()->RemoveFailedConfirmation(
          confirmation)) {
    BLOG(0, "Failed to remove "
                << confirmation.type << " confirmation for "
                << confirmation.ad_type << " with transaction id "
                << confirmation.transaction_id << " and creative instance id "
                << confirmation.creative_instance_id
                << " from the confirmations queue");

    return;
  }

  BLOG(1, "Removed " << confirmation.type << " confirmation for "
                     << confirmation.ad_type << " with transaction id "
                     << confirmation.transaction_id
                     << " and creative instance id "
                     << confirmation.creative_instance_id
                     << " from the confirmations queue");

  ConfirmationStateManager::GetInstance()->Save();
}

}  // namespace

Confirmations::Confirmations(privacy::TokenGeneratorInterface* token_generator)
    : token_generator_(token_generator),
      redeem_unblinded_token_(std::make_unique<RedeemUnblindedToken>()) {
  DCHECK(token_generator_);

  redeem_unblinded_token_->SetDelegate(this);
}

Confirmations::~Confirmations() {
  delegate_ = nullptr;
}

void Confirmations::Confirm(const TransactionInfo& transaction) {
  DCHECK(transaction.IsValid());

  BLOG(1, "Confirming " << transaction.confirmation_type << " for "
                        << transaction.ad_type << " with transaction id "
                        << transaction.id << " and creative instance id "
                        << transaction.creative_instance_id);

  const base::Time created_at = base::Time::Now();

  const ConfirmationUserDataBuilder user_data_builder(
      created_at, transaction.creative_instance_id,
      transaction.confirmation_type);
  user_data_builder.Build(
      base::BindOnce(&Confirmations::CreateNewConfirmationAndRedeemToken,
                     base::Unretained(this), transaction, created_at));
}

void Confirmations::ProcessRetryQueue() {
  if (!retry_timer_.IsRunning()) {
    Retry();
  }
}

///////////////////////////////////////////////////////////////////////////////

void Confirmations::Retry() {
  const ConfirmationList& failed_confirmations =
      ConfirmationStateManager::GetInstance()->GetFailedConfirmations();
  if (failed_confirmations.empty()) {
    BLOG(1, "No failed confirmations to retry");
    return;
  }

  DCHECK(!retry_timer_.IsRunning());
  const base::Time retry_at = retry_timer_.StartWithPrivacy(
      FROM_HERE, kRetryAfter,
      base::BindOnce(&Confirmations::OnRetry, base::Unretained(this)));

  BLOG(1, "Retry sending failed confirmations "
              << FriendlyDateAndTime(retry_at, /*use_sentence_style*/ true));
}

void Confirmations::OnRetry() {
  const ConfirmationList& failed_confirmations =
      ConfirmationStateManager::GetInstance()->GetFailedConfirmations();
  DCHECK(!failed_confirmations.empty());

  BLOG(1, "Retry sending failed confirmations");

  const ConfirmationInfo confirmation_copy = failed_confirmations.front();
  RemoveFromRetryQueue(confirmation_copy);

  const ConfirmationUserDataBuilder user_data_builder(
      confirmation_copy.created_at, confirmation_copy.creative_instance_id,
      confirmation_copy.type);
  user_data_builder.Build(
      base::BindOnce(&Confirmations::RebuildConfirmationAndRedeemToken,
                     base::Unretained(this), confirmation_copy));
}

void Confirmations::StopRetrying() {
  retry_timer_.Stop();
}

void Confirmations::CreateNewConfirmationAndRedeemToken(
    const TransactionInfo& transaction,
    const base::Time& created_at,
    base::Value::Dict user_data) {
  const absl::optional<ConfirmationInfo> confirmation = CreateConfirmation(
      token_generator_, created_at, transaction.id,
      transaction.creative_instance_id, transaction.confirmation_type,
      transaction.ad_type, std::move(user_data));
  if (!confirmation) {
    BLOG(0, "Failed to create and redeem confirmation token");
    return;
  }

  RedeemConfirmationToken(*confirmation);
}

void Confirmations::RebuildConfirmationAndRedeemToken(
    const ConfirmationInfo& confirmation,
    base::Value::Dict user_data) {
  const absl::optional<ConfirmationInfo> new_confirmation = CreateConfirmation(
      token_generator_, confirmation.created_at, confirmation.transaction_id,
      confirmation.creative_instance_id, confirmation.type,
      confirmation.ad_type, std::move(user_data));
  if (!new_confirmation) {
    BLOG(0, "Failed to rebuild and redeem confirmation token");
    return;
  }

  RedeemConfirmationToken(confirmation);
}

void Confirmations::RedeemConfirmationToken(
    const ConfirmationInfo& confirmation) {
  DCHECK(IsValid(confirmation));

  redeem_unblinded_token_->Redeem(confirmation);
}

void Confirmations::OnDidSendConfirmation(
    const ConfirmationInfo& confirmation) {
  if (delegate_) {
    delegate_->OnDidConfirm(confirmation);
  }

  StopRetrying();

  ProcessRetryQueue();
}

void Confirmations::OnFailedToSendConfirmation(
    const ConfirmationInfo& confirmation,
    const bool should_retry) {
  if (should_retry) {
    AppendToRetryQueue(confirmation);
  }

  if (delegate_) {
    delegate_->OnFailedToConfirm(confirmation);
  }

  ProcessRetryQueue();
}

void Confirmations::OnDidRedeemUnblindedToken(
    const ConfirmationInfo& confirmation,
    const privacy::UnblindedPaymentTokenInfo& unblinded_payment_token) {
  if (privacy::UnblindedPaymentTokenExists(unblinded_payment_token)) {
    BLOG(1, "Unblinded payment token is a duplicate");
    return OnFailedToRedeemUnblindedToken(confirmation, /*should_retry*/ false,
                                          /*should_backoff*/ false);
  }

  privacy::AddUnblindedPaymentTokens({unblinded_payment_token});

  const base::Time next_token_redemption_at =
      AdsClientHelper::GetInstance()->GetTimePref(
          prefs::kNextTokenRedemptionAt);

  BLOG(1, "You have " << privacy::UnblindedPaymentTokenCount()
                      << " unblinded payment tokens which will be redeemed "
                      << FriendlyDateAndTime(next_token_redemption_at,
                                             /*use_sentence_style*/ true));

  if (delegate_) {
    delegate_->OnDidConfirm(confirmation);
  }

  StopRetrying();

  ProcessRetryQueue();
}

void Confirmations::OnFailedToRedeemUnblindedToken(
    const ConfirmationInfo& confirmation,
    const bool should_retry,
    const bool should_backoff) {
  DCHECK(IsValid(confirmation));

  if (should_retry) {
    AppendToRetryQueue(confirmation);
  }

  if (delegate_) {
    delegate_->OnFailedToConfirm(confirmation);
  }

  if (!should_backoff) {
    StopRetrying();
  }

  ProcessRetryQueue();
}

}  // namespace ads
