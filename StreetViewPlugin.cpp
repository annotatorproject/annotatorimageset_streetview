// Copyright 2017 Annotator Team

#include "StreetViewPlugin.h"
#include "StreetView.h"

#include <Poco/ClassLibrary.h>

StreetViewPlugin::StreetViewPlugin() {}

std::shared_ptr<AnnotatorLib::ImageSet::AbstractImageSet>
StreetViewPlugin::create(std::string path) {
  return std::make_shared<AnnotatorLib::ImageSet::StreetView>(path);
}

ANNOTATORLIB_API POCO_BEGIN_MANIFEST(AnnotatorLib::ImageSet::ImageSetPlugin)
    POCO_EXPORT_CLASS(StreetViewPlugin) POCO_END_MANIFEST
