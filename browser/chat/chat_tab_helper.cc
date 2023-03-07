/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/chat/chat_tab_helper.h"

#include <utility>

ChatTabHelper::~ChatTabHelper() = default;

ChatTabHelper::ChatTabHelper(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      content::WebContentsUserData<ChatTabHelper>(*web_contents) {}

void ChatTabHelper::AddToConversationHistory(const ConversationTurn& turn) {
  chat_history_.push_back(turn);
}

void ChatTabHelper::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ChatTabHelper::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void ChatTabHelper::PrimaryPageChanged(content::Page& page) {
  chat_history_.clear();

  for (auto& obs : observers_) {
    obs.OnPageChanged();
  }
}

void ChatTabHelper::WebContentsDestroyed() {
  chat_history_.clear();
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(ChatTabHelper);
