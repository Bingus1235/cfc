// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { render } from 'react-dom'
import { initLocale } from 'brave-ui'

import { loadTimeData } from '../../../common/loadTimeData'
import BraveCoreThemeProvider from '../../../common/BraveCoreThemeProvider'
import getPageHandlerInstance, { ConversationTurn, CharacterType } from './api/page_handler'

function App () {
  const [text, setText] = React.useState('')
  const [isLoading, setIsLoading] = React.useState(false)
  const [conversationHistory, setConversationHistory] = React.useState<ConversationTurn[]>([])

  const handleTextChange = (e: any) => {
    const target = e.target as HTMLInputElement
    setText(target.value)
  }

  const handleSubmit = (e: any) => {
    e.preventDefault()
    setIsLoading(true)
    getPageHandlerInstance().pageHandler.queryPrompt(text)
    const turn = { text, characterType: CharacterType.HUMAN }
    updateHistory(turn)
    setText('')
  }

  const updateHistory = (turn: ConversationTurn) => {
    setConversationHistory((prevState) => [...prevState, turn])
  }

  React.useEffect(() => {
    getPageHandlerInstance().pageHandler.getConversationHistory().then(res => setConversationHistory(res.conversationHistory))

    getPageHandlerInstance().callbackRouter.onResponse.addListener((turn: ConversationTurn) => {
      updateHistory(turn)
      setIsLoading(false)
    })
  }, [])

  return (
    <BraveCoreThemeProvider>
      <div>
        <section>
          {conversationHistory?.map((entry, idx) => (
            <div
              className="turn"
              key={idx}
              style={{
                border: entry.characterType === CharacterType.HUMAN ? 0 : ''
              }}
            >
              {entry.text}
            </div>
          ))}
        </section>
        <div className="loader">{isLoading && '🤔 Thinking…'}</div>
        <form className="form-box">
          <textarea wrap="soft" placeholder="Ask a question" onChange={handleTextChange} value={text} />
          <input type="submit" value="Submit" onClick={handleSubmit} />
        </form>
      </div>
    </BraveCoreThemeProvider>
  )
}

function initialize () {
  initLocale(loadTimeData.data_)
  render(<App />, document.getElementById('mountPoint'))
}

document.addEventListener('DOMContentLoaded', initialize)
