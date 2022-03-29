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
/************************************************************************
 *
 * @file       SpecificEntitiesMicros.cpp
 * @author     Romaric Besancon (romaric.besancon@cea.fr)
 * @date       Thu Apr  5 2007
 * copyright   Copyright (C) 2007 by CEA LIST
 *
 ***********************************************************************/

#include "SpecificEntitiesMicros.h"
#include "common/AbstractFactoryPattern/SimpleFactory.h"
#include "linguisticProcessing/common/PropertyCode/PropertyCodeManager.h"
#include "linguisticProcessing/common/linguisticData/languageData.h"
#include "common/MediaticData/mediaticData.h"
#include "common/Data/strwstrtools.h"

using namespace Lima::Common::XMLConfigurationFiles;
using namespace Lima::Common::PropertyCode;
using namespace Lima::Common::MediaticData;
using namespace std;

namespace Lima {
namespace LinguisticProcessing {
namespace SpecificEntities {

SimpleFactory<AbstractResource,SpecificEntitiesMicros>
SpecificEntitiesMicrosFactory(SPECIFICENTITIESMICROS_CLASSID);

SpecificEntitiesMicros::SpecificEntitiesMicros():
m_micros()
{
}

SpecificEntitiesMicros::~SpecificEntitiesMicros()
{
}

//***********************************************************************
void SpecificEntitiesMicros::init(
    GroupConfigurationStructure& unitConfiguration,
    Manager* manager)
{
#ifdef DEBUG_LP
  SELOGINIT;
  LDEBUG << "SpecificEntitiesMicros initialization";
#endif
  auto language = manager->getInitializationParameters().language;
  const auto& microManager = static_cast<const LanguageData&>(MediaticData::single().mediaData(language)).getPropertyCodeManager().getPropertyManager("MICRO");

  const auto& entities = unitConfiguration.getLists();
  #ifdef DEBUG_LP
  LDEBUG << "entities.size() " << entities.size();
  #endif

  for (auto entity: entities)
  {
    auto entityName = QString::fromStdString(entity.first);
#ifdef DEBUG_LP
    LDEBUG << "Adding categories to entity " << entityName;
#endif
    try
    {
      EntityType type;
      try {
        type = static_cast<const MediaticData&>(MediaticData::single()).getEntityType(entityName);
      } catch (const LimaException& e) {
        SELOGINIT;
        LIMA_EXCEPTION("Unknown entity" << entityName << e.what());
      }
      for (auto micro: entity.second)
      {
        auto code = microManager.getPropertyValue(micro);
        if (code == L_NONE)
        {
          SELOGINIT;
          LERROR << "SpecificEntitiesMicros::init on entity" << entityName
                  << "," << micro
                  << "linguistic code is not defined for language"
                  << MediaticData::single().getMediaId(language);
        }
        else
        {
#ifdef DEBUG_LP
          LDEBUG << "Adding " << micro << code << " to EntityType " << type;
#endif
          m_micros[type].insert(code);
        }
      }
    }
    catch (LimaException& e)
    {
      // just a warning (on LERROR)
      SELOGINIT;
      LERROR << entityName << " is not a defined specific entity";
    }
  }
}

const set<LinguisticCode>* SpecificEntitiesMicros::
getMicros(const EntityType& type)
{
  map<EntityType,set<LinguisticCode> >::const_iterator it=m_micros.find(type);
  if (it==m_micros.end())
  {
    SELOGINIT;
    LERROR << "no microcategories defined for type " << type << " ("
           << Common::Misc::limastring2utf8stdstring(Common::MediaticData::MediaticData::single().getEntityName(type))
           << ")";
    throw LimaException( (QString(QLatin1String("no microcategories defined for type %1"))).arg(MediaticData::single().getEntityName(type)).toUtf8().constData() );
  }
  return &((*it).second);
}



} // end namespace
} // end namespace
} // end namespace
