/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi, <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef oatpp_async_Processor_hpp
#define oatpp_async_Processor_hpp

#include "./Coroutine.hpp"
#include "oatpp/core/collection/FastQueue.hpp"

namespace oatpp { namespace async {
  
class Processor {
private:
  
  bool checkWaitingQueue();
  bool considerCheckWaitingQueue(bool immediate);
  
private:
  oatpp::collection::FastQueue<AbstractCoroutine> m_activeQueue;
  oatpp::collection::FastQueue<AbstractCoroutine> m_waitingQueue;
private:
  v_int32 m_sleepCountdown = 0;
  v_int32 m_checkWaitingQueueCountdown = 0;
public:

  void addCoroutine(AbstractCoroutine* coroutine);
  void addWaitingCoroutine(AbstractCoroutine* coroutine);
  bool iterate(v_int32 numIterations);
  
};
  
}}

#endif /* oatpp_async_Processor_hpp */
