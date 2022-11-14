//
// Created by sieve on 2021/9/25.
//

#include "simple_xml.h"

namespace simplexml {

};// namespace simplexml

xml_construct::xml_construct() : doc_(xmlNewDoc(BAD_CAST "1.0")),
                                 root_(xmlNewNode(NULL, BAD_CAST "Patch")) {
  xmlDocSetRootElement(doc_, root_);
}

xml_construct::~xml_construct() {
  xmlFreeDoc(doc_);
}
