// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include "brave/browser/ui/webui/chat_ui/chat_ui_page_handler.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "base/strings/strcat.h"
#include "base/strings/utf_string_conversions.h"
#include "brave/components/chat_ui/common/chat_ui_constants.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/common/chrome_isolated_world_ids.h"
#include "components/grit/brave_components_strings.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/storage_partition.h"
#include "ui/base/l10n/l10n_util.h"

using chat_ui::mojom::CharacterType;
using chat_ui::mojom::ConversationTurnVisibility;

ChatUIPageHandler::ChatUIPageHandler(
    TabStripModel* tab_strip_model,
    mojo::PendingReceiver<chat_ui::mojom::PageHandler> receiver)
    : receiver_(this, std::move(receiver)) {
  DCHECK(tab_strip_model);
  tab_strip_model->AddObserver(this);

  auto* web_contents = tab_strip_model->GetActiveWebContents();
  if (!web_contents) {
    return;
  }

  active_chat_tab_helper_ = ChatTabHelper::FromWebContents(web_contents);

  chat_tab_helper_observation_.Observe(active_chat_tab_helper_);

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

void ChatUIPageHandler::GetCompletions(const std::string& input) {
  MakeAPIRequestWithConversationHistoryUpdate(
      {CharacterType::HUMAN, ConversationTurnVisibility::VISIBLE, input});
}

void ChatUIPageHandler::GetConversationHistory(
    GetConversationHistoryCallback callback) {
  std::vector<ConversationTurn> history =
      active_chat_tab_helper_->GetConversationHistory();

  std::vector<chat_ui::mojom::ConversationTurnPtr> list;

  // Remove conversations that are meant to be hidden from the user
  auto new_end_it = std::remove_if(
      history.begin(), history.end(), [](const ConversationTurn& turn) {
        return turn.visibility == ConversationTurnVisibility::HIDDEN;
      });

  std::transform(history.begin(), new_end_it, std::back_inserter(list),
                 [](const ConversationTurn& turn) { return turn.Clone(); });

  std::move(callback).Run(std::move(list));
}

void ChatUIPageHandler::GetSummary() {
  active_chat_tab_helper_->GetArticleSummaryString(base::BindOnce(
      &ChatUIPageHandler::OnArticleSummaryResult, base::Unretained(this)));
}

void ChatUIPageHandler::OnArticleSummaryResult(const std::u16string& result,
                                               bool is_from_cache) {
  if (is_from_cache) {
    active_chat_tab_helper_->AddToConversationHistory({
        CharacterType::ASSISTANT,
        ConversationTurnVisibility::VISIBLE,
        base::UTF16ToUTF8(result),
    });
    page_.get()->OnConversationHistoryUpdate();
    return;
  }

  std::string summarize_prompt = base::ReplaceStringPlaceholders(
      l10n_util::GetStringUTF8(IDS_CHAT_PROMPT_TEMPLATE),
      {base::UTF16ToUTF8(result)}, nullptr);

  // We hide the prompt with article content from the user
  MakeAPIRequestWithConversationHistoryUpdate(
      {CharacterType::HUMAN, ConversationTurnVisibility::HIDDEN,
       summarize_prompt});
}

void ChatUIPageHandler::MakeAPIRequestWithConversationHistoryUpdate(
    ConversationTurn turn) {
  active_chat_tab_helper_->AddToConversationHistory(turn);
  page_.get()->OnConversationHistoryUpdate();

  std::string prompt_with_history =
      base::StrCat({active_chat_tab_helper_->GetConversationHistoryAsString(),
                    chat_ui::kAIPrompt});

  DCHECK(api_helper_);

  // We assume that a hidden conversation contains a summary
  // TODO(nullhook): This is not always true, but should be a better heuristic
  bool contains_summary =
      turn.visibility == ConversationTurnVisibility::HIDDEN ? true : false;

  api_helper_->QueryPrompt(
      base::BindOnce(&ChatUIPageHandler::OnAPIResponse, base::Unretained(this),
                     contains_summary),
      std::move(prompt_with_history));
}

void ChatUIPageHandler::OnAPIResponse(bool contains_summary,
                                      const std::string& assistant_input,
                                      bool success) {
  if (!success) {
    return;
  }

  ConversationTurn turn = {CharacterType::ASSISTANT,
                           ConversationTurnVisibility::VISIBLE,
                           assistant_input};
  active_chat_tab_helper_->AddToConversationHistory(turn);

  std::u16string output;
  if (contains_summary &&
      base::UTF8ToUTF16(turn.text.c_str(), turn.text.length(), &output)) {
    active_chat_tab_helper_->SetArticleSummaryString(output);
  }

  page_.get()->OnConversationHistoryUpdate();
}

void ChatUIPageHandler::OnPageChanged() {
  page_.get()->OnConversationHistoryUpdate();
}

void ChatUIPageHandler::OnTabStripModelChanged(
    TabStripModel* tab_strip_model,
    const TabStripModelChange& change,
    const TabStripSelectionChange& selection) {
  if (selection.active_tab_changed()) {
    if (active_chat_tab_helper_) {
      active_chat_tab_helper_ = nullptr;
      chat_tab_helper_observation_.Reset();
    }

    if (selection.new_contents) {
      active_chat_tab_helper_ =
          ChatTabHelper::FromWebContents(selection.new_contents);
      chat_tab_helper_observation_.Observe(active_chat_tab_helper_);

      page_.get()->OnConversationHistoryUpdate();
    }
  }
}
