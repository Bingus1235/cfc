/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/asset_discovery_manager.h"

#include <map>
#include <utility>

#include "base/base64.h"
#include "base/strings/strcat.h"
#include "base/strings/stringprintf.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "brave/components/brave_wallet/browser/blockchain_registry.h"
#include "brave/components/brave_wallet/browser/brave_wallet_constants.h"
#include "brave/components/brave_wallet/browser/brave_wallet_service.h"
#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"
#include "brave/components/brave_wallet/browser/eth_topics_builder.h"
#include "brave/components/brave_wallet/browser/json_rpc_service.h"
#include "brave/components/brave_wallet/browser/keyring_service.h"
#include "brave/components/brave_wallet/browser/pref_names.h"
#include "brave/components/brave_wallet/common/eth_address.h"
#include "brave/components/brave_wallet/common/hex_utils.h"
#include "brave/components/brave_wallet/common/solana_utils.h"
#include "brave/components/brave_wallet/common/string_utils.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "net/base/url_util.h"
#include "services/network/public/cpp/shared_url_loader_factory.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

net::NetworkTrafficAnnotationTag GetNetworkTrafficAnnotationTag() {
  return net::DefineNetworkTrafficAnnotation("asset_discovery_manager", R"(
      semantics {
        sender: "Asset Discovery Manager"
        description:
          "This service is used to discover crypto assets"on behalf "
          "of the user interacting with the native Brave wallet."
        trigger:
          "Triggered by uses of the native Brave wallet."
        data:
          "NFT assets."
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

}  // namespace

namespace brave_wallet {

AssetDiscoveryManager::AssetDiscoveryManager(
    scoped_refptr<network::SharedURLLoaderFactory> url_loader_factory,
    BraveWalletService* wallet_service,
    JsonRpcService* json_rpc_service,
    KeyringService* keyring_service,
    PrefService* prefs)
    : api_request_helper_(new APIRequestHelper(GetNetworkTrafficAnnotationTag(),
                                               url_loader_factory)),
      wallet_service_(wallet_service),
      json_rpc_service_(json_rpc_service),
      keyring_service_(keyring_service),
      prefs_(prefs),
      weak_ptr_factory_(this) {
  keyring_service_->AddObserver(
      keyring_service_observer_receiver_.BindNewPipeAndPassRemote());
}

AssetDiscoveryManager::~AssetDiscoveryManager() = default;

const std::vector<std::string>&
AssetDiscoveryManager::GetAssetDiscoverySupportedEthChains() {
  if (supported_chains_for_testing_.size() > 0) {
    return supported_chains_for_testing_;
  }

  // Use the hardcoded list of BalanceScanner contract addresses to determine
  // which chains are supported.
  static const base::NoDestructor<std::vector<std::string>>
      asset_discovery_supported_chains([] {
        std::vector<std::string> supported_chains;
        for (const auto& entry : GetEthBalanceScannerContractAddresses()) {
          supported_chains.push_back(entry.first);
        }
        return supported_chains;
      }());
  return *asset_discovery_supported_chains;
}

void AssetDiscoveryManager::DiscoverSolAssets(
    const std::vector<std::string>& account_addresses,
    bool triggered_by_accounts_added) {
  // Convert each account address to SolanaAddress and check validity
  std::vector<SolanaAddress> solana_addresses;
  for (const auto& address : account_addresses) {
    absl::optional<SolanaAddress> solana_address =
        SolanaAddress::FromBase58(address);
    if (!solana_address.has_value()) {
      continue;
    }

    if ((*solana_address).IsValid()) {
      solana_addresses.push_back(*solana_address);
    }
  }

  if (solana_addresses.empty()) {
    CompleteDiscoverAssets(std::vector<mojom::BlockchainTokenPtr>(),
                           triggered_by_accounts_added);
    return;
  }

  // TODO(nvonpentz): When custom networks are supported, we need to check
  // that the active network is our own that supports this RPC call.

  const auto barrier_callback =
      base::BarrierCallback<std::vector<SolanaAddress>>(
          solana_addresses.size(),
          base::BindOnce(&AssetDiscoveryManager::MergeDiscoveredSolanaAssets,
                         weak_ptr_factory_.GetWeakPtr(),
                         triggered_by_accounts_added));
  for (const auto& account_address : solana_addresses) {
    json_rpc_service_->GetSolanaTokenAccountsByOwner(
        account_address,
        base::BindOnce(&AssetDiscoveryManager::OnGetSolanaTokenAccountsByOwner,
                       weak_ptr_factory_.GetWeakPtr(),
                       std::move(barrier_callback)));
  }
}

void AssetDiscoveryManager::OnGetSolanaTokenAccountsByOwner(
    base::OnceCallback<void(std::vector<SolanaAddress>)> barrier_callback,
    const std::vector<SolanaAccountInfo>& token_accounts,
    mojom::SolanaProviderError error,
    const std::string& error_message) {
  if (error != mojom::SolanaProviderError::kSuccess || token_accounts.empty()) {
    std::move(barrier_callback).Run(std::vector<SolanaAddress>());
    return;
  }

  // Add each token account to the all_discovered_contract_addresses list
  std::vector<SolanaAddress> discovered_mint_addresses;
  for (const auto& token_account : token_accounts) {
    // Decode Base64
    const absl::optional<std::vector<uint8_t>> data =
        base::Base64Decode(token_account.data);
    if (data.has_value()) {
      // Decode the address
      const absl::optional<SolanaAddress> mint_address =
          DecodeMintAddress(data.value());
      if (mint_address.has_value()) {
        // Add the contract address to the list
        discovered_mint_addresses.push_back(mint_address.value());
      }
    }
  }

  std::move(barrier_callback).Run(discovered_mint_addresses);
}

void AssetDiscoveryManager::MergeDiscoveredSolanaAssets(
    bool triggered_by_accounts_added,
    const std::vector<std::vector<SolanaAddress>>&
        all_discovered_contract_addresses) {
  // Create vector of all discovered contract addresses
  std::vector<std::string> discovered_mint_addresses;
  for (const auto& discovered_contract_address_list :
       all_discovered_contract_addresses) {
    for (const auto& discovered_contract_address :
         discovered_contract_address_list) {
      discovered_mint_addresses.push_back(
          discovered_contract_address.ToBase58());
    }
  }

  // Convert vector to flat_set
  base::flat_set<std::string> discovered_mint_addresses_set(
      std::move(discovered_mint_addresses));

  auto internal_callback = base::BindOnce(
      &AssetDiscoveryManager::OnGetSolanaTokenRegistry,
      weak_ptr_factory_.GetWeakPtr(), triggered_by_accounts_added,
      std::move(discovered_mint_addresses_set));

  // Fetch SOL registry tokens
  BlockchainRegistry::GetInstance()->GetAllTokens(mojom::kSolanaMainnet,
                                                  mojom::CoinType::SOL,
                                                  std::move(internal_callback));
}

void AssetDiscoveryManager::OnGetSolanaTokenRegistry(
    bool triggered_by_accounts_added,
    const base::flat_set<std::string>& discovered_mint_addresses,
    std::vector<mojom::BlockchainTokenPtr> sol_token_registry) {
  std::vector<mojom::BlockchainTokenPtr> discovered_tokens;
  for (const auto& token : sol_token_registry) {
    if (discovered_mint_addresses.contains(token->contract_address)) {
      if (!BraveWalletService::AddUserAsset(token.Clone(), prefs_)) {
        continue;
      }
      discovered_tokens.push_back(token.Clone());
    }
  }

  CompleteDiscoverAssets(std::move(discovered_tokens),
                         triggered_by_accounts_added);
}

void AssetDiscoveryManager::DiscoverEthAssets(
    const std::vector<std::string>& account_addresses,
    bool triggered_by_accounts_added) {
  if (account_addresses.empty()) {
    CompleteDiscoverAssets({}, triggered_by_accounts_added);
    return;
  }

  std::vector<mojom::BlockchainTokenPtr> user_assets =
      BraveWalletService::GetUserAssets(prefs_);
  TokenListMap token_list_map =
      BlockchainRegistry::GetInstance()->GetEthTokenListMap(
          GetAssetDiscoverySupportedEthChains());

  // Create set of all user assets per chain to use to ensure we don't
  // include assets the user has already added in the call to the BalanceScanner
  base::flat_map<std::string, base::flat_set<base::StringPiece>>
      user_assets_per_chain;
  for (const auto& user_asset : user_assets) {
    user_assets_per_chain[user_asset->chain_id].insert(
        user_asset->contract_address);
  }

  // Create a map of chain_id to a map contract address to BlockchainToken
  // to easily lookup tokens by contract address when the results of the
  // BalanceScanner calls are merged
  base::flat_map<std::string,
                 base::flat_map<std::string, mojom::BlockchainTokenPtr>>
      chain_id_to_contract_address_to_token;

  // Create a map of chain_id to a vector of contract addresses (strings, rather
  // than BlockchainTokens) to pass to GetERC20TokenBalances
  base::flat_map<std::string, std::vector<std::string>>
      chain_id_to_contract_addresses;

  // Populate the chain_id_to_contract_addresses using the token_list_map of
  // BlockchainTokenPtrs
  for (auto& [chain_id, token_list] : token_list_map) {
    for (auto& token : token_list) {
      if (!user_assets_per_chain[chain_id].contains(token->contract_address)) {
        chain_id_to_contract_addresses[chain_id].push_back(
            token->contract_address);
        chain_id_to_contract_address_to_token[chain_id]
                                             [token->contract_address] =
                                                 std::move(token);
      }
    }
  }

  // Use a barrier callback to wait for all GetERC20TokenBalances calls to
  // complete (one for each account address).
  const auto barrier_callback =
      base::BarrierCallback<std::map<std::string, std::vector<std::string>>>(
          account_addresses.size() * chain_id_to_contract_addresses.size(),
          base::BindOnce(&AssetDiscoveryManager::MergeDiscoveredEthAssets,
                         weak_ptr_factory_.GetWeakPtr(),
                         std::move(chain_id_to_contract_address_to_token),
                         triggered_by_accounts_added));

  // For each account address, call GetERC20TokenBalances for each chain ID
  for (const auto& account_address : account_addresses) {
    for (const auto& [chain_id, contract_addresses] :
         chain_id_to_contract_addresses) {
      auto internal_callback = base::BindOnce(
          &AssetDiscoveryManager::OnGetERC20TokenBalances,
          weak_ptr_factory_.GetWeakPtr(), barrier_callback, chain_id,
          triggered_by_accounts_added, contract_addresses);
      json_rpc_service_->GetERC20TokenBalances(contract_addresses,
                                               account_address, chain_id,
                                               std::move(internal_callback));
    }
  }
}

void AssetDiscoveryManager::OnGetERC20TokenBalances(
    base::OnceCallback<void(std::map<std::string, std::vector<std::string>>)>
        barrier_callback,
    const std::string& chain_id,
    bool triggered_by_accounts_added,
    const std::vector<std::string>&
        contract_addresses,  // Contract addresses queried for
    std::vector<mojom::ERC20BalanceResultPtr> balance_results,
    mojom::ProviderError error,
    const std::string& error_message) {
  // If the request failed, return an empty map
  if (error != mojom::ProviderError::kSuccess || balance_results.empty()) {
    std::move(barrier_callback).Run({});
    return;
  }

  // Create a map of chain_id to a vector of contract addresses that have a
  // balance greater than 0
  std::map<std::string, std::vector<std::string>>
      chain_id_to_contract_addresses_with_balance;

  // Populate the map using the balance_results
  for (size_t i = 0; i < balance_results.size(); i++) {
    if (balance_results[i]->balance.has_value()) {
      uint256_t balance_uint;
      bool success =
          HexValueToUint256(balance_results[i]->balance.value(), &balance_uint);
      if (success && balance_uint > 0) {
        chain_id_to_contract_addresses_with_balance[chain_id].push_back(
            contract_addresses[i]);
      }
    }
  }

  std::move(barrier_callback).Run(chain_id_to_contract_addresses_with_balance);
}

void AssetDiscoveryManager::MergeDiscoveredEthAssets(
    base::flat_map<std::string,
                   base::flat_map<std::string, mojom::BlockchainTokenPtr>>
        chain_id_to_contract_address_to_token,
    bool triggered_by_accounts_added,
    const std::vector<std::map<std::string, std::vector<std::string>>>&
        discovered_assets_results) {
  // Create a vector of BlockchainTokenPtrs to return
  std::vector<mojom::BlockchainTokenPtr> discovered_tokens;

  // Keep track of which contract addresses have been seen per chain
  base::flat_map<std::string, base::flat_set<base::StringPiece>>
      seen_contract_addresses;
  for (const auto& discovered_assets_result : discovered_assets_results) {
    for (const auto& [chain_id, contract_addresses] :
         discovered_assets_result) {
      for (const auto& contract_address : contract_addresses) {
        // Skip if seen
        if (seen_contract_addresses[chain_id].contains(contract_address)) {
          continue;
        }

        // Add to seen and discovered_tokens if not
        seen_contract_addresses[chain_id].insert(contract_address);
        auto token = std::move(
            chain_id_to_contract_address_to_token[chain_id][contract_address]);
        if (token && BraveWalletService::AddUserAsset(token.Clone(), prefs_)) {
          discovered_tokens.push_back(std::move(token));
        }
      }
    }
  }

  CompleteDiscoverAssets(std::move(discovered_tokens),
                         triggered_by_accounts_added);
}

void AssetDiscoveryManager::DiscoverNFTsOnAllSupportedChains(
    std::map<mojom::CoinType, std::vector<std::string>>& account_addresses,
    bool triggered_by_accounts_added) {
  // Use a barrier callback to wait for all FetchNFTsFromSimpleHash calls to
  // complete (one for each account address).
  const auto& eth_account_addresses = account_addresses[mojom::CoinType::ETH];
  const auto& sol_account_addresses = account_addresses[mojom::CoinType::SOL];
  const auto barrier_callback =
      base::BarrierCallback<std::vector<mojom::BlockchainTokenPtr>>(
          eth_account_addresses.size() + sol_account_addresses.size(),
          base::BindOnce(&AssetDiscoveryManager::MergeDiscoveredNFTs,
                         weak_ptr_factory_.GetWeakPtr(),
                         triggered_by_accounts_added));
  for (const auto& account_address : eth_account_addresses) {
    FetchNFTsFromSimpleHash(account_address,
                            GetAssetDiscoverySupportedEthChains(),
                            mojom::CoinType::ETH, std::move(barrier_callback));
  }

  for (const auto& account_address : sol_account_addresses) {
    FetchNFTsFromSimpleHash(account_address, {mojom::kSolanaMainnet},
                            mojom::CoinType::SOL, std::move(barrier_callback));
  }
}

void AssetDiscoveryManager::MergeDiscoveredNFTs(
    bool triggered_by_accounts_added,
    const std::vector<std::vector<mojom::BlockchainTokenPtr>>& nfts) {
  // De-dupe the NFTs
  base::flat_set<mojom::BlockchainTokenPtr> seen_nft;
  std::vector<mojom::BlockchainTokenPtr> discovered_nfts;
  for (const auto& nft_list : nfts) {
    for (const auto& nft : nft_list) {
      if (seen_nft.contains(nft)) {
        continue;
      }
      seen_nft.insert(nft.Clone());

      // Add the NFT to the user's assets
      if (BraveWalletService::AddUserAsset(nft.Clone(), prefs_)) {
        discovered_nfts.push_back(nft.Clone());  // TODO(nvonpentz) don't clone
      }
    }
  }

  CompleteDiscoverAssets(std::move(discovered_nfts),
                         triggered_by_accounts_added);
}

// Calls
// https://api.simplehash.com/api/v0/nfts/owners?chains={chains}&wallet_addresses={wallet_addresses}
void AssetDiscoveryManager::FetchNFTsFromSimpleHash(
    const std::string& account_address,
    const std::vector<std::string>& chain_ids,
    mojom::CoinType coin,
    FetchNFTsFromSimpleHashCallback callback) {
  GURL url = GetSimpleHashNftsByWalletUrl(account_address, chain_ids);
  if (!url.is_valid()) {
    std::move(callback).Run({});
    return;
  }
  std::vector<mojom::BlockchainTokenPtr> nfts_so_far = {};
  auto internal_callback =
      base::BindOnce(&AssetDiscoveryManager::OnFetchNFTsFromSimpleHash,
                     weak_ptr_factory_.GetWeakPtr(), std::move(nfts_so_far),
                     coin, std::move(callback));

  api_request_helper_->Request("GET", url, "", "", true,
                               std::move(internal_callback));
}

void AssetDiscoveryManager::OnFetchNFTsFromSimpleHash(
    std::vector<mojom::BlockchainTokenPtr> nfts_so_far,
    mojom::CoinType coin,
    FetchNFTsFromSimpleHashCallback callback,
    APIRequestResult api_request_result) {
  if (!api_request_result.Is2XXResponseCode()) {
    std::move(callback).Run(std::move(nfts_so_far));
    return;
  }

  // Invalid JSON becomes an empty string after sanitization
  if (api_request_result.body().empty()) {
    std::move(callback).Run(std::move(nfts_so_far));
    return;
  }

  // optional of a pair of GURL and vector of blockchaintokenptrs
  absl::optional<std::pair<GURL, std::vector<mojom::BlockchainTokenPtr>>>
      result = ParseNFTsFromSimpleHash(api_request_result.value_body(), coin);
  if (!result) {
    std::move(callback).Run(std::move(nfts_so_far));
    return;
  }

  // Add discovered NFTs to nfts_so_far
  for (auto& token : result.value().second) {
    nfts_so_far.push_back(std::move(token));
  }

  // If there is a next page, fetch it
  if (result.value().first.is_valid()) {
    auto internal_callback =
        base::BindOnce(&AssetDiscoveryManager::OnFetchNFTsFromSimpleHash,
                       weak_ptr_factory_.GetWeakPtr(), std::move(nfts_so_far),
                       coin, std::move(callback));
    api_request_helper_->Request("GET", result.value().first, "", "", true,
                                 std::move(internal_callback));
    return;
  }

  // Otherwise, return the nfts_so_far
  std::move(callback).Run(std::move(nfts_so_far));
}

absl::optional<std::pair<GURL, std::vector<mojom::BlockchainTokenPtr>>>
AssetDiscoveryManager::ParseNFTsFromSimpleHash(const base::Value& json_value,
                                               mojom::CoinType coin) {
  // {
  //   "next":
  //   "https://api.simplehash.com/api/v0/nfts/owners?chains=polygon%2Cethereum&cursor=ZXZtLXBnLjB4YmZiMWU1NzZkZThjYTZkOTlhNzQ3MTIyMDhjMWZhODZiMzgyYzY0Yy4wMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDA0Mjg0NTg0MDg1MzdfMjAyMy0wMi0xMyAxMzozNjowMCswMDowMF9fbmV4dA&limit=1&wallet_addresses=0xB4B2802129071b2B9eBb8cBB01EA1E4D14B34961",
  //   "previous": null,
  //   "nfts": [
  //     {
  //       "nft_id":
  //       "polygon.0xbfb1e576de8ca6d99a74712208c1fa86b382c64c.428458408537",
  //       "chain": "polygon",
  //       "contract_address": "0xBfb1E576dE8ca6d99A74712208C1fA86b382c64c",
  //       "token_id": "428458408537",
  //       "name": "EtherMail.io - Web3 wallet email + ENS / UD token",
  //       "description": "\"Instead of needing to know your complete wallet
  //       address by heart, you can simply use your Unstoppable or ENS Web3
  //       domain. [Claim your Web3 email, while it's
  //       available!](https://ethermail.io/alias?afid=63dd31e0793cf5d1041b9a1f)\n---
  //       \n\n*💌 delivered by [walletads.io](https://www.walletads.io)*   \n*⚙️
  //       [unsubscribe](https://unsubscribe.walletads.io)*\"", "previews": {
  //         "image_small_url":
  //         "https://lh3.googleusercontent.com/4MYVH4KA2f_mOaEHWPWx9sLu_9g9IuEr3ITdZ08f6iIDH74VjzGLE-Hre248cRwUC6V9AA4K3d0AD8Eayq_F9IH8wqfY1QvD7DA=s250",
  //         "image_medium_url":
  //         "https://lh3.googleusercontent.com/4MYVH4KA2f_mOaEHWPWx9sLu_9g9IuEr3ITdZ08f6iIDH74VjzGLE-Hre248cRwUC6V9AA4K3d0AD8Eayq_F9IH8wqfY1QvD7DA",
  //         "image_large_url":
  //         "https://lh3.googleusercontent.com/4MYVH4KA2f_mOaEHWPWx9sLu_9g9IuEr3ITdZ08f6iIDH74VjzGLE-Hre248cRwUC6V9AA4K3d0AD8Eayq_F9IH8wqfY1QvD7DA=s1000",
  //         "image_opengraph_url":
  //         "https://lh3.googleusercontent.com/4MYVH4KA2f_mOaEHWPWx9sLu_9g9IuEr3ITdZ08f6iIDH74VjzGLE-Hre248cRwUC6V9AA4K3d0AD8Eayq_F9IH8wqfY1QvD7DA=k-w1200-s2400-rj",
  //         "blurhash": "UG5},po+N4WF%jbfRpoh%NohM}WCWBazWCof",
  //         "predominant_color": "#061033"
  //       },
  //       "image_url":
  //       "https://cdn.simplehash.com/assets/46772326f1eab2c765b2ebed4acf1c94f2e5fdbdd14eea8fb2b41af5b20521a3.gif",
  //       "image_properties": {
  //         "width": 500,
  //         "height": 500,
  //         "size": 4093120,
  //         "mime_type": "image/gif"
  //       },
  //       "video_url": null,
  //       "video_properties": null,
  //       "audio_url": null,
  //       "audio_properties": null,
  //       "model_url": null,
  //       "model_properties": null,
  //       "background_color": null,
  //       "external_url": null,
  //       "created_date": "2023-02-10T09:43:42",
  //       "status": "minted",
  //       "token_count": 19995,
  //       "owner_count": 17262,
  //       "owners": [ { "owner_address":
  //       "0xFF0EC440231Ef0cBbfe8e35E655ACE8112C3C575", "quantity": 2734,
  //       "first_acquired_date": "2023-02-10T09:43:42", "last_acquired_date":
  //       "2023-02-10T09:43:42" }], "last_sale": null, "first_created": {
  //         "minted_to": "0xff0ec440231ef0cbbfe8e35e655ace8112c3c575",
  //         "quantity": 20000,
  //         "timestamp": "2023-02-10T09:43:42",
  //         "block_number": 39129989,
  //         "transaction":
  //         "0xccaecd628480662848d20eb2af48132b17b1165eaf7cb7f5400cc0b80ca9f453",
  //         "transaction_initiator":
  //         "0xf58fb3aa94d603062daec9dd3c1d8fd8b8d0db77"
  //       },
  //       "contract": {
  //         "type": "ERC1155",
  //         "name": "ethermail.io",
  //         "symbol": "ETHMAIL",
  //         "deployed_by": "0xf58fb3aa94d603062daec9dd3c1d8fd8b8d0db77",
  //         "deployed_via_contract": null
  //       },
  //       "collection": {
  //         "collection_id": "f2526d49315d02894f4107daa36db47a",
  //         "name": "Attention UD & ENS holders: announcing the ethermail.io
  //         alias!", "description": "Attention UD & ENS Holders! 10m EMCs
  //         raffled and airdropped upon signing up with EtherMail. Turn
  //         0x3f...01b@ethermail.io into marc.eth@ethermail.io. [Claim your
  //         Web3 email, while it's
  //         available!](https://ethermail.io/alias?afid=63dd31e0793cf5d1041b9a1f)",
  //         "image_url":
  //         "https://lh3.googleusercontent.com/8NdkjxUAHEjW34DwdJiXFKciqwqeZWs2G0ocGWL6OalMPeykHdy0OFQnCPBEvSf0NTLEAHqAx4lsme87FjL4V6g0owAJPNvNhA",
  //         "banner_image_url": null,
  //         "external_url": null,
  //         "twitter_username": null,
  //         "discord_url": null,
  //         "marketplace_pages": [
  //           {
  //             "marketplace_id": "opensea",
  //             "marketplace_name": "OpenSea",
  //             "marketplace_collection_id":
  //             "attention-ud-ens-holders-announcing-the-ethermail", "nft_url":
  //             "https://opensea.io/assets/matic/0xbfb1e576de8ca6d99a74712208c1fa86b382c64c/428458408537",
  //             "collection_url":
  //             "https://opensea.io/collection/attention-ud-ens-holders-announcing-the-ethermail",
  //             "verified": false
  //           }
  //         ],
  //         "metaplex_mint": null,
  //         "metaplex_first_verified_creator": null,
  //         "spam_score": 25,
  //         "floor_prices": [
  //           {
  //             "marketplace_id": "opensea",
  //             "marketplace_name": "OpenSea",
  //             "value": 812761433454256,
  //             "payment_token": {
  //               "payment_token_id":
  //               "polygon.0x7ceb23fd6bc0add59e62ac25578270cff1b9f619", "name":
  //               "Wrapped Ether", "symbol": "WETH", "address":
  //               "0x7ceB23fD6bC0adD59E62ac25578270cFf1b9f619", "decimals": 18
  //             }
  //           }
  //         ],
  //         "distinct_owner_count": 18166,
  //         "distinct_nft_count": 2,
  //         "total_quantity": 29994,
  //         "top_contracts": [
  //           "polygon.0xbfb1e576de8ca6d99a74712208c1fa86b382c64c"
  //         ]
  //       },
  //       "rarity": {
  //         "rank": null,
  //         "score": null,
  //         "unique_attributes": null
  //       },
  //       "extra_metadata": {
  //         "attributes": [],
  //         "__image_id": "127",
  //         "image_original_url":
  //         "ipfs://QmeCJnLGypZ8aqyCSH9jtxrKYzfpNdHdRydwVz9Zg9uZLh",
  //         "animation_original_url": null,
  //         "metadata_original_url":
  //         "ipfs://QmbDck97MGAK9ea7fvRTPo3dXXaZckqrfZBU2ycoeTDBaQ"
  //       }
  //     }
  //   ]
  // }

  if (!json_value.is_dict()) {
    return absl::nullopt;
  }

  // FetchNFTsFromSimpleHashResult result;
  GURL nextURL;
  auto* next = json_value.FindStringKey("next");
  if (next) {
    nextURL = GURL(*next);
  }

  const base::Value* nfts = json_value.FindKey("nfts");
  if (!nfts || !nfts->is_list()) {
    return absl::nullopt;
  }

  const base::flat_map<std::string, std::string>& chain_ids =
      FromSimpleHashChainId();
  std::vector<mojom::BlockchainTokenPtr> nft_tokens;
  for (const auto& nft : nfts->GetList()) {
    auto token = mojom::BlockchainToken::New();

    // chain_id
    auto* chain = nft.FindStringKey("chain");
    if (!chain || !chain_ids.contains(*chain)) {
      continue;
    }
    token->chain_id = chain_ids.at(*chain);

    // contract_address
    auto* contract_address = nft.FindStringKey("contract_address");
    if (!contract_address) {
      continue;
    }
    token->contract_address = *contract_address;

    // token_id
    auto* token_id = nft.FindStringKey("token_id");
    if (!token_id) {
      continue;
    }
    uint256_t token_id_uint256;
    if (!Base10ValueToUint256(*token_id, &token_id_uint256)) {
      continue;
    }
    token->token_id = Uint256ValueToHex(token_id_uint256);

    // is_erc721, is_erc20, is_nft
    auto* contract = nft.FindDictKey("contract");
    if (!contract) {
      continue;
    }
    auto* type = contract->FindStringKey("type");
    if (!type) {
      continue;
    }
    bool is_erc721 = base::EqualsCaseInsensitiveASCII(*type, "ERC721");
    if (!is_erc721) {
      continue;
    }
    token->is_erc721 = true;
    token->is_nft = true;
    token->coin = coin;

    nft_tokens.push_back(std::move(token));
  }

  return std::make_pair(nextURL, std::move(nft_tokens));
}

// Called when asset discovery has completed for
void AssetDiscoveryManager::CompleteDiscoverAssets(
    std::vector<mojom::BlockchainTokenPtr> discovered_assets_for_bucket,
    bool triggered_by_accounts_added) {
  if (discover_assets_completed_callback_for_testing_) {
    std::vector<mojom::BlockchainTokenPtr> discovered_assets_for_bucket_clone;
    for (const auto& asset : discovered_assets_for_bucket) {
      discovered_assets_for_bucket_clone.push_back(asset.Clone());
    }
    discover_assets_completed_callback_for_testing_.Run(
        std::move(discovered_assets_for_bucket_clone));
  }

  // Do not emit event or modify remaining_buckets_ count if
  // call was triggered by an AccountsAdded event
  if (triggered_by_accounts_added) {
    return;
  }

  // Complete the call by decrementing remaining_buckets_, storing the
  // discovered assets for later, and emitting the event if this was the final
  // chain to finish
  remaining_buckets_--;
  for (auto& asset : discovered_assets_for_bucket) {
    discovered_assets_.push_back(std::move(asset));
  }

  if (remaining_buckets_ == 0) {
    wallet_service_->OnDiscoverAssetsCompleted(std::move(discovered_assets_));
    discovered_assets_.clear();
  }
}

void AssetDiscoveryManager::DiscoverAssetsOnAllSupportedChainsAccountsAdded(
    mojom::CoinType coin,
    const std::vector<std::string>& account_addresses) {
  if (coin == mojom::CoinType::ETH) {
    DiscoverEthAssets(account_addresses, true);
  } else if (coin == mojom::CoinType::SOL) {
    DiscoverSolAssets(account_addresses, true);
  }
}

void AssetDiscoveryManager::DiscoverAssetsOnAllSupportedChainsRefresh(
    std::map<mojom::CoinType, std::vector<std::string>>& account_addresses) {
  // Simple client side rate limiting (only applies to refreshes)
  const base::Time assets_last_discovered_at =
      prefs_->GetTime(kBraveWalletLastDiscoveredAssetsAt);
  if (!assets_last_discovered_at.is_null() &&
      ((base::Time::Now() - base::Minutes(kAssetDiscoveryMinutesPerRequest)) <
       assets_last_discovered_at)) {
    wallet_service_->OnDiscoverAssetsCompleted({});
    return;
  }
  prefs_->SetTime(kBraveWalletLastDiscoveredAssetsAt, base::Time::Now());

  // Return early and do not send a notification
  // if a discover assets process is flight already
  if (remaining_buckets_ != 0) {
    return;
  }

  remaining_buckets_ = 2;  // 1 for ETH + 1 for SOL
  DiscoverSolAssets(account_addresses[mojom::CoinType::SOL], false);
  DiscoverEthAssets(account_addresses[mojom::CoinType::ETH], false);
}

void AssetDiscoveryManager::AccountsAdded(
    mojom::CoinType coin,
    const std::vector<std::string>& addresses) {
  if (!(coin == mojom::CoinType::ETH || coin == mojom::CoinType::SOL) ||
      addresses.size() == 0u) {
    return;
  }
  DiscoverAssetsOnAllSupportedChainsAccountsAdded(coin, addresses);
}

// static
// Parses the Account object for the `mint` field which is a 32 byte public key.
// See
// https://github.com/solana-labs/solana-program-library/blob/f97a3dc7cf0e6b8e346d473a8c9d02de7b213cfd/token/program/src/state.rs#L86-L105
absl::optional<SolanaAddress> AssetDiscoveryManager::DecodeMintAddress(
    const std::vector<uint8_t>& data) {
  if (data.size() < 32) {
    return absl::nullopt;
  }

  std::vector<uint8_t> pub_key_bytes(data.begin(), data.begin() + 32);
  return SolanaAddress::FromBytes(pub_key_bytes);
}

// static
// Creates a URL like
// https://api.simplehash.com/api/v0/nfts/owners?chains={chains}&wallet_addresses={wallet_addresses}
GURL AssetDiscoveryManager::GetSimpleHashNftsByWalletUrl(
    const std::string& account_address,
    const std::vector<std::string>& chain_ids) {
  if (chain_ids.empty() || account_address.empty()) {
    return GURL();
  }

  std::string urlStr =
      base::StringPrintf("%s/api/v0/nfts/owners", kSimpleHashUrl);
  const base::flat_map<std::string, std::string>& simple_hash_chain_ids =
      ToSimpleHashChainId();
  std::string chain_ids_param;
  for (const auto& chain_id : chain_ids) {
    auto it = simple_hash_chain_ids.find(chain_id);
    if (it != simple_hash_chain_ids.end()) {
      if (!chain_ids_param.empty()) {
        chain_ids_param += ",";
      }
      chain_ids_param += it->second;
    }
  }

  if (chain_ids_param.empty()) {
    return GURL();
  }

  GURL url = GURL(urlStr);

  // Add the chain IDs to the URL as a query parameter
  url = net::AppendQueryParameter(url, "chains", chain_ids_param);

  // Add the wallet address as a query parameter to the URL
  url = net::AppendQueryParameter(url, "wallet_addresses", account_address);

  return GURL(url);
}

}  // namespace brave_wallet
