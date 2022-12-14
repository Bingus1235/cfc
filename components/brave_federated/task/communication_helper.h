/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_FEDERATED_TASK_COMMUNICATION_HELPER_H_
#define BRAVE_COMPONENTS_BRAVE_FEDERATED_TASK_COMMUNICATION_HELPER_H_

#include <string>

#include "brave/components/brave_federated/task/typing.h"

namespace brave_federated {

std::string BuildGetTasksPayload();
std::string BuildPostTaskResultsPayload(TaskResult result);
}  // namespace brave_federated

#endif  // BRAVE_COMPONENTS_BRAVE_FEDERATED_TASK_COMMUNICATION_HELPER_H_
