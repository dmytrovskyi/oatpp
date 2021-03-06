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

#ifndef oatpp_web_server_AsyncHttpConnectionHandler_hpp
#define oatpp_web_server_AsyncHttpConnectionHandler_hpp

#include "./HttpProcessor.hpp"

#include "./handler/ErrorHandler.hpp"

#include "./HttpRouter.hpp"

#include "oatpp/web/protocol/http/incoming/Request.hpp"
#include "oatpp/web/protocol/http/outgoing/Response.hpp"

#include "oatpp/network/server/Server.hpp"
#include "oatpp/network/Connection.hpp"

#include "oatpp/core/concurrency/Thread.hpp"
#include "oatpp/core/concurrency/Runnable.hpp"

#include "oatpp/core/data/stream/StreamBufferedProxy.hpp"
#include "oatpp/core/data/buffer/IOBuffer.hpp"
#include "oatpp/core/async/Processor.hpp"

namespace oatpp { namespace web { namespace server {
  
class AsyncHttpConnectionHandler : public base::Controllable, public network::server::ConnectionHandler {
private:
  
  class Task : public base::Controllable, public concurrency::Runnable {
  public:
    typedef oatpp::collection::LinkedList<std::shared_ptr<HttpProcessor::ConnectionState>> Connections;
  private:
    void consumeConnections(oatpp::async::Processor& processor);
  private:
    oatpp::concurrency::SpinLock::Atom m_atom;
    Connections m_connections;
  private:
    HttpRouter* m_router;
    std::shared_ptr<handler::ErrorHandler> m_errorHandler;
    HttpProcessor::RequestInterceptors* m_requestInterceptors;
  public:
    Task(HttpRouter* router,
         const std::shared_ptr<handler::ErrorHandler>& errorHandler,
         HttpProcessor::RequestInterceptors* requestInterceptors)
      : m_atom(false)
      , m_router(router)
      , m_errorHandler(errorHandler)
      , m_requestInterceptors(requestInterceptors)
    {}
  public:
    
    static std::shared_ptr<Task> createShared(HttpRouter* router,
                                              const std::shared_ptr<handler::ErrorHandler>& errorHandler,
                                              HttpProcessor::RequestInterceptors* requestInterceptors){
      return std::make_shared<Task>(router, errorHandler, requestInterceptors);
    }
    
    void run() override;
    
    void addConnection(const std::shared_ptr<HttpProcessor::ConnectionState>& connectionState){
      oatpp::concurrency::SpinLock lock(m_atom);
      m_connections.pushBack(connectionState);
    }
    
  };
  
private:
  std::shared_ptr<HttpRouter> m_router;
  std::shared_ptr<handler::ErrorHandler> m_errorHandler;
  HttpProcessor::RequestInterceptors m_requestInterceptors;
  v_int32 m_taskBalancer;
  v_int32 m_threadCount;
  std::shared_ptr<Task>* m_tasks;
public:
  AsyncHttpConnectionHandler(const std::shared_ptr<HttpRouter>& router)
    : m_router(router)
    , m_errorHandler(handler::DefaultErrorHandler::createShared())
    , m_taskBalancer(0)
    , m_threadCount(2)
  {
    m_tasks = new std::shared_ptr<Task>[m_threadCount];
    for(v_int32 i = 0; i < m_threadCount; i++) {
      auto task = Task::createShared(m_router.get(), m_errorHandler, &m_requestInterceptors);
      m_tasks[i] = task;
      concurrency::Thread thread(task);
      thread.detach();
    }
  }
public:
  
  static std::shared_ptr<AsyncHttpConnectionHandler> createShared(const std::shared_ptr<HttpRouter>& router){
    return std::shared_ptr<AsyncHttpConnectionHandler>(new AsyncHttpConnectionHandler(router));
  }
  
  ~AsyncHttpConnectionHandler(){
    delete [] m_tasks;
  }
  
  void setErrorHandler(const std::shared_ptr<handler::ErrorHandler>& errorHandler){
    m_errorHandler = errorHandler;
    if(!m_errorHandler) {
      m_errorHandler = handler::DefaultErrorHandler::createShared();
    }
  }
  
  void addRequestInterceptor(const std::shared_ptr<handler::RequestInterceptor>& interceptor) {
    m_requestInterceptors.pushBack(interceptor);
  }
  
  void handleConnection(const std::shared_ptr<oatpp::data::stream::IOStream>& connection) override;
  
};
  
}}}

#endif /* oatpp_web_server_AsyncHttpConnectionHandler_hpp */

