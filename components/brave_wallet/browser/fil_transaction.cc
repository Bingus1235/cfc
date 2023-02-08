/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/fil_transaction.h"

#include <algorithm>
#include <string>
#include <utility>

#include "base/json/json_writer.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "brave/components/filecoin/rs/src/lib.rs.h"
#include "brave/components/json/rs/src/lib.rs.h"

namespace brave_wallet {

namespace {
// https://github.com/filecoin-project/go-state-types/blob/95828685f9df463f052a5d42b8f6c2502f873ceb/crypto/signature.go#L17
// https://spec.filecoin.io/algorithms/crypto/signatures/#section-algorithms.crypto.signatures.signature-types
enum SigType { ECDSASigType = 1, BLSSigType = 2 };

bool IsNumericString(const std::string& value) {
  return std::all_of(value.begin(), value.end(),
                     [](char c) { return std::isdigit(c) != 0; });
}

}  // namespace

FilTransaction::FilTransaction() = default;

FilTransaction::FilTransaction(const FilTransaction&) = default;
FilTransaction::FilTransaction(absl::optional<uint64_t> nonce,
                               const std::string& gas_premium,
                               const std::string& gas_fee_cap,
                               int64_t gas_limit,
                               const std::string& max_fee,
                               const FilAddress& to,
                               const FilAddress& from,
                               const std::string& value)
    : nonce_(nonce),
      gas_premium_(gas_premium),
      gas_fee_cap_(gas_fee_cap),
      gas_limit_(gas_limit),
      max_fee_(max_fee),
      to_(to),
      from_(from),
      value_(value) {}

FilTransaction::~FilTransaction() = default;

bool FilTransaction::IsEqual(const FilTransaction& tx) const {
  return nonce_ == tx.nonce_ && gas_premium_ == tx.gas_premium_ &&
         gas_fee_cap_ == tx.gas_fee_cap_ && gas_limit_ == tx.gas_limit_ &&
         max_fee_ == tx.max_fee_ && to_ == tx.to_ && from_ == tx.from_ &&
         value_ == tx.value_;
}

bool FilTransaction::operator==(const FilTransaction& other) const {
  return IsEqual(other);
}

bool FilTransaction::operator!=(const FilTransaction& other) const {
  return !IsEqual(other);
}

// static
absl::optional<FilTransaction> FilTransaction::FromTxData(
    const mojom::FilTxDataPtr& tx_data) {
  FilTransaction tx;
  uint64_t nonce = 0;
  if (!tx_data->nonce.empty() && base::StringToUint64(tx_data->nonce, &nonce)) {
    tx.nonce_ = nonce;
  }

  auto address = FilAddress::FromAddress(tx_data->to);
  if (address.IsEmpty())
    return absl::nullopt;
  tx.to_ = address;

  auto from = FilAddress::FromAddress(tx_data->from);
  if (from.IsEmpty())
    return absl::nullopt;
  tx.from_ = from;

  if (tx_data->value.empty() || !IsNumericString(tx_data->value))
    return absl::nullopt;
  tx.set_value(tx_data->value);

  if (!IsNumericString(tx_data->gas_fee_cap))
    return absl::nullopt;
  tx.set_fee_cap(tx_data->gas_fee_cap);

  if (!IsNumericString(tx_data->gas_premium))
    return absl::nullopt;
  tx.set_gas_premium(tx_data->gas_premium);

  if (!IsNumericString(tx_data->max_fee))
    return absl::nullopt;
  tx.set_max_fee(tx_data->max_fee);

  int64_t gas_limit = 0;
  if (!tx_data->gas_limit.empty()) {
    if (!base::StringToInt64(tx_data->gas_limit, &gas_limit))
      return absl::nullopt;
  }
  tx.set_gas_limit(gas_limit);

  return tx;
}

base::Value::Dict FilTransaction::ToValue() const {
  base::Value::Dict dict;
  dict.Set("Nonce", nonce_ ? base::NumberToString(nonce_.value()) : "");
  dict.Set("GasPremium", gas_premium_);
  dict.Set("GasFeeCap", gas_fee_cap_);
  dict.Set("MaxFee", max_fee_);
  dict.Set("GasLimit", base::NumberToString(gas_limit_));
  dict.Set("To", to_.EncodeAsString());
  dict.Set("From", from_.EncodeAsString());
  dict.Set("Value", value_);
  dict.Set("chain_id", chain_id);
 return dict;
}

// static
absl::optional<FilTransaction> FilTransaction::FromValue(
    const base::Value::Dict& value) {
  FilTransaction tx;
  const std::string* nonce_value = value.FindString("Nonce");
  if (!nonce_value)
    return absl::nullopt;

  if (!nonce_value->empty()) {
    uint64_t nonce = 0;
    if (!base::StringToUint64(*nonce_value, &nonce))
      return absl::nullopt;
    tx.nonce_ = nonce;
  }

  const std::string* gas_premium = value.FindString("GasPremium");
  if (!gas_premium)
    return absl::nullopt;
  tx.gas_premium_ = *gas_premium;

  const std::string* gas_fee_cap = value.FindString("GasFeeCap");
  if (!gas_fee_cap)
    return absl::nullopt;
  tx.gas_fee_cap_ = *gas_fee_cap;

  const std::string* max_fee = value.FindString("MaxFee");
  if (!max_fee)
    return absl::nullopt;
  tx.max_fee_ = *max_fee;

  const std::string* gas_limit = value.FindString("GasLimit");
  if (!gas_limit || !base::StringToInt64(*gas_limit, &tx.gas_limit_))
    return absl::nullopt;

  const std::string* from = value.FindString("From");
  if (!from)
    return absl::nullopt;
  tx.from_ = FilAddress::FromAddress(*from);

  const std::string* to = value.FindString("To");
  if (!to)
    return absl::nullopt;
  tx.to_ = FilAddress::FromAddress(*to);

  const std::string* tx_value = value.FindString("Value");
  if (!tx_value)
    return absl::nullopt;
  tx.value_ = *tx_value;

  const auto* chain_id_string = value.FindString("chain_id"); 
  if(chain_id_string){
    tx.set_chain_id(*chain_id_string);
  }

  return tx;
}

absl::optional<std::string> FilTransaction::GetMessageToSign() const {
  auto value = ToValue();
  value.Remove("MaxFee");
  value.Set("Method", 0);
  value.Set("Version", 0);
  value.Set("Params", "");
  const std::string* nonce_value = value.FindString("Nonce");
  bool nonce_empty = nonce_value && nonce_value->empty();
  // Nonce is empty usually for first transactions. We set it to 0 and skip
  // conversion bellow to get correct signature.
  if (nonce_empty) {
    value.Set("Nonce", 0);
  }
  std::string json;
  base::JSONWriter::Write(value, &json);

  std::string converted_json =
      json::convert_string_value_to_int64("/GasLimit", json.c_str(), false)
          .c_str();
  if (converted_json.empty())
    return absl::nullopt;
  if (!nonce_empty) {
    converted_json = json::convert_string_value_to_uint64(
                         "/Nonce", converted_json.c_str(), false)
                         .c_str();
  }
  return converted_json;
}

// https://spec.filecoin.io/algorithms/crypto/signatures/#section-algorithms.crypto.signatures
absl::optional<std::string> FilTransaction::GetSignedTransaction(
    const std::vector<uint8_t>& private_key) const {
  auto message = GetMessageToSign();
  if (!message)
    return absl::nullopt;
  base::Value::Dict signature;
  {
    std::string data(filecoin::transaction_sign(
        *message,
        rust::Slice<const uint8_t>{private_key.data(), private_key.size()}));
    if (data.empty())
      return absl::nullopt;
    signature.Set("Data", data);
  }
  // Set signature type based on protocol.
  // https://spec.filecoin.io/algorithms/crypto/signatures/#section-algorithms.crypto.signatures.signature-types
  auto sig_type = from().protocol() == mojom::FilecoinAddressProtocol::SECP256K1
                      ? SigType::ECDSASigType
                      : SigType::BLSSigType;
  signature.Set("Type", sig_type);
  base::Value::Dict dict;
  dict.Set("Message", "{message}");
  dict.Set("Signature", std::move(signature));
  std::string json;
  if (!base::JSONWriter::Write(dict, &json))
    return absl::nullopt;
  base::ReplaceFirstSubstringAfterOffset(&json, 0, "\"{message}\"", *message);
  return json;
}

mojom::FilTxDataPtr FilTransaction::ToFilTxData() const {
  return mojom::FilTxData::New(
      nonce() ? base::NumberToString(*nonce()) : "", gas_premium(),
      gas_fee_cap(), base::NumberToString(gas_limit()), max_fee(),
      to().EncodeAsString(), from().EncodeAsString(), value());
}

}  // namespace brave_wallet
