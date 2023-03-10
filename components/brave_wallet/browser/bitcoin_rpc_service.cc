/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/bitcoin_rpc_service.h"

#include <deque>

#include "base/check.h"
#include "base/functional/bind.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"
#include "base/notreached.h"
#include "base/ranges/algorithm.h"
#include "brave/components/brave_wallet/browser/keyring_service.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom-forward.h"
#include "components/prefs/pref_service.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"

#pragma clang optimize off

namespace {

net::NetworkTrafficAnnotationTag GetNetworkTrafficAnnotationTag() {
  return net::DefineNetworkTrafficAnnotation("bitcoin_rpc_service", R"(
      semantics {
        sender: "Bitcoin RPC Service"
        description:
          "This service is used to communicate with Bitcoin nodes "
          "on behalf of the user interacting with the native Brave wallet."
        trigger:
          "Triggered by uses of the native Brave wallet."
        data:
          "Bitcoin JSON RPC response bodies."
        destination: WEBSITE
      }
      policy {
        cookies_allowed: NO
        setting:
          "You can enable or disable this feature on chrome://flags."
        policy_exception_justification:
          "Not implemented."
      }
    )");
}

const GURL MakeListUtxoUrl(const GURL& base_url, const std::string& address) {
  GURL::Replacements replacements;
  const std::string path =
      base_url.path() + base::JoinString({"address", address, "utxo"}, "/");
  replacements.SetPathStr(path);

  return base_url.ReplaceComponents(replacements);
}

const GURL MakeFetchTransactionUrl(const GURL& base_url,
                                   const std::string& txid) {
  GURL::Replacements replacements;
  const std::string path =
      base_url.path() + base::JoinString({"tx", txid}, "/");
  replacements.SetPathStr(path);

  return base_url.ReplaceComponents(replacements);
}

GURL BaseRpcUrl(const std::string& keyring_id) {
  return GURL(keyring_id == brave_wallet::mojom::kBitcoinKeyringId
                  ? "https://blockstream.info/api/"
                  : "https://blockstream.info/testnet/api/");
}
}  // namespace

namespace brave_wallet {

namespace {
mojom::BitcoinUnspentOutputPtr BitcoinUnspentOutputFromValue(
    const base::Value::Dict* dict) {
  if (!dict) {
    return nullptr;
  }

  auto result = mojom::BitcoinUnspentOutput::New();
  if (auto* txid = dict->FindString("txid")) {
    result->txid = *txid;
  } else {
    return nullptr;
  }

  if (auto vout = dict->FindInt("vout")) {
    result->vout = *vout;
  } else {
    return nullptr;
  }

  // TOOD(apaymyshev): int64
  if (auto value = dict->FindInt("value")) {
    result->value = *value;
  } else {
    return nullptr;
  }

  return result;
}
}  // namespace

struct GetBitcoinAccountInfoContext
    : base::RefCounted<GetBitcoinAccountInfoContext> {
 public:
  GetBitcoinAccountInfoContext() = default;

  std::set<std::string> pending_addresses;
  mojom::BitcoinAccountInfoPtr account_info;
  mojom::BitcoinRpcService::GetBitcoinAccountInfoCallback callback;

 private:
  friend class base::RefCounted<GetBitcoinAccountInfoContext>;
  ~GetBitcoinAccountInfoContext() = default;
};

struct BitcoinInput {
  mojom::BitcoinUnspentOutputPtr unspent_output;
  base::Value transaction;
  std::string unlock_script;
};

struct SendToContext {
 public:
  std::string keyring_id;
  uint32_t account_index;
  std::string address_to;
  uint64_t amount;
  uint64_t fee;
  mojom::BitcoinAccountInfoPtr account_info;
  std::vector<BitcoinInput> inputs;
  mojom::BitcoinRpcService::SendToCallback callback;

