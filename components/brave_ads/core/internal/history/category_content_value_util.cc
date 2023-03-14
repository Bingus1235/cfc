/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/history/category_content_value_util.h"

#include <string>

#include "brave/components/brave_ads/core/category_content_info.h"

namespace brave_ads {

namespace {

constexpr char kCategoryKey[] = "category";
constexpr char kOptActionKey[] = "optAction";

constexpr char kLegacyOptActionKey[] = "opt_action";

}  // namespace

base::Value::Dict CategoryContentToValue(
    const CategoryContentInfo& category_content) {
  base::Value::Dict dict;

  dict.Set(kCategoryKey, category_content.category);
  dict.Set(kOptActionKey, static_cast<int>(category_content.opt_action_type));

  return dict;
}

CategoryContentInfo CategoryContentFromValue(const base::Value::Dict& root) {
  CategoryContentInfo category_content;

  if (const std::string* value = root.FindString(kCategoryKey)) {
    category_content.category = *value;
  }

  if (const auto value = root.FindInt(kOptActionKey)) {
    category_content.opt_action_type =
        static_cast<CategoryContentOptActionType>(*value);
  } else if (const auto legacy_value = root.FindInt(kLegacyOptActionKey)) {
    category_content.opt_action_type =
        static_cast<CategoryContentOptActionType>(*legacy_value);
  }

  return category_content;
}

}  // namespace brave_ads
