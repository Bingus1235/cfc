// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/webui/chat_ui/chat_ui_page_handler.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "base/strings/strcat.h"
#include "brave/components/chat_ui/common/chat_ui_constants.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"

ChatUIPageHandler::ChatUIPageHandler(
    TabStripModel* tab_strip_model,
    mojo::PendingReceiver<chat_ui::mojom::PageHandler> receiver)
    : receiver_(this, std::move(receiver)) {
  DCHECK(tab_strip_model);

  auto* web_contents = tab_strip_model->GetActiveWebContents();
  if (!web_contents) {
    return;
  }

  active_chat_tab_helper_ = ChatTabHelper::FromWebContents(web_contents);

  api_helper_ = std::make_unique<chat_ui::ChatUIAPIRequest>(
      web_contents->GetBrowserContext()
          ->GetDefaultStoragePartition()
          ->GetURLLoaderFactoryForBrowserProcess());
}

ChatUIPageHandler::~ChatUIPageHandler() = default;

void ChatUIPageHandler::SetClientPage(
    mojo::PendingRemote<chat_ui::mojom::ChatUIPage> page) {
  page_.Bind(std::move(page));
}

void ChatUIPageHandler::QueryPrompt(const std::string& input) {
  active_chat_tab_helper_->AddToConversationHistory(
      {chat_ui::mojom::CharacterType::HUMAN, input});

  DCHECK(api_helper_);

  api_helper_->QueryPrompt(
      base::BindOnce(&ChatUIPageHandler::OnResponse, base::Unretained(this)),
      base::StrCat({chat_ui::kHumanPrompt, input, chat_ui::kAIPrompt}));
}

void ChatUIPageHandler::GetConversationHistory(
    GetConversationHistoryCallback callback) {
  std::vector<ConversationTurn> history =
      active_chat_tab_helper_->GetConversationHistory();

  std::vector<chat_ui::mojom::ConversationTurnPtr> list;
  std::transform(history.begin(), history.end(), std::back_inserter(list),
                 [](const ConversationTurn& turn) { return turn.Clone(); });

  std::move(callback).Run(std::move(list));
}

void ChatUIPageHandler::OnResponse(const std::string& assistant_input,
                                   bool success) {
  if (!success) {
    return;
  }

  ConversationTurn turn = {CharacterType::ASSISTANT, assistant_input};
  page_.get()->OnResponse(turn.Clone());
  active_chat_tab_helper_->AddToConversationHistory(turn);
}
