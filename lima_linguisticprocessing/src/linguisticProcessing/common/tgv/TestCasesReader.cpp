// Copyright 2002-2022 CEA LIST
// SPDX-FileCopyrightText: 2022 CEA LIST <gael.de-chalendar@cea.fr>
//
// SPDX-License-Identifier: MIT

/***************************************************************************
 * @author Benoit Mathieu
 ***************************************************************************/

#include "linguisticProcessing/common/tgv/TestCasesReader.h"
#include "linguisticProcessing/common/tgv/TestCaseError.hpp"
#include "common/Data/strwstrtools.h"

#include <QXmlStreamReader>

#include <iostream>

namespace Lima
{
namespace Common
{
namespace TGV
{

char const * const FATAL_ERROR_TYPE = "bloquant";
int const EXIT_FATAL_ERROR = 64;

class TestCasesReaderPrivate
{
  friend class TestCasesReader;

public:
  TestCasesReaderPrivate(TestCaseProcessor& processor);
   ~TestCasesReaderPrivate() = default;

  bool parse(QIODevice *device);
  void readTestcases();
  void readTestcase();
  void readCallParameters();
  void readParam();
  void readList();
  void readMap();
  void readItem();
  void readExpl();
  void readTest();
  void readText();

  QXmlStreamReader m_reader;

  std::map<std::string, TestCasesReader::TestReport> m_reportByType;


  TestCaseProcessor& m_processor;
  TestCase currentTestCase;

  std::string getName(const QString& localName,
                      const QString& qName);

  bool m_inList;
  bool m_inMap;
  std::string m_listKey;
  std::string m_mapKey;

