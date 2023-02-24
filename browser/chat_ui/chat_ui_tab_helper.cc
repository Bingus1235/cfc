/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/chat_ui/chat_ui_tab_helper.h"

#include <utility>

ChatUITabHelper::~ChatUITabHelper() = default;

ChatUITabHelper::ChatUITabHelper(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      content::WebContentsUserData<ChatUITabHelper>(*web_contents) {}

void ChatUITabHelper::AddToConversationHistory(const ConversationTurn& turn) {
  chat_history_.push_back(turn);
}

void ChatUITabHelper::DidStopLoading() {}

void ChatUITabHelper::WebContentsDestroyed() {}

WEB_CONTENTS_USER_DATA_KEY_IMPL(ChatUITabHelper);
