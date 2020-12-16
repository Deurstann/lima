/*
    Copyright 2002-2020 CEA LIST

    This file is part of LIMA.

    LIMA is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LIMA is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with LIMA.  If not, see <http://www.gnu.org/licenses/>
*/
#ifndef LIMA_LINGUISTICPROCESSING_METADATAHANDLER_H
#define LIMA_LINGUISTICPROCESSING_METADATAHANDLER_H

#include "AnalysisHandlersExport.h"
#include "common/Handler/AbstractAnalysisHandler.h"
#include "linguisticProcessing/client/AnalysisHandlers/SimpleStreamHandler.h"
#include "linguisticProcessing/client/AnalysisHandlers/AbstractTextualAnalysisHandler.h"

#include <ostream>

namespace Lima
{
namespace LinguisticProcessing
{

class LIMA_ANALYSISHANDLERS_EXPORT MetaDataHandler : public AbstractTextualAnalysisHandler
{
  Q_OBJECT
public:
  MetaDataHandler(std::ostream* out);

  MetaDataHandler();
  virtual ~MetaDataHandler();


  virtual void endAnalysis() override;
  virtual void handle(const char* buf, int length) override ;
  virtual void startAnalysis() override;

  void startDocument(const Common::Misc::GenericDocumentProperties&) override;
  void endDocument() override;
  void startNode( const std::string& elementName, bool forIndexing ) override;
  void endNode(const Common::Misc::GenericDocumentProperties& props) override;
  
  virtual void setOut( std::ostream* out ) override;
  
  const std::map<std::string,std::string>& getMetaData();

private:
  std::ostringstream* m_stream;
  std::ostream* m_out;
  Lima::LinguisticProcessing::SimpleStreamHandler* m_writer;
  std::map<std::string,std::string> m_metadata;

};

} // closing namespace LinguisticProcessing
} // closing namespace Lima

#endif
