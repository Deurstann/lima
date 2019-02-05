/************************************************************************
 *
 * @file       EventTemplateDefinitionResource.cpp
 * @author     Romaric Besancon (romaric.besancon@cea.fr)
 * @date       Fri Sep  2 2011
 * copyright   Copyright (C) 2011 by CEA LIST
 * 
 ***********************************************************************/

#include "EventTemplateDefinitionResource.h"
#include "common/AbstractFactoryPattern/SimpleFactory.h"
#include "common/MediaticData/mediaticData.h"
#include <boost/tokenizer.hpp>

using namespace Lima::Common::XMLConfigurationFiles;
using namespace std;

namespace Lima {
namespace LinguisticProcessing {
namespace EventAnalysis {

//----------------------------------------------------------------------
SimpleFactory<AbstractResource,EventTemplateDefinitionResource> 
EventTemplateDefinitionResourceFactory(EVENTTEMPLATEDEFINITIONRESOURCE_CLASSID);


//----------------------------------------------------------------------
EventTemplateDefinitionResource::EventTemplateDefinitionResource():
m_language(0),
m_templates(),
m_elementMapping()
{
}

EventTemplateDefinitionResource::~EventTemplateDefinitionResource() {
}

const std::string& EventTemplateDefinitionResource::getMention (const std::string name) const
{
  static std::string mention="";
  LOGINIT("LP::EventAnalysis");
  LDEBUG << "getMention m_templates.size() " << m_templates.size();
  for(std::vector<EventTemplateStructure>::const_iterator it=m_templates.begin();it!=m_templates.end();it++)
  {
    LDEBUG << "Cuurent Mention " << it->getMention();
    if (name.compare(it->getName())==0) return it->getMention();
  }
  return mention;
}

const std::map<std::string,Common::MediaticData::EntityType>& EventTemplateDefinitionResource::getStructure (const std::string name) const
{
  static std::map<std::string,Common::MediaticData::EntityType> structure;
  LOGINIT("LP::EventAnalysis");
  LDEBUG << "getMention m_templates.size() " << m_templates.size();
  for(std::vector<EventTemplateStructure>::const_iterator it=m_templates.begin();it!=m_templates.end();it++)
  {
    //LDEBUG << "Cuurent Mention " << it->getMention();
    if (name.compare(it->getName())==0) return it->getStructure();
  }
  return structure;
}

//----------------------------------------------------------------------
void EventTemplateDefinitionResource::
init(GroupConfigurationStructure& unitConfiguration,
     Manager* manager)
   
{
  LOGINIT("LP::EventAnalysis");

  m_language=manager->getInitializationParameters().language;
  EventTemplateStructure structure;
  // get name
  try
  {
    string name = unitConfiguration.getParamsValueAtKey("templateName");
    structure.setName(name);
    LDEBUG << "Template name = "<< name;
    
  }
  catch (NoSuchParam& ) {
    LERROR << "No param 'templateName' in EventTemplateDefinitionResource for language " << (int)m_language;
    throw InvalidConfiguration();
  }
  try{
  
    string nameMention = unitConfiguration.getParamsValueAtKey("templateMention");
    LDEBUG << "Template mention = "<< nameMention;
    structure.setMention(nameMention);
  }
  
  catch (NoSuchParam& ) {
    LERROR << "No param 'templateMention' in EventTemplateDefinitionResource for language " << (int)m_language;
    //throw InvalidConfiguration();
  }

  // get template elements: role and entity types
  try
  {
    map<string,string> elts  = unitConfiguration.getMapAtKey("templateElements");
    LDEBUG << "templateElements .size " << elts.size();
    for(auto it=elts.begin(),it_end=elts.end();it!=it_end;it++) {
      LDEBUG << "templateElement =" << (*it).first;
      structure.addTemplateElement((*it).first,(*it).second);
    }
  }
  catch (NoSuchParam& ) {
    LERROR << "No param 'templateName' in EventTemplateDefinition for language " << (int)m_language;
    throw InvalidConfiguration();
  }

  // get element mapping, for template merging
  LDEBUG << "get elementMapping ";
  try
  {
    map<string,string> mapping  = unitConfiguration.getMapAtKey("elementMapping");
    LDEBUG << "after Getting map ";
    for(auto it=mapping.cbegin(),it_end=mapping.cend();it!=it_end;it++) {
      const std::string& elements=(*it).second;
      // comma-separated list of elements
      boost::char_separator<char> sep(",; ");
      boost::tokenizer<boost::char_separator<char> > tok(elements,sep);
      for(auto e=tok.begin(),e_end=tok.end(); e!=e_end;e++) {
        LDEBUG << "EventTemplateDefinitionResource: add mapping " 
                << (*it).first << ":" << *e;
        m_elementMapping[(*it).first].insert(*e);
      }
    }
  }
  catch (NoSuchMap& ) {
    LDEBUG << "No param 'elementMapping' in EventTemplateDefinition for language " 
            << (int)m_language;
  }
  LDEBUG << "Adding Structure ";
  m_templates.push_back(structure);
}

int EventTemplateDefinitionResource::
existsMapping(const std::string& eltName1, const std::string& eltName2) const
{
  int res=0; 
  map<string,set<string> >::const_iterator it=m_elementMapping.find(eltName1);
  if (it!=m_elementMapping.end()) {
    if ( (*it).second.find(eltName2) != (*it).second.end() ) {
      res=1;
    }
  }
  else {
    // try other way
    map<string,set<string> >::const_iterator it=m_elementMapping.find(eltName2);
    if (it!=m_elementMapping.end()) {
      if ( (*it).second.find(eltName1) != (*it).second.end() ) {
        res=-1;
      }
    }
  }
  LOGINIT("LP::EventAnalysis");
  LDEBUG << "EventTemplateDefinitionResource::existsMapping : compare " << eltName1 << " and " << eltName2 << "->" << res;
  return res;
}



} // end namespace
} // end namespace
} // end namespace
