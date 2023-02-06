/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import { Model, ModelState, defaultState } from './model'
import { TipPanelProxy } from './tip_panel_proxy'
import { createStateManager } from '../../shared/lib/state_manager'
import { ExternalWalletProvider, externalWalletProviderFromString } from '../../shared/lib/external_wallet'
import { optional } from '../../shared/lib/optional'

import { WalletStatus } from 'gen/brave/components/brave_rewards/common/mojom/ledger_types.mojom.m'
import { PublisherStatus } from 'gen/brave/components/brave_rewards/common/mojom/ledger.mojom.m'

export function createModel (): Model {
  const stateManager = createStateManager<ModelState>(defaultState())
  const proxy = TipPanelProxy.getInstance()

  async function loadData () {
    const [
      { balance },
      { banner },
      { externalWallet },
      { parameters }
    ] = await Promise.all([
      proxy.handler.getBalance(),
      proxy.handler.getBanner(),
      proxy.handler.getExternalWallet(),
      proxy.handler.getRewardsParameters()
    ])

    function mapWalletProvider () {
      return externalWallet
        ? externalWalletProviderFromString(externalWallet.type)
        : null
    }

    function mapWalletAuthorized () {
      return externalWallet
        ? externalWallet.status == WalletStatus.kConnected
        : false
    }

    function mapBanner () {
      if (!banner) {
        return {
          title: '',
          description: '',
          logo: '',
          background: '',
          links: {},
          web3URL: ''
        }
      }
      return {
        title: banner.title,
        description: banner.description,
        logo: banner.logo,
        background: banner.background,
        links: { ...(banner.links) },
        web3URL: banner.web3URL
      }
    }

    function mapCreatorWallets (): ExternalWalletProvider[] {
      if (!banner) {
        return []
      }
      switch (banner.status) {
        case PublisherStatus.BITFLYER_VERIFIED: return ['bitflyer']
        case PublisherStatus.GEMINI_VERIFIED: return ['gemini']
        case PublisherStatus.UPHOLD_VERIFIED: return ['uphold']
      }
      return []
    }

    stateManager.update({
      loading: false,
      rewardsUser: {
        balance: optional(balance ? balance.total : undefined),
        walletProvider: mapWalletProvider(),
        walletAuthorized: mapWalletAuthorized()
      },
      rewardsParameters: {
        exchangeCurrency: 'USD',
        exchangeRate: parameters ? parameters.rate : 1,
        contributionAmounts: parameters ? parameters.tipChoices : []
      },
      creatorBanner: mapBanner(),
      creatorWallets: mapCreatorWallets()
    })
  }

  loadData()

  return {
    getState: stateManager.getState,
    addListener: stateManager.addListener,
    onInitialRender () {
      proxy.handler.showUI()
    },
    async sendContribution (amount: number, monthly: boolean) {
      const result = await proxy.handler.sendContribution(amount, monthly)
      return result.success
    },
    reconnectWallet () {
    },
    shareContribution () {
      proxy.handler.shareContribution()
    }
  }
}
