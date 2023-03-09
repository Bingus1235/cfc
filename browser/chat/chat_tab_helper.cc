/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/chat/chat_tab_helper.h"

#include <queue>
#include <utility>

#include "base/strings/strcat.h"
#include "brave/components/chat_ui/common/chat_ui_constants.h"
#include "ui/accessibility/ax_node.h"
#include "ui/accessibility/ax_tree.h"

namespace {
static const ax::mojom::Role kContentRoles[]{
    ax::mojom::Role::kHeading,
    ax::mojom::Role::kParagraph,
};

static const ax::mojom::Role kRolesToSkip[]{
    ax::mojom::Role::kAudio,
    ax::mojom::Role::kBanner,
    ax::mojom::Role::kButton,
    ax::mojom::Role::kComplementary,
    ax::mojom::Role::kContentInfo,
    ax::mojom::Role::kFooter,
    ax::mojom::Role::kFooterAsNonLandmark,
    ax::mojom::Role::kImage,
    ax::mojom::Role::kLabelText,
    ax::mojom::Role::kNavigation,
};

void GetContentRootNodes(const ui::AXNode* root,
                         std::vector<const ui::AXNode*>* content_root_nodes) {
  std::queue<const ui::AXNode*> queue;
  queue.push(root);
  while (!queue.empty()) {
    const ui::AXNode* node = queue.front();
    queue.pop();
    // If a main or article node is found, add it to the list of content root
    // nodes and continue. Do not explore children for nested article nodes.
    if (node->GetRole() == ax::mojom::Role::kMain ||
        node->GetRole() == ax::mojom::Role::kArticle) {
      content_root_nodes->push_back(node);
      continue;
    }
    for (auto iter = node->UnignoredChildrenBegin();
         iter != node->UnignoredChildrenEnd(); ++iter) {
      queue.push(iter.get());
    }
  }
}

void AddContentNodesToVector(const ui::AXNode* node,
                             std::vector<const ui::AXNode*>* content_nodes) {
  if (base::Contains(kContentRoles, node->GetRole())) {
    content_nodes->emplace_back(node);
    return;
  }
  if (base::Contains(kRolesToSkip, node->GetRole())) {
    return;
  }
  for (auto iter = node->UnignoredChildrenBegin();
       iter != node->UnignoredChildrenEnd(); ++iter) {
    AddContentNodesToVector(iter.get(), content_nodes);
  }
}

void AddTextNodesToVector(const ui::AXNode* node,
                          std::vector<std::u16string>* strings) {
  const ui::AXNodeData& node_data = node->data();

  if (node_data.role == ax::mojom::Role::kStaticText) {
    if (node_data.HasStringAttribute(ax::mojom::StringAttribute::kName)) {
      strings->emplace_back(
          node_data.GetString16Attribute(ax::mojom::StringAttribute::kName));
    }
    return;
  }

  for (const auto* child : node->children()) {
    AddTextNodesToVector(child, strings);
  }
}
}  // namespace

ChatTabHelper::ChatTabHelper(content::WebContents* web_contents)
    : content::WebContentsObserver(web_contents),
      content::WebContentsUserData<ChatTabHelper>(*web_contents) {}

ChatTabHelper::~ChatTabHelper() = default;

std::string ChatTabHelper::GetConversationHistoryAsString() {
  std::string history_text;

  for (ConversationTurn history : chat_history_) {
    history_text = base::StrCat({history_text,
                                 history.character_type == CharacterType::HUMAN
                                     ? chat_ui::kHumanPrompt
                                     : chat_ui::kAIPrompt,
                                 history.text, "\n"});
  }

  return history_text;
}

std::vector<ConversationTurn> ChatTabHelper::GetConversationHistory() {
  return chat_history_;
}

void ChatTabHelper::AddToConversationHistory(const ConversationTurn& turn) {
  chat_history_.push_back(turn);
}

void ChatTabHelper::GetArticleSummaryString(
    OnArticleSummaryCallback on_article_summary_callback) {
  if (!article_summary_.empty()) {
    VLOG(1) << __func__ << " Summary coming from cache: " << article_summary_;

    std::move(on_article_summary_callback).Run(article_summary_, true);
    return;
  }

  ui::AXTreeID tree_id = web_contents()->GetPrimaryMainFrame()->GetAXTreeID();
  content::RenderFrameHost* rfh =
      content::RenderFrameHost::FromAXTreeID(tree_id);

  rfh->GetMainFrame()->RequestAXTreeSnapshot(
      base::BindOnce(&ChatTabHelper::OnSnapshotFinished, base::Unretained(this),
                     std::move(on_article_summary_callback)),
      ui::AXMode::kWebContents, false, 5000, {});
}

void ChatTabHelper::SetArticleSummaryString(const std::u16string text) {
  article_summary_ = text;
}

void ChatTabHelper::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void ChatTabHelper::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

void ChatTabHelper::OnSnapshotFinished(
    OnArticleSummaryCallback on_article_summary_callback,
    const ui::AXTreeUpdate& snapshot) {
  ui::AXTree tree;
  if (!tree.Unserialize(snapshot)) {
    return;
  }

  // Start AX distillation process
  // Don't copy the tree, as it can be expensive.
  DistillViaAlgorithm(std::move(on_article_summary_callback), std::move(tree));
}

void ChatTabHelper::DistillViaAlgorithm(
    OnArticleSummaryCallback&& on_article_summary_callback,
    const ui::AXTree&& tree) {
  std::vector<const ui::AXNode*> content_root_nodes;
  std::vector<const ui::AXNode*> content_nodes;
  GetContentRootNodes(tree.root(), &content_root_nodes);

  for (const ui::AXNode* content_root_node : content_root_nodes) {
    AddContentNodesToVector(content_root_node, &content_nodes);
  }

  std::vector<std::u16string> text_node_contents;
  for (const ui::AXNode* content_node : content_nodes) {
    AddTextNodesToVector(content_node, &text_node_contents);
  }

  std::move(on_article_summary_callback)
      .Run(base::JoinString(text_node_contents, u" "), false);
}

void ChatTabHelper::PrimaryPageChanged(content::Page& page) {
  CleanUp();

  for (auto& obs : observers_) {
    obs.OnPageChanged();
  }
}

void ChatTabHelper::WebContentsDestroyed() {
  CleanUp();
}

void ChatTabHelper::CleanUp() {
  chat_history_.clear();
  article_summary_.clear();
}

WEB_CONTENTS_USER_DATA_KEY_IMPL(ChatTabHelper);
