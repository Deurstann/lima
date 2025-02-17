// Copyright 2002-2022 CEA LIST
// SPDX-FileCopyrightText: 2022 CEA LIST <gael.de-chalendar@cea.fr>
//
// SPDX-License-Identifier: MIT

/**
  * @date       begin Mon Oct, 13 2003 (ven oct 18 2002)
  * @author     Gael de Chalendar <Gael.de-Chalendar@cea.fr>
  */

#ifndef XMLCONFIGURATIONFILEPARSER_H
#define XMLCONFIGURATIONFILEPARSER_H

#include "common/LimaCommon.h"

#include <string>
#include <deque>

namespace Lima {
namespace Common {
namespace XMLConfigurationFiles {

class ConfigurationStructure;
class ModuleConfigurationStructure;
class GroupConfigurationStructure;

class XMLConfigurationFileParserPrivate;
/**
  * @brief Parser class for the lima's xml configuration files
  * @author Gael de Chalendar
  */
class LIMA_XMLCONFIGURATIONFILES_EXPORT XMLConfigurationFileParser
{
public:
  XMLConfigurationFileParser(const QString &configurationFileName);
//   /// @deprecated
//   XMLConfigurationFileParser(const std::string &configurationFileName);
  XMLConfigurationFileParser(const XMLConfigurationFileParser& p);
  ~XMLConfigurationFileParser();
  XMLConfigurationFileParser() = delete;
  XMLConfigurationFileParser& operator=(const XMLConfigurationFileParser& p) = delete;

    ConfigurationStructure& getConfiguration(void);

    /** @throw NoSuchModule*/
    ModuleConfigurationStructure& getModuleConfiguration(
      const std::string& moduleName);

    /** @throw NoSuchModule, @throw NoSuchGroup*/
    GroupConfigurationStructure& getModuleGroupConfiguration(
      const std::string& moduleName,
      const std::string& groupName);

    /** @throw NoSuchModule, @throw NoSuchGroup, @throw NoSuchParam */
    std::string& getModuleGroupParamValue(const std::string& moduleName,
                                          const std::string& groupName,
                                          const std::string& key);

    /** @throw NoSuchModule, @throw NoSuchGroup, @throw NoSuchList */
    std::deque< std::string >& getModuleGroupListValues(
      const std::string& moduleName,
      const std::string& groupName,
      const std::string& key);

    friend LIMA_XMLCONFIGURATIONFILES_EXPORT std::ostream& operator<<(
      std::ostream& os, XMLConfigurationFileParser& parser);

    const QString& getConfigurationFileName() const;

private:
  std::unique_ptr<XMLConfigurationFileParserPrivate> m_d;
};


} // closing namespace XMLConfigurationFiles
} // closing namespace Common
} // closing namespace Lima

#endif