  bool InputsPicked() { return inputs.empty(); }
  bool InputTransactionsReady() {
    return base::ranges::all_of(
        inputs, [](auto& input) { return input.transaction.is_dict(); });
  }
};

BitcoinRpcService::BitcoinRpcService(
    KeyringService* keyring_service,
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    PrefService* prefs,
    PrefService* local_state_prefs)
    : keyring_service_(keyring_service),
      api_request_helper_(new APIRequestHelper(GetNetworkTrafficAnnotationTag(),
                                               url_loader_factory)),
      prefs_(prefs),
      local_state_prefs_(local_state_prefs) {}

BitcoinRpcService::~BitcoinRpcService() = default;

mojo::PendingRemote<mojom::BitcoinRpcService> BitcoinRpcService::MakeRemote() {
  mojo::PendingRemote<mojom::BitcoinRpcService> remote;
  receivers_.Add(this, remote.InitWithNewPipeAndPassReceiver());
  return remote;
}

void BitcoinRpcService::Bind(
    mojo::PendingReceiver<mojom::BitcoinRpcService> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void BitcoinRpcService::GetUtxoList(const std::string& keyring_id,
                                    const std::string& address,
                                    GetUtxoListCallback callback) {
  if (!prefs_ || !local_state_prefs_) {
    return;
  }

  if (keyring_id != mojom::kBitcoinKeyringId &&
      keyring_id != mojom::kBitcoinTestnetKeyringId) {
    return;
  }

  GURL network_url = BaseRpcUrl(keyring_id);

  auto internal_callback =
      base::BindOnce(&BitcoinRpcService::OnGetUtxoList,
                     weak_ptr_factory_.GetWeakPtr(), std::move(callback));
  RequestInternal(true, MakeListUtxoUrl(network_url, address),
                  std::move(internal_callback));
}

void BitcoinRpcService::GetBitcoinAccountInfo(
    const std::string& keyring_id,
    uint32_t account_index,
    GetBitcoinAccountInfoCallback callback) {
  if (keyring_id != mojom::kBitcoinKeyringId &&
      keyring_id != mojom::kBitcoinTestnetKeyringId) {
    std::move(callback).Run(nullptr);
    return;
  }

  auto addresses =
      keyring_service_->GetBitcoinAddressesSync(keyring_id, account_index);
  if (addresses.first.empty() || addresses.second.empty()) {
    NOTREACHED();
    std::move(callback).Run(nullptr);
    return;
  }

  auto context = base::MakeRefCounted<GetBitcoinAccountInfoContext>();
  context->account_info = mojom::BitcoinAccountInfo::New();

  for (auto& address : addresses.first) {
    auto& info = context->account_info->address_infos.emplace_back(
        mojom::BitcoinAddressInfo::New());
    info->address = address;
    info->change = false;

    context->pending_addresses.insert(address);
  }

  for (auto& address : addresses.second) {
    auto& info = context->account_info->address_infos.emplace_back(
        mojom::BitcoinAddressInfo::New());
    info->address = address;
    info->change = true;

    context->pending_addresses.insert(address);
  }

  context->callback = std::move(callback);

  for (auto& address : context->pending_addresses) {
    auto request_url = MakeListUtxoUrl(BaseRpcUrl(keyring_id), address);
    auto internal_callback =
        base::BindOnce(&BitcoinRpcService::OnGetUtxoListForBitcoinAccountInfo,
                       weak_ptr_factory_.GetWeakPtr(), context, address);
    RequestInternal(true, request_url, std::move(internal_callback));
  }
}

void BitcoinRpcService::SendTo(const std::string& keyring_id,
                               uint32_t account_index,
                               const std::string& address_to,
                               uint64_t amount,
                               uint64_t fee,
                               SendToCallback callback) {
  if (keyring_id != mojom::kBitcoinKeyringId &&
      keyring_id != mojom::kBitcoinTestnetKeyringId) {
    std::move(callback).Run("");
    return;
  }

  auto context = std::make_unique<SendToContext>();
  context->keyring_id = keyring_id;
  context->account_index = account_index;
  context->address_to = address_to;
  context->amount = amount;
  context->fee = fee;
  context->callback = std::move(callback);

  WorkOnSendTo(std::move(context));
}

void BitcoinRpcService::OnGetBitcoinAccountInfoForSendTo(
    std::unique_ptr<SendToContext> context,
    mojom::BitcoinAccountInfoPtr account_info) {
  if (!account_info) {
    std::move(context->callback).Run("");
    return;
  }
  context->account_info = std::move(account_info);
  WorkOnSendTo(std::move(context));
}

bool BitcoinRpcService::PickInputs(SendToContext& context) {
  uint64_t amount_picked = 0;
  bool done = false;

  // TODO(apaymyshev): needs something better than greedy strategy.
  for (auto& address_info : context.account_info->address_infos) {
    for (auto& utxo : address_info->utxo_list) {
      amount_picked += utxo->value;
      auto& input = context.inputs.emplace_back();
      input.unspent_output = utxo->Clone();
      if (amount_picked >= context.amount + context.fee) {
        done = true;
        break;
      }
    }
    if (done) {
      break;
    }
  }

  DCHECK(!context.inputs.empty());
  return done;
}

void BitcoinRpcService::FetchInputTransactions(
    std::unique_ptr<SendToContext> context) {
  DCHECK(!context->InputTransactionsReady());

  for (auto& input : context->inputs) {
    if (input.transaction.is_none()) {
      auto keyring_id = context->keyring_id;
      auto txid = input.unspent_output->txid;
      FetchTransaction(
          keyring_id, txid,
          base::BindOnce(&BitcoinRpcService::OnFetchTransactionForSendTo,
                         weak_ptr_factory_.GetWeakPtr(), txid,
                         std::move(context)));
      return;
    }
  }

  NOTREACHED();
}

void BitcoinRpcService::OnFetchTransactionForSendTo(
    std::unique_ptr<SendToContext> context,
    std::string txid,
    base::Value transaction) {
  if (transaction.is_none()) {
    std::move(context->callback).Run("");
    return;
  }

  for (auto& input : context->inputs) {
    if (input.unspent_output->txid == txid) {
      input.transaction = transaction.Clone();
    }
  }

  WorkOnSendTo(std::move(context));
}

void BitcoinRpcService::WorkOnSendTo(std::unique_ptr<SendToContext> context) {
  DCHECK(context);
  if (context->account_info) {
    GetBitcoinAccountInfo(
        context->keyring_id, context->account_index,
        base::BindOnce(&BitcoinRpcService::OnGetBitcoinAccountInfoForSendTo,
                       weak_ptr_factory_.GetWeakPtr(), std::move(context)));
    return;
  }

  if (!context->InputsPicked()) {
    if (!PickInputs(*context)) {
      std::move(context->callback).Run("");
      return;
    }
  }

  if (!context->InputTransactionsReady()) {
    FetchInputTransactions(std::move(context));
    return;
  }
}

// void BitcoinRpcService::ContinueSendTo(
//     std::unique_ptr<SendToContext> context,
//     mojom::BitcoinAccountInfoPtr account_info) {
//   if (!account_info) {
//     std::move(context->callback).Run("");
//     return;
//   }
// }

void BitcoinRpcService::OnGetUtxoListForBitcoinAccountInfo(
    scoped_refptr<GetBitcoinAccountInfoContext> context,
    std::string requested_address,
    APIRequestResult api_request_result) {
  LOG(ERROR) << api_request_result.body();

  if (!context->callback) {
    return;
  }

  if (!api_request_result.Is2XXResponseCode()) {
    std::move(context->callback).Run(nullptr);
    return;
  }

  if (!api_request_result.value_body().is_list()) {
    std::move(context->callback).Run(nullptr);
    return;
  }

  mojom::BitcoinAddressInfo* address_info = nullptr;
  for (auto& address : context->account_info->address_infos) {
    if (address->address == requested_address) {
      address_info = address.get();
      break;
    }
  }

  if (!address_info) {
    NOTREACHED();
    std::move(context->callback).Run(nullptr);
    return;
  }

  std::vector<mojom::BitcoinUnspentOutputPtr> utxo_list;
  for (auto& item : api_request_result.value_body().GetList()) {
    auto output = BitcoinUnspentOutputFromValue(item.GetIfDict());
    if (!output) {
      std::move(context->callback).Run(nullptr);
      return;
    }

    address_info->utxo_list.push_back(std::move(output));
  }

  context->pending_addresses.erase(requested_address);

  if (context->pending_addresses.empty()) {
    for (auto& address : context->account_info->address_infos) {
      for (auto& utxo : address->utxo_list) {
        address->balance += utxo->value;
      }
      context->account_info->balance += address->balance;
    }

    std::move(context->callback).Run(std::move(context->account_info));
  }
}

void BitcoinRpcService::OnGetUtxoList(GetUtxoListCallback callback,
                                      APIRequestResult api_request_result) {
  LOG(ERROR) << api_request_result.body();
  std::move(callback).Run(api_request_result.body(), "");
}

// void BitcoinRpcService::GetTransaction(
//     const std::string& keyring_id,
//     const std::string& txid,
//     base::OnceCallback<void(base::Value)> callback) {
//   if (transactions_cache_.contains(txid)) {
//     std::move(callback).Run(transactions_cache_.at(txid));
//     return;
//   }

//   FetchTransaction(keyring_id, txid,
//   base::BindOnce(&BitcoinRpcService::OnGetTransaction,
//                      weak_ptr_factory_.GetWeakPtr(), std::move(callback))

//   GURL network_url = BaseRpcUrl(keyring_id);

//   auto internal_callback =
//       base::BindOnce(&BitcoinRpcService::OnFetchTransaction,
//                      weak_ptr_factory_.GetWeakPtr(), std::move(callback));
//   RequestInternal(true, MakeFetchTransactionUrl(network_url, txid),
//                   std::move(internal_callback));
// }

// void BitcoinRpcService::OnGetTransaction(
//     base::OnceCallback<void(base::Value)> callback,
//     base::Value transaction) {}

void BitcoinRpcService::FetchTransaction(
    const std::string& keyring_id,
    const std::string& txid,
    base::OnceCallback<void(base::Value)> callback) {
  if (transactions_cache_.contains(txid)) {
    std::move(callback).Run(transactions_cache_.at(txid).Clone());
    return;
  }

  GURL network_url = BaseRpcUrl(keyring_id);

  auto internal_callback =
      base::BindOnce(&BitcoinRpcService::OnFetchTransaction,
                     weak_ptr_factory_.GetWeakPtr(), txid, std::move(callback));
  RequestInternal(true, MakeFetchTransactionUrl(network_url, txid),
                  std::move(internal_callback));
}

void BitcoinRpcService::OnFetchTransaction(
    const std::string& txid,
    base::OnceCallback<void(base::Value)> callback,
    APIRequestResult api_request_result) {
  if (!api_request_result.Is2XXResponseCode()) {
    std::move(callback).Run(base::Value());
    return;
  }

  if (api_request_result.value_body().is_dict()) {
    transactions_cache_[txid] = api_request_result.value_body().Clone();
    std::move(callback).Run(api_request_result.value_body().Clone());
  } else {
    std::move(callback).Run(base::Value());
  }
}

void BitcoinRpcService::RequestInternal(
    bool auto_retry_on_network_change,
    const GURL& request_url,
    RequestIntermediateCallback callback,
    APIRequestHelper::ResponseConversionCallback conversion_callback) {
  DCHECK(request_url.is_valid());

  api_request_helper_->Request(
      "GET", request_url, "", "", auto_retry_on_network_change,
      std::move(callback), {}, -1u, std::move(conversion_callback));
}

}  // namespace brave_wallet
