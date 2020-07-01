/*
    Copyright 2002-2013 CEA LIST

    This file is part of LIMA.

    LIMA is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LIMA is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public Lice↔nse
    along with LIMA.  If not, see <http://www.gnu.org/licenses/>
*/


#include "AnalysisContent.h"

using namespace std;

namespace Lima
{

AnalysisContent::AnalysisContent() :
    m_analysisData()
{}

AnalysisContent::~AnalysisContent()
{
  for (auto it = m_analysisData.begin(); it != m_analysisData.end(); it++)
  {
    delete it->second;
    it->second=0;
  }
}

AnalysisData* AnalysisContent::getData(const std::string& id)
{
  auto it = m_analysisData.find(id);
  if (it == m_analysisData.end())
  {
#ifdef DEBUG_CD
    PROCESSUNITFRAMEWORKLOGINIT;
    LTRACE << "data " << id.c_str() << " doesn't exists, return 0";
#endif
    return nullptr;
  }
  return it->second;
}

const AnalysisData* AnalysisContent::getData(const std::string& id) const
{
  auto it = m_analysisData.find(id);
  if (it==m_analysisData.end())
  {
#ifdef DEBUG_CD
    PROCESSUNITFRAMEWORKLOGINIT;
    LTRACE << "data " << id.c_str() << " doesn't exists, return 0";
#endif
    return nullptr;
  }
  return it->second;
}

void AnalysisContent::setData(const std::string& id, AnalysisData* data)
{
#ifdef DEBUG_CD
  PROCESSUNITFRAMEWORKLOGINIT;
  LDEBUG << "setData " << id.c_str();
#endif
  auto it = m_analysisData.find(id);
  if (it != m_analysisData.end())
  {
    PROCESSUNITFRAMEWORKLOGINIT;
    LERROR << "id " << id.c_str() << " already exists, it will be replaced";
    if (it->second != 0)
    {
      delete it->second;
    }
    it->second=data;
  }
  m_analysisData[id] = data;
}

void AnalysisContent::removeData(const std::string& id)
{
#ifdef DEBUG_CD
  PROCESSUNITFRAMEWORKLOGINIT;
  LDEBUG << "removeData " << id.c_str();
#endif
  auto it = m_analysisData.find(id);
  if (it != m_analysisData.end())
  {
    if (it->second != 0)
    {
      delete it->second;
    }
    m_analysisData.erase(it);
  }
}

void AnalysisContent::releaseData(const std::string& id)
{
#ifdef DEBUG_CD
  PROCESSUNITFRAMEWORKLOGINIT;
  LDEBUG << "AnalysisContent::releaseData " << id.c_str();
#endif
  auto it = m_analysisData.find(id);
  if (it != m_analysisData.end())
  {
    m_analysisData.erase(it);
  }
}

} // Lima
