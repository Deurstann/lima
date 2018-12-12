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

    You should have received a copy of the GNU Affero General Public License
    along with LIMA.  If not, see <http://www.gnu.org/licenses/>
*/
/***************************************************************************
 *   Copyright (C) 2004-2012 by CEA LIST                               *
 *                                                                         *
 ***************************************************************************/
#ifndef LIMA_LINGUISTICPROCESSING_ANALYSISDICTABSTRACTRWACCESSRESOURCE_H
#define LIMA_LINGUISTICPROCESSING_ANALYSISDICTABSTRACTRWACCESSRESOURCE_H

#include "linguisticProcessing/core/LinguisticResources/AbstractAccessResource.h"
#include "common/misc/AbstractRwAccessByString.h"
#include "common/misc/AbstractAccessByString.h"

namespace Lima {
namespace LinguisticProcessing {
namespace AnalysisDict {

/**
@author Olivier Mesnard
*/
class AbstractRwAccessResource : public AbstractAccessResource
{
  Q_OBJECT
public:

    AbstractRwAccessResource(QObject* parent = 0);
    virtual ~AbstractRwAccessResource();

    virtual void init(
      Common::XMLConfigurationFiles::GroupConfigurationStructure& unitConfiguration,
      Manager* manager) override = 0;

    virtual Common::AbstractAccessByString* getAccessByString() const override = 0;
 
    virtual Common::AbstractModifierOnAccessByString* getRwAccessByString() const = 0;

};

}

}

}

#endif
