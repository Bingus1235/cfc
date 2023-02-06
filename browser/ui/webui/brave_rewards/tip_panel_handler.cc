/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/brave_rewards/tip_panel_handler.h"

#include <utility>

#include "base/functional/bind.h"
#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/browser/ui/views/brave_rewards/tip_panel_bubble_host.h"
#include "brave/components/brave_rewards/browser/rewards_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_finder.h"

namespace brave_rewards {

namespace {

std::string GetRequestedPublisherID(Profile* profile) {
  DCHECK(profile);
  if (auto* browser = chrome::FindLastActiveWithProfile(profile)) {
    if (auto* host = TipPanelBubbleHost::FromBrowser(browser)) {
      return host->publisher_id();
    }
  }
  return "";
}

}  // namespace

TipPanelHandler::TipPanelHandler(
    mojo::PendingRemote<mojom::TipPanel> banner,
    mojo::PendingReceiver<mojom::TipPanelHandler> receiver,
    base::WeakPtr<ui::MojoBubbleWebUIController::Embedder> embedder,
    Profile* profile)
    : receiver_(this, std::move(receiver)),
      banner_(std::move(banner)),
      embedder_(embedder),
      rewards_service_(RewardsServiceFactory::GetForProfile(profile)),
      publisher_id_(GetRequestedPublisherID(profile)) {
  DCHECK(embedder_);
  DCHECK(rewards_service_);
}

TipPanelHandler::~TipPanelHandler() = default;

void TipPanelHandler::ShowUI() {
  if (embedder_) {
    embedder_->ShowUI();
  }
}

void TipPanelHandler::CloseUI() {
  if (embedder_) {
    embedder_->CloseUI();
  }
}

void TipPanelHandler::GetRewardsParameters(
    GetRewardsParametersCallback callback) {
  DCHECK(rewards_service_);
  rewards_service_->GetRewardsParameters(std::move(callback));
}

void TipPanelHandler::GetBalance(GetBalanceCallback callback) {
  DCHECK(rewards_service_);

  auto fn = [](GetBalanceCallback callback, FetchBalanceResult result) {
    if (!result.has_value() || !result.value()) {
      std::move(callback).Run(nullptr);
    } else {
      std::move(callback).Run(std::move(result.value()));
    }
  };

  rewards_service_->FetchBalance(base::BindOnce(fn, std::move(callback)));
}

void TipPanelHandler::GetBanner(GetBannerCallback callback) {
  DCHECK(rewards_service_);
  // TODO(zenparsing): Implement
}

void TipPanelHandler::GetExternalWallet(GetExternalWalletCallback callback) {
  DCHECK(rewards_service_);

  auto fn = [](GetExternalWalletCallback callback,
               GetExternalWalletResult result) {
    auto result_value = std::move(result).value_or(nullptr);
    if (!result_value ||
        result_value->status == ledger::mojom::WalletStatus::kNotConnected) {
      return std::move(callback).Run(nullptr);
    }
    std::move(callback).Run(std::move(result.value()));
  };

  rewards_service_->GetExternalWallet(base::BindOnce(fn, std::move(callback)));
}

void TipPanelHandler::SendContribution(double amount,
                                       bool set_monthly,
                                       SendContributionCallback callback) {
  DCHECK(rewards_service_);
  // TODO(zenparsing): Implement
}

void TipPanelHandler::ShareContribution() {
  DCHECK(rewards_service_);
  // TODO(zenparsing): Implement
}

}  // namespace brave_rewards
