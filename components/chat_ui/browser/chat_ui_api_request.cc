/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/chat_ui/browser/chat_ui_api_request.h"

#include <base/containers/flat_map.h>

#include <utility>

#include "base/debug/dump_without_crashing.h"
#include "base/json/json_writer.h"
#include "brave/components/chat_ui/browser/buildflags.h"
#include "brave/components/chat_ui/common/chat_ui_constants.h"
#include "net/traffic_annotation/network_traffic_annotation.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "url/gurl.h"

namespace {

net::NetworkTrafficAnnotationTag GetNetworkTrafficAnnotationTag() {
  return net::DefineNetworkTrafficAnnotation("chat_ui", R"(
      semantics {
        sender: "ChatUI"
        description:
          "This is used to communicate with our partner AI API"
          "on behalf of the user interacting with the ChatUI."
        trigger:
          "Triggered by user sending a prompt."
        data:
          "Will generate a text completion that attempts to match whatever context or pattern the user gave it"
        destination: WEBSITE
      }
      policy {
        cookies_allowed: NO
        policy_exception_justification:
          "Not implemented."
      }
    )");
}

GURL GetURLWithPath(const std::string& host, const std::string& path) {
  return GURL(std::string(url::kHttpsScheme) + "://" + host).Resolve(path);
}

std::string CreateJSONRequestBody(base::ValueView node) {
  std::string json;
  base::JSONWriter::Write(node, &json);
  return json;
}

}  // namespace

namespace chat_ui {

ChatUIAPIRequest::ChatUIAPIRequest(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory)
    : api_request_helper_(GetNetworkTrafficAnnotationTag(),
                          url_loader_factory) {}

ChatUIAPIRequest::~ChatUIAPIRequest() = default;

void ChatUIAPIRequest::QueryPrompt(ResponseCallback callback,
                                   const std::string& prompt) {
  auto internal_callback =
      base::BindOnce(&ChatUIAPIRequest::OnGetResponse,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));

  base::Value::Dict dict;
  dict.Set("prompt", prompt);
  dict.Set("max_tokens_to_sample", 200);
  dict.Set("temperature", 1);
  dict.Set("top_k", -1);
  dict.Set("top_p", 0.999);
  dict.Set("model", "claude-v1");
  dict.Set("stream", false);

  base::flat_map<std::string, std::string> headers;
  headers.emplace("x-api-key", BUILDFLAG_INTERNAL_BRAVE_AI_PARTNER_KEY());

  api_request_helper_.Request("POST", GetURLWithPath(kUrlBase, kCompletionPath),
                              CreateJSONRequestBody(dict), "application/json",
                              true, std::move(internal_callback), headers);
}

void ChatUIAPIRequest::OnGetResponse(
    ResponseCallback callback,
    api_request_helper::APIRequestResult result) {
  // NOTE: |api_request_helper_| uses JsonSanitizer to sanitize input made with
  // requests. |body| will be empty when the response from service is invalid
  // json.
  const bool success = result.response_code() == 200;
  std::move(callback).Run(result.body(), success);
}

}  // namespace chat_ui
