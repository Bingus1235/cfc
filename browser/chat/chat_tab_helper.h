/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_CHAT_CHAT_TAB_HELPER_H_
#define BRAVE_BROWSER_CHAT_CHAT_TAB_HELPER_H_

#include <memory>
#include <string>
#include <vector>

#include "brave/components/chat_ui/common/chat_ui.mojom.h"
#include "content/public/browser/web_contents_observer.h"
#include "content/public/browser/web_contents_user_data.h"

using chat_ui::mojom::CharacterType;
using chat_ui::mojom::ConversationTurn;

class ChatTabHelper : public content::WebContentsObserver,
                      public content::WebContentsUserData<ChatTabHelper> {
 public:
  ChatTabHelper(const ChatTabHelper&) = delete;
  ChatTabHelper& operator=(const ChatTabHelper&) = delete;
  ~ChatTabHelper() override;

  std::vector<ConversationTurn> GetConversationHistory() {
    return chat_history_;
  }
  void AddToConversationHistory(const ConversationTurn& turn);

 private:
  friend class content::WebContentsUserData<ChatTabHelper>;

  explicit ChatTabHelper(content::WebContents* web_contents);

  // content::WebContentsObserver
  void DidStopLoading() override;
  void WebContentsDestroyed() override;

  std::vector<ConversationTurn> chat_history_;

  WEB_CONTENTS_USER_DATA_KEY_DECL();
};

#endif  // BRAVE_BROWSER_CHAT_CHAT_TAB_HELPER_H_