  bool m_hasFatalError;
};


TestCasesReaderPrivate::TestCasesReaderPrivate(TestCaseProcessor& processor)
  : m_reportByType(), m_processor(processor), m_hasFatalError(false)
{
}

TestCasesReader::TestCasesReader( TestCaseProcessor& processor) :
  m_d(new TestCasesReaderPrivate(processor))
{
}

TestCasesReader::~TestCasesReader()
{
  delete m_d;
}

const std::map<std::string, TestCasesReader::TestReport>& TestCasesReader::reportByType() const
{
  return m_d->m_reportByType;
}

bool TestCasesReader::parse(QIODevice *device)
{
  return m_d->parse(device);
}

// <?xml version='1.0' encoding='UTF-8'?>
// <testcases>
//   <testcase id="eng.se.DATE.1" type="bloquant">
//     <call-parameters>
//       <param key="text" value="he was born on 10 April 1950."/>
//       <list key="pipelines">
//         <item value="limaserver"/>
//       </list>
//       …
//     </call-parameters>
//     <expl>rule for complete date with cardinal number</expl>
//     <test id="eng.se.DATE.1.1" trace=".se.xml"
//           comment="type"
//           left="XPATH#//entities/entity[pos=16][len=13]/type"
//           operator="="
//           right="DateTime.DATE"/>
//   …
//   </testcase>
//   …
// </testcases>
bool TestCasesReaderPrivate::parse(QIODevice *device)
{
  TGVLOGINIT;
  LDEBUG << "TestCasesReaderPrivate::parse";
  m_reader.setDevice(device);
  if (m_reader.readNextStartElement()) {
      if (m_reader.name() == QLatin1String("testcases"))
      {
          readTestcases();
      }
      else
      {
          m_reader.raiseError(QObject::tr("The file is not a LIMA test cases file."));
      }
  }
  return !m_reader.error();
}

// <testcases>
//   <testcase id="eng.se.DATE.1" type="bloquant">
//   …
//   </testcase>
//   …
// </testcases>
void TestCasesReaderPrivate::readTestcases()
{
  TGVLOGINIT;
  LTRACE << "TestCasesReaderPrivate::readCodes" << m_reader.name();
  Q_ASSERT(m_reader.isStartElement() && m_reader.name() == QLatin1String("testcases"));

  while (m_reader.readNextStartElement()) {
      if (m_reader.name() == QLatin1String("testcase"))
          readTestcase();
      else
          m_reader.raiseError(QObject::tr("Expected a code but got a %1.").arg(m_reader.name()));
  }
}

//   <testcase id="eng.se.DATE.1" type="bloquant">
//     <call-parameters>
//       …
//     </call-parameters>
//     <expl>rule for complete date with cardinal number</expl>
//     <test id="eng.se.DATE.1.1" trace=".se.xml"
//           comment="type"
//           left="XPATH#//entities/entity[pos=16][len=13]/type"
//           operator="="
//           right="DateTime.DATE"/>
//   …
//   </testcase>
void TestCasesReaderPrivate::readTestcase()
{
  TGVLOGINIT;
  LTRACE << "TestCasesReaderPrivate::readTestcase" << m_reader.name();
  Q_ASSERT(m_reader.isStartElement() && m_reader.name() == QLatin1String("testcase"));

  LDEBUG << "TestCasesReaderPrivate::readTestcase: new testcase ";
  currentTestCase = TestCase();
  currentTestCase.id = m_reader.attributes().value("id").toString().toStdString();
  auto typech = m_reader.attributes().value("type").toString().toStdString();
  currentTestCase.type = typech.empty() ? "undefined": typech;
  LDEBUG << "TestCasesReaderPrivate::readTestcase: id=" << currentTestCase.id
          << ", type=" << currentTestCase.type;

  bool hasExpl = false;
  bool hasCallParameters = false;
  bool hasTest = false;
  while (m_reader.readNextStartElement())
  {
    if (m_reader.name() == QLatin1String("call-parameters"))
    {
      hasCallParameters = true;
      readCallParameters();
    }
    else if (m_reader.name() == QLatin1String("expl"))
    {
      hasExpl = true;
      readExpl();
    }
    else if (m_reader.name() == QLatin1String("test"))
    {
      hasTest = true;
      readTest();
    }
    else
        m_reader.raiseError(QObject::tr("Expected a call-parameters, a expl or a test but got a %1.").arg(m_reader.name()));
  }
  if (!hasCallParameters)
    m_reader.raiseError(QObject::tr("Testcase is missing a call-parameters."));
  if (!hasTest)
    m_reader.raiseError(QObject::tr("Testcase should have at least one test. None found."));
  if (!hasExpl)
  {
    LINFO << "Test case" << currentTestCase.id << "," << currentTestCase.type << "at line"
          << m_reader.lineNumber() << ", column" << m_reader.columnNumber()
          << "has no explanation. This is wrong.";
  }

  m_reportByType[currentTestCase.type].nbtests++;
  LDEBUG << "TestCasesReader::endElement: call processTestCase(" << currentTestCase.id
                                                          << "," << currentTestCase.type << ")";
  TestCaseError e = m_processor.processTestCase(currentTestCase);
  if (e())
  {
    std::cout << currentTestCase.id << " (" << currentTestCase.type << ") got error (type: '"<<e()<<"'): "
              << std::endl << e.what() << std::endl;
    if (currentTestCase.type == FATAL_ERROR_TYPE)
    {
      m_hasFatalError = true;
    }
    if (e() == TestCaseError::TestCaseFailed)
    {
      if (e.isConditional())
      {
        m_reportByType[currentTestCase.type].conditional++;
      }
      else
      {
        m_reportByType[currentTestCase.type].failed++;
      }
    }
    else
    {
      std::cerr << "runtime error: " << e.what() << std::endl;
      m_reportByType[currentTestCase.type].failed++;
      throw std::runtime_error(e.what());
    }
  }
  else
  {
    std::cout << currentTestCase.id << " (" << currentTestCase.type << ") passed successfully." << std::endl;
    m_reportByType[currentTestCase.type].success++;
  }

}

//     <call-parameters>
//       <param key="text" value="he was born on 10 April 1950."/>
//       <list key="pipelines">
//         <item value="limaserver"/>
//       </list>
//       …
//     </call-parameters>
void TestCasesReaderPrivate::readCallParameters()
{
  TGVLOGINIT;
  LTRACE << "TestCasesReaderPrivate::readCallParameters" << m_reader.name();
  Q_ASSERT(m_reader.isStartElement() && m_reader.name() == QLatin1String("call-parameters"));

  while (m_reader.readNextStartElement())
  {
      if (m_reader.name() == QLatin1String("param"))
          readParam();
      else if (m_reader.name() == QLatin1String("list"))
          readList();
      else if (m_reader.name() == QLatin1String("map"))
          readMap();
      else
          m_reader.raiseError(QObject::tr("Expected a param, a list or a map but got a %1.").arg(m_reader.name()));
  }
//   LTRACE << "TestCasesReaderPrivate::readCallParameters no more start element";

}

//       <param key="text" value="he was born on 10 April 1950."/>
// OR    <param key="text">he was born on 10 April 1950.</param>
void TestCasesReaderPrivate::readParam()
{
  TGVLOGINIT;
  LTRACE << "TestCasesReaderPrivate::readParam" << m_reader.name();
  Q_ASSERT(m_reader.isStartElement() && m_reader.name() == QLatin1String("param"));

  std::string key = m_reader.attributes().value("key").toString().toStdString();
  std::string val;
  if (m_reader.attributes().hasAttribute("value"))
  {
    val = m_reader.attributes().value("value").toString().toStdString();
    m_reader.skipCurrentElement();
  }
  else
  {
    val = m_reader.readElementText().toStdString();
  }
  LTRACE << "TestCasesReaderPrivate::readParam key:" << key << "value:" << val;
  currentTestCase.simpleValCallParams.insert(std::pair<std::string, std::string>(key,val) );
  // TODO: read multiValCallParams "pipelines"
  // std::string pipch=m_reader.attributes().value("pipelines");
}

//       <list key="pipelines">
//         <item value="limaserver"/>
//         …
//       </list>
void TestCasesReaderPrivate::readList()
{
  TGVLOGINIT;
  LTRACE << "TestCasesReaderPrivate::readList" << m_reader.name();
  Q_ASSERT(m_reader.isStartElement() && m_reader.name() == QLatin1String("list"));

  m_listKey = m_reader.attributes().value("key").toString().toStdString();
  auto pos = currentTestCase.multiValCallParams.find(m_listKey);
  if( pos == currentTestCase.multiValCallParams.end() )
  {
    // typedef std::map<std::string,std::list<std::string> > MultiValCallParams;
    std::pair<std::string,std::list<std::string> > newElem(m_listKey,std::list<std::string>() );
//      std::pair<MultiValCallParams::iterator,bool> ret =
//        currentTestCase.multiValCallParams.insert(
//          std::pair<std::string,std::list<std::string> >(m_listKey,std::list<std::string>) );
    /*std::pair<MultiValCallParams::iterator,bool> ret =*/ currentTestCase.multiValCallParams.insert(newElem);
  }
  m_inList = true;

  while (m_reader.readNextStartElement())
  {
    if (m_reader.name() == QLatin1String("item"))
        readItem();
    else
        m_reader.raiseError(QObject::tr("Expected an item but got a %1.").arg(m_reader.name()));
  }

  m_inList=false;

}

//       <map key="pipelines">
//         <item value="limaserver"/>
//         …
//       </map>
void TestCasesReaderPrivate::readMap()
{
  TGVLOGINIT;
  LTRACE << "TestCasesReaderPrivate::readMap" << m_reader.name();
  Q_ASSERT(m_reader.isStartElement() && m_reader.name() == QLatin1String("map"));

  m_mapKey = m_reader.attributes().value("key").toString().toStdString();
  MapValCallParams::iterator pos = currentTestCase.mapValCallParams.find(m_mapKey);
  if( pos == currentTestCase.mapValCallParams.end() ) {
    // typedef std::map<std::string,std::list<std::string> > MultiValCallParams;
    std::pair<std::string,std::map<std::string,std::string> > newElem(m_mapKey,std::map<std::string,std::string>() );
//      std::pair<MultiValCallParams::iterator,bool> ret =
//        currentTestCase.multiValCallParams.insert(
//          std::pair<std::string,std::list<std::string> >(m_listKey,std::list<std::string>) );
    /*std::pair<MapValCallParams::iterator,bool> ret =*/ currentTestCase.mapValCallParams.insert(newElem);
  }
  m_inMap=true;

  while (m_reader.readNextStartElement())
  {
    if (m_reader.name() == QLatin1String("item"))
        readItem();
    else
        m_reader.raiseError(QObject::tr("Expected ??? but got a %1.").arg(m_reader.name()));
  }

  m_inMap=false;

}

//         <item value="limaserver"/>
void TestCasesReaderPrivate::readItem()
{
  TGVLOGINIT;
  LTRACE << "TestCasesReaderPrivate::readItem" << m_reader.name();
  Q_ASSERT(m_reader.isStartElement() && m_reader.name() == QLatin1String("item"));

  if(m_inList)
  {
    MultiValCallParams::iterator pos = currentTestCase.multiValCallParams.find(m_listKey);
    if( pos != currentTestCase.multiValCallParams.end() ) {
      std::string val = m_reader.attributes().value("value").toString().toStdString();
      (*pos).second.push_back(val);
    }
    else {
      LERROR << "!! TestCasesReader::startElement: no list for param " << m_listKey;
    }
  }
  else if (m_inMap == true)
  {
    MapValCallParams::iterator pos = currentTestCase.mapValCallParams.find(m_mapKey);
    if( pos != currentTestCase.mapValCallParams.end() ) {
      std::string attr = m_reader.attributes().value("key").toString().toStdString();
      std::string val = m_reader.attributes().value("value").toString().toStdString();
      (*pos).second.insert(make_pair(attr,val));
    }
    else {
      LERROR << "!! TestCasesReader::startElement: no list for param " << m_listKey;
    }
  }
  else
  {
    m_reader.raiseError(QObject::tr("Should be in a list or a map when reading an item.").arg(m_reader.name()));
  }
//    currentTestCase.multiValCallParams.insert(std::pair<std::string, std::string>(key,val) );
  m_reader.skipCurrentElement();

}

//     <expl>rule for complete date with cardinal number</expl>
void TestCasesReaderPrivate::readExpl()
{
  TGVLOGINIT;
  LTRACE << "TestCasesReaderPrivate::readExpl" << m_reader.name();
  Q_ASSERT(m_reader.isStartElement() && m_reader.name() == QLatin1String("expl"));
  currentTestCase.explanation.append(m_reader.text().toString().toStdString());

  m_reader.skipCurrentElement();

}

//     <test id="eng.se.DATE.1.1" trace=".se.xml"
//           comment="type"
//           left="XPATH#//entities/entity[pos=16][len=13]/type"
//           operator="="
//           right="DateTime.DATE"/>
void TestCasesReaderPrivate::readTest()
{
  TGVLOGINIT;
  LTRACE << "TestCasesReaderPrivate::readTest" << m_reader.name();
  Q_ASSERT(m_reader.isStartElement() && m_reader.name() == QLatin1String("test"));

  // build a testUnit
  //cout << "add test" << std::endl;
  TestCase::TestUnit tu;
  tu.id = m_reader.attributes().value("id").toString().toStdString();
  tu.trace = m_reader.attributes().value("trace").toString().toStdString();
  tu.comment = m_reader.attributes().value("comment").toString().toStdString();
  tu.left = m_reader.attributes().value("left").toString().toStdString();
  tu.op = m_reader.attributes().value("operator").toString().toStdString();
  tu.right = m_reader.attributes().value("right").toString().toStdString();
  tu.conditional = (m_reader.attributes().value("conditional").toString()=="yes");
  currentTestCase.tests.push_back(tu);
  LDEBUG << "TestCasesReader::startElement: push testUnit"
          << ", id =" << tu.id
          << ", trace =" << tu.trace
          << ", comment =" << tu.comment
          << ", left =" << tu.left
          << ", op =" << tu.op
          << ", right =" << tu.right
          << ", conditional =" << tu.conditional;
  m_reader.skipCurrentElement();
}

void TestCasesReaderPrivate::readText()
{
  // For compatibility with old testTva format
  std::string newTextVal(m_reader.readElementText().toStdString());

  SimpleValCallParams::iterator pos = currentTestCase.simpleValCallParams.find("text");
  if( pos == currentTestCase.simpleValCallParams.end() )
  {
//       std::pair<SimpleValCallParams::iterator,bool> ret =
      currentTestCase.simpleValCallParams.insert(std::pair<std::string, std::string>("text", newTextVal));
  }
  else
  {
    std::string& textVal = (*pos).second;
    textVal.append(newTextVal);
  }
}

bool TestCasesReader::hasFatalError() const
{
  return m_d->m_hasFatalError;
}

int exitCode(TestCasesReader const & tch)
{
  return tch.hasFatalError() ? EXIT_FATAL_ERROR : EXIT_SUCCESS;
}

QString TestCasesReader::errorString() const
{
  TGVLOGINIT;
  auto errorStr = QObject::tr("%1, Line %2, column %3")
          .arg(m_d->m_reader.errorString())
          .arg(m_d->m_reader.lineNumber())
          .arg(m_d->m_reader.columnNumber());
  LERROR << errorStr;
  return errorStr;
}

void TestCasesReader::clear()
{
  m_d->m_reportByType.clear();
}

} // TGV
} // Common
} // Lima
