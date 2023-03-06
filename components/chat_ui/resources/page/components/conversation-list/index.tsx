// Copyright (c) 2023 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'
import classnames from 'classnames'

import styles from './style.module.scss'
import { ConversationTurn, CharacterType } from '../../api/page_handler'

interface InputBoxProps {
  list: ConversationTurn[]
}

function ConversationList (props: InputBoxProps) {
  return (
    <div className={styles.list}>
      {props.list.map((turn, id) => {
        const turnClass = classnames({
          [styles.turnAI]: turn.characterType === CharacterType.ASSISTANT,
          [styles.turnHuman]: turn.characterType === CharacterType.HUMAN,
        })

        return (
          <div key={id} className={turnClass}>
            <p>
              {turn.text}
            </p>
          </div>
        )
      })}
    </div>
  )
}

export default ConversationList
