// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BRAVE_BROWSER_UI_WEBUI_CHAT_UI_CHAT_UI_PAGE_HANDLER_H_
#define BRAVE_BROWSER_UI_WEBUI_CHAT_UI_CHAT_UI_PAGE_HANDLER_H_

#include <stdint.h>

#include <memory>
#include <string>

#include "base/scoped_observation.h"
#include "brave/browser/chat/chat_tab_helper.h"
#include "brave/browser/speedreader/speedreader_tab_helper.h"
#include "brave/components/chat_ui/browser/chat_ui_api_request.h"
#include "brave/components/chat_ui/common/chat_ui.mojom.h"
#include "chrome/browser/ui/tabs/tab_strip_model_observer.h"
#include "components/services/screen_ai/buildflags/buildflags.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"

namespace ui {
class AXTree;
}  // namespace ui

class TabStripModel;
namespace content {
class WebContents;
}  // namespace content

class ChatUIPageHandler : public chat_ui::mojom::PageHandler,
                          public TabStripModelObserver,
                          public ChatTabHelper::Observer {
 public:
  ChatUIPageHandler(
      TabStripModel* tab_strip_model,
      mojo::PendingReceiver<chat_ui::mojom::PageHandler> receiver);

  ChatUIPageHandler(const ChatUIPageHandler&) = delete;
  ChatUIPageHandler& operator=(const ChatUIPageHandler&) = delete;

  ~ChatUIPageHandler() override;

  // chat_ui::mojom::PageHandler:
  void SetClientPage(
      mojo::PendingRemote<chat_ui::mojom::ChatUIPage> page) override;
  void GetCompletions(const std::string& input) override;
  void GetConversationHistory(GetConversationHistoryCallback callback) override;
  void GetSummary() override;

 private:
  void OnArticleSummaryResult(const std::u16string& result, bool is_from_cache);
  void MakeAPIRequestWithConversationHistoryUpdate(ConversationTurn turn);
  void OnAPIResponse(bool contains_summary,
                     const std::string& assistant_input,
                     bool success);

  // ChatTabHelper::Observer
  void OnPageChanged() override;

  // TabStripModelObserver
  void OnTabStripModelChanged(
      TabStripModel* tab_strip_model,
      const TabStripModelChange& change,
      const TabStripSelectionChange& selection) override;

  std::unique_ptr<chat_ui::ChatUIAPIRequest> api_helper_ = nullptr;

  mojo::Remote<chat_ui::mojom::ChatUIPage> page_;

  raw_ptr<ChatTabHelper> active_chat_tab_helper_ = nullptr;

  base::ScopedObservation<ChatTabHelper, ChatTabHelper::Observer>
      chat_tab_helper_observation_{this};
  mojo::Receiver<chat_ui::mojom::PageHandler> receiver_;
};

#endif  // BRAVE_BROWSER_UI_WEBUI_CHAT_UI_CHAT_UI_PAGE_HANDLER_H_
