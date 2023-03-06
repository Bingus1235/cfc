// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { render } from 'react-dom'
import { initLocale } from 'brave-ui'
import { setIconBasePath } from '@brave/leo/react/icon'

import '$web-components/app.global.scss'
import '@brave/leo/tokens/css/variables.css'

import { loadTimeData } from '../../../common/loadTimeData'
import BraveCoreThemeProvider from '../../../common/BraveCoreThemeProvider'
import Main from './components/main'
import ConversationList from './components/conversation-list'
import InputBox from './components/input-box'
import { useConversationHistory, useInput } from './state/hooks'
import getPageHandlerInstance, { CharacterType } from './api/page_handler'

setIconBasePath('chrome-untrusted://resources/brave-icons')

function App () {
  const { conversationHistory, appendHistory } = useConversationHistory()
  const { value, setValue } = useInput();

  const handleInputChange = (e: any) => {
    const target = e.target as HTMLInputElement
    setValue(target.value)
  }

  const handleSubmit = (e: any) => {
    e.preventDefault()
    getPageHandlerInstance().pageHandler.queryPrompt(value)
    appendHistory({ text: value, characterType: CharacterType.HUMAN })
    setValue('')
  }

  const conversationList = (
    <ConversationList
      list={conversationHistory}
    />
  )

  const inputBox = (
    <InputBox
      value={value}
      onInputChange={handleInputChange}
      onSubmit={handleSubmit}
    />
  )

  return (
    <BraveCoreThemeProvider>
      <Main
        conversationList={conversationList}
        inputBox={inputBox}
      />
    </BraveCoreThemeProvider>
  )
}

function initialize () {
  initLocale(loadTimeData.data_)
  render(<App />, document.getElementById('mountPoint'))
}

document.addEventListener('DOMContentLoaded', initialize)
