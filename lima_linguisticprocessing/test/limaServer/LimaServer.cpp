/****************************************************************************
 **
 ** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** You may use this file under the terms of the BSD license as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
 **     of its contributors may be used to endorse or promote products derived
 **     from this software without specific prior written permission.
 **
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "LimaServer.h"
#include "analysisthread.h"

// #ifdef WIN32
#include "common/AbstractFactoryPattern/AmosePluginsManager.h"
// #endif

#include "common/Data/strwstrtools.h"
#include "common/MediaProcessors/MediaAnalysisDumper.h"
#include "common/MediaProcessors/MediaProcessUnit.h"
#include "common/MediaticData/mediaticData.h"
#include "common/time/traceUtils.h"
#include "common/tools/FileUtils.h"
#include "common/XMLConfigurationFiles/xmlConfigurationFileParser.h"

// definition of logger
#include "linguisticProcessing/LinguisticProcessingCommon.h"
// factories
#include "linguisticProcessing/client/LinguisticProcessingClientFactory.h"
#include "linguisticProcessing/client/AbstractLinguisticProcessingClient.h"


#include <qhttpserver.h>
#include <qhttprequest.h>
#include <qhttpresponse.h>

#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QCoreApplication>
#include <QTimer>

#include <stdlib.h>
#include <boost/graph/buffer_concepts.hpp>

using namespace Lima;
using namespace Lima::Common;
using namespace Lima::Common::MediaticData;
using namespace Lima::Common::XMLConfigurationFiles;
using namespace Lima::Common::Misc;
using namespace Lima::LinguisticProcessing;

LimaServer::LimaServer( const QString& configPath,
                        const QString& commonConfigFile,
                        const QString& lpConfigFile,
                        const QString& resourcesPath,
                        const std::deque<std::string>& langs,
                        const std::deque<std::string>& pipelines,
                        int port,
                        QObject *parent,
                        QTimer* t)
     : QObject(parent), m_langs(langs.begin(),langs.end()), m_timer(t)
{
  LIMASERVERLOGINIT;
  LDEBUG << "::LimaServer::LimaServer()...";
  
  std::ostringstream oss;
  std::ostream_iterator<std::string> out_it (oss,", ");
  std::copy ( langs.begin(), langs.end(), out_it );
  LDEBUG << "LimaServer::LimaServer: init MediaticData("
           << "resourcesPath=" << resourcesPath
           << "configPath=" << configPath
           << "commonConfigFile=" << commonConfigFile
           << "lpConfigFile=" << lpConfigFile
           << "langs=" << oss.str();
  Common::MediaticData::MediaticData::changeable().init(
    resourcesPath.toUtf8().constData(),
    configPath.toUtf8().constData(),
    commonConfigFile.toUtf8().constData(),
    langs);
  
  // initialize linguistic processing
  QString fullLpConfigFile = findFileInPaths(configPath, lpConfigFile);
  std::string clientId("lima-coreclient");
  Lima::Common::XMLConfigurationFiles::XMLConfigurationFileParser lpconfig(fullLpConfigFile.toUtf8().constData());

  LDEBUG << "LimaServer::LimaServer: configureClientFactory...";
  LinguisticProcessingClientFactory::changeable().configureClientFactory(
    clientId,
    lpconfig,
    langs,
    pipelines);
  
  LDEBUG << "LimaServer::LimaServer: createClient...";
  m_analyzer = std::dynamic_pointer_cast<AbstractLinguisticProcessingClient>(LinguisticProcessingClientFactory::single().createClient(clientId));
  
  LDEBUG << "LimaServer::LimaServer: create QHttpServer...";
  m_server = new QHttpServer(this);
  LDEBUG << "LimaServer::LimaServer: connect...";
  connect(m_server, SIGNAL(newRequest(QHttpRequest*,QHttpResponse*)),
          this, SLOT(handleRequest(QHttpRequest*,QHttpResponse*)));

  LINFO << "LimaServer::LimaServer: server listen...";
  m_server->listen(QHostAddress::Any, port);
  LINFO << "Server listening";
 }

LimaServer::~LimaServer()
{
}

void LimaServer::quit() {
  // free httpserver ?
  LIMASERVERLOGINIT;
  LINFO << "LimaServer::quit()...";
  m_timer->stop();
  m_server->close();
  LINFO << "LimaServer::quit(): ask close to tcpServer";
  QCoreApplication *app = (QCoreApplication *)parent();
  app->quit();
}


void LimaServer::handleRequest(QHttpRequest *req, QHttpResponse *resp)
{
  LIMASERVERLOGINIT;
  req->storeBody();
  LDEBUG << "LimaServer::handleRequest:insert" << resp << " at " << req << "...";
  // Need to get response from request in slotHandleEndedRequest
  m_responses.insert(std::pair<QHttpRequest*, QHttpResponse *>(req,resp));

#ifdef MULTITHREAD
  connect(req,SIGNAL(end()),this,SLOT(slotHandleEndedRequest()));
  connect(resp,SIGNAL(done()),this,SLOT(slotResponseDone()));
#else
#endif
}

void LimaServer::slotHandleEndedRequest()
{
  LIMASERVERLOGINIT;
  LDEBUG << "LimaServer::slotHandleEndedRequest";
  QHttpRequest *req = static_cast<QHttpRequest*>(sender());
  QHttpResponse *resp = m_responses[req];
  
  LDEBUG << "LimaServer::slotHandleEndedRequest: create AnalysisThread...";
  AnalysisThread *thread = new AnalysisThread(m_analyzer.get(), req, resp, m_langs, this );
  // Need to get request from thread in sendResults
  m_requests.insert(std::pair<QThread*,QHttpRequest*>(thread,req));
  m_threads.insert(std::pair<QHttpResponse*,QThread*>(resp,thread));
  
#ifdef MULTITHREAD
  connect(thread, SIGNAL(finished()), this, SLOT(sendResults()));
//  connect(thread,SIGNAL(finished()),thread, SLOT(deleteLater()));
  thread->start();
  LDEBUG << "LimaServer::slotHandleEndedRequest: my job is done...";
#else
#endif
}


void LimaServer::slotResponseDone()
{
  QHttpResponse* response = static_cast<QHttpResponse*>(sender());
  LIMASERVERLOGINIT;
  LDEBUG << "LimaServer::slotResponseDone";
  QThread* thread = m_threads[response];
  if (thread->isRunning())
    thread->quit();
  LDEBUG << "LimaServer::slotResponseDone done";
}

void LimaServer::sendResults()
{
  LIMASERVERLOGINIT;
  QThread* thread = static_cast<QThread*>(sender());
  LDEBUG << "LimaServer::sendResults for thread" << thread;
  QHttpRequest *req = m_requests[thread];
  QHttpResponse *resp = m_responses[req];
  AnalysisThread* analysisthread = static_cast<AnalysisThread*>(sender());
  
  resp->writeHead(analysisthread->response_code());
  const std::map<QString,QString>& headers = analysisthread->response_header();
  for( std::map<QString,QString>::const_iterator headerIt = headers.begin() ; headerIt != headers.end() ; headerIt++ ) {
    resp->setHeader(headerIt->first, headerIt->second);
  }

  resp->end(analysisthread->response_body());
  
  // req->deleteLater();
  // thread->deleteLater();
  
  // TODO: clean: remove element from m_requests and m_responses
  LDEBUG << "LimaServer::sendResults done";

}
