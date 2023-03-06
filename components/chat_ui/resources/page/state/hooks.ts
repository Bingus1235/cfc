// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import getPageHandlerInstance, { ConversationTurn } from '../api/page_handler'

export function useConversationHistory() {
  const [conversationHistory, setConversationHistory] = React.useState<ConversationTurn[]>([])

  const getConversationHistory = () => {
    getPageHandlerInstance().pageHandler.getConversationHistory().then(res => setConversationHistory(res.conversationHistory))
  }

  const appendHistory = (turn: ConversationTurn) => {
    setConversationHistory((prevState) => [...prevState, turn])
  }

  React.useEffect(() => {
    getConversationHistory()

    getPageHandlerInstance().callbackRouter.onContextChange.addListener(getConversationHistory)

    getPageHandlerInstance().callbackRouter.onResponse.addListener((turn: ConversationTurn) => {
      appendHistory(turn)
    })
  }, [])

  return {
    conversationHistory,
    appendHistory
  }
}

export function useInput() {
  const [value, setValue] = React.useState('')

  return {
    value,
    setValue
  }
}
