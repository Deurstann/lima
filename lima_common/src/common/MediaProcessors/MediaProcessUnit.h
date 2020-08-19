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
/***************************************************************************
 *   Copyright (C) 2004-2020 by CEA LIST                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef LIMA_COMMON_MEDIAPROCESSORSMEDIAPROCESSUNIT_H
#define LIMA_COMMON_MEDIAPROCESSORSMEDIAPROCESSUNIT_H

#include "common/ProcessUnitFramework/AbstractProcessUnit.h"
#include "common/LimaCommon.h"

namespace Lima
{

namespace Common {
namespace XMLConfigurationFiles {
  class GroupConfigurationStructure;
}
}
class AnalysisContent;

struct MediaProcessUnitInitializationParameters {
  MediaId media;
};

class LIMA_MEDIAPROCESSORS_EXPORT MediaProcessUnit : public AbstractProcessUnit<MediaProcessUnit,MediaProcessUnitInitializationParameters>
{
public:

  /**
  * @brief initialize with parameters from configuration file.
  * @param unitConfiguration @IN : <group> tag in xml configuration file that
  *        contains parameters to initialize the object.
  * @param manager @IN : manager that asked for initialization and carries init params
  * Use it to initialize other objects of same kind.
  * @throw InvalidConfiguration when parameters are invalids.
  */
  virtual void init(
    Common::XMLConfigurationFiles::GroupConfigurationStructure& unitConfiguration,
    Manager* manager) override = 0;

  /**
    * @brief Process on data in analysisContent.
    * This method should not return any exception
    * @param analysis AnalysisContent object on which to process
    * @throw UndefinedMethod if this method hasn't been specialized
    */
  virtual LimaStatusCode process(AnalysisContent& analysis) const override = 0;
};



} // Lima

#endif
